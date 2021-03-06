#include "BaseVisitor.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

#include <fstream>
#include <string>
#include <iostream>


namespace
{
	bool isSpecialMemberFunction(clang::CXXMethodDecl* fct)
	{
	    if(isa<CXXConstructorDecl>(*fct) || isa<CXXDestructorDecl>(*fct) || isa<CXXConversionDecl>(*fct))
		return true;
	    else
		return false;
    
	}

	std::string removeExtrakeyword(std::string s)
	{
		static std::string keywords[]={"struct","class","union"};
		for(auto w : keywords)
		{
			auto it = s.find(w);
			if(it!=std::string::npos)
			{
				s.erase(it,w.length()+1);
			}
		}
		return s;
	}
}

//return the code between begin and end
//main idea was taken from SO
std::string BaseVisitor::pos2str(SourceLocation begin,SourceLocation end) {
  auto& sm =astContext.getSourceManager();
  clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(end, 0, sm, astContext.getLangOpts()));
  return std::string(sm.getCharacterData(begin),
                     sm.getCharacterData(e)-sm.getCharacterData(begin));
}
  
//generate the return type for fct
std::string BaseVisitor::ReturnType(CXXMethodDecl* fct)
{
  if(!isSpecialMemberFunction(fct))
  {	
    // TODO : see if clang has a paramter to NOT add
    // struct/class on a return type


    //r2+=fct.getResultType().getAsString()+" ";
    auto  el_type=fct->getCallResultType().getDesugaredType(astContext);
    return el_type.getAsString();
  }
  else
    return "";
}

//generate the list of aguments for fct
std::string BaseVisitor::Arguments(CXXMethodDecl* fct, bool addDefaultValue)
{
  std::string r2("(");
  for(auto param = fct->param_begin();param!=fct->param_end();++param)
  {
    r2+=(*param)->getOriginalType().getAsString()+" "+(*param)->getNameAsString();
            
    if(addDefaultValue && (*param)->hasDefaultArg())
    {
      auto* defaut_arg=(*param)->getDefaultArg();
      r2+=" /* "+pos2str(defaut_arg->getLocStart(),defaut_arg->getLocEnd())+" */";
    }

    r2+=",";
  }

  if(fct->param_size()>0)
    r2.pop_back();

  r2+=")";

  return r2;
}

//generate the exception specifier for fct
std::string BaseVisitor::ExceptionSpecification(CXXMethodDecl* fct)
{
  std::string r2;
  auto qualtype = fct->getType();
  auto rawtype = dyn_cast<FunctionProtoType>(qualtype.getTypePtrOrNull());

  if(rawtype->exception_begin()!=rawtype->exception_end()){
    r2+="throw (";
    for(auto ex_it=rawtype->exception_begin();ex_it<rawtype->exception_end();ex_it++)
    {
      r2+=ex_it->getAsString()+",";
    }
    r2.pop_back();
    r2+=")";
  }
  else if(rawtype->isNothrow(astContext))
  {
    r2+=" noexcept ";
  }
  return r2;
}

std::string BaseVisitor::generateAFunction(std::string base,CXXMethodDecl* fct)
{
  std::string r2;

  //hold template class
  r2+=(templateInfos.isTemplateClass ? templateInfos.baseTemplate : "")+ " ";

  r2+=(fct->isVirtual() && Myoptions::addVirtual ? " /*virtual*/ " : "");
  r2+=std::move(removeExtrakeyword(ReturnType(fct)))+" ";				
  r2+=base;

  //r2+=TemplateList(fct);
  r2+=(templateInfos.isTemplateClass ? templateInfos.listTemplate : "");

  r2+="::";	
  //r2+=fct->getDeclName().getAsString();
  r2+=getNameAndRemoveTemplateList(fct);

  r2+=Arguments(fct,Myoptions::addDefaultValue)+" ";
  r2+=(fct->isConst() ? " const " : "");
  //r2+=ExceptionSpecification(fct);

  r2+="\n{\n}\n";
  return r2;
} 

BaseVisitor::BaseVisitor(CompilerInstance *CI,StringRef file) 
  : astContext(CI->getASTContext()),
    ctx(nullptr),
    outputFile(file.str(),std::ios::app) 
{
     
}

bool BaseVisitor::VisitCXXRecordDecl(CXXRecordDecl *dd) {
  if(dd->getDeclName().getAsString()==Myoptions::classToExpand)
  {
    ctx=dd;
    GenerateTemplateInfos();
    generateAFunctions();
  }
  return true;
}

//add a definition for static member 
bool BaseVisitor::VisitVarDecl(VarDecl* var)
{
  if(var->getDeclContext()==ctx &&
     var->isStaticDataMember() && 
     !var->getOutOfLineDefinition()){

    outputFile<<var->getType().getAsString()+" "+var->getQualifiedNameAsString()+";"
              <<std::endl;
  }
  return true;
}



void BaseVisitor::GenerateTemplateInfos() 
{
	templateInfos.templateClass=ctx->getDescribedClassTemplate();
    	if(templateInfos.templateClass)
    	{
		templateInfos.isTemplateClass=true; 
	    
		auto list=templateInfos.templateClass->getTemplateParameters();
		templateInfos.baseTemplate+=pos2str(list->getLAngleLoc(),list->getRAngleLoc()); 
	 
	
		for(auto it : *list)
			templateInfos.listTemplate+=it->getNameAsString()+",";

		if(list->begin()!=list->end())
		  templateInfos.listTemplate.pop_back();

		templateInfos.listTemplate+=">";
		//std::cout<<templateInfos.baseTemplate<<"\t"<<templateInfos.listTemplate<<std::endl;
	}
}

std::string BaseVisitor::getNameAndRemoveTemplateList(clang::CXXMethodDecl* fct)
{
	std::string s=fct->getDeclName().getAsString();

	if( (isa<CXXConstructorDecl>(*fct) || isa<CXXDestructorDecl>(*fct)) && templateInfos.isTemplateClass)
		{
			auto it1= s.find("<");
			auto it2= s.find(">");
			s.erase(it1,it2-it1+1);
		}
	
	return s;
}
