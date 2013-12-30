#include "clang/Driver/Options.h"
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

#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;


std::string classToExpand;

bool addVirtual=false;
bool addDefaultValue=false;

class BaseVisitor : public RecursiveASTVisitor<BaseVisitor> {
protected:
  ASTContext *astContext; // used for getting additional AST info
  CXXRecordDecl* ctx;

  std::ofstream off;

  std::string pos2str(SourceLocation begin,SourceLocation end) {
            auto& sm =astContext->getSourceManager();
	    clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(end, 0, sm, astContext->getLangOpts()));
	    return std::string(sm.getCharacterData(begin),
		sm.getCharacterData(e)-sm.getCharacterData(begin));
	}

  virtual void GenerateFunctions() =0;

  std::string ReturnType(CXXMethodDecl* fct)
  {

          if( !(isa<CXXConstructorDecl>(*fct) || 
                isa<CXXDestructorDecl>(*fct) || 
                isa<CXXConversionDecl>(*fct)
               )
            )
          {	
            //r2+=fct.getResultType().getAsString()+" ";
	      auto el_type=fct->getResultType().getDesugaredType(*astContext);
	      return el_type.getAsString();
          }
	  else
		return "";
  }

  std::string Arguments(CXXMethodDecl* fct, bool addDefaultValue)
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

   std::string ExceptionSpecification(CXXMethodDecl* fct)
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
	  else if(rawtype->isNothrow(*astContext))
	  {
		r2+=" noexcept ";
	  }
	return r2;
   }

  std::string generateFunction(std::string base,CXXMethodDecl* fct)
  {
          std::string r2;

	  r2+=(fct->isVirtual() && addVirtual ? " /*virtual*/ " : "");
	  r2+=ReturnType(fct)+" ";				
          r2+=base+fct->getNameAsString();
	  r2+=Arguments(fct,addDefaultValue)+" ";
          r2+=(fct->isConst() ? " const " : "");
	  r2+=ExceptionSpecification(fct);

	  r2+="\n{\n}\n";
	  return r2;
  } 
public: 
  explicit BaseVisitor(CompilerInstance *CI,StringRef file) 
    : astContext(&(CI->getASTContext())),
      ctx(nullptr),
      off(file.str(),std::ios::app) // initialize private members
    {
     
    }

  virtual bool VisitCXXRecordDecl(CXXRecordDecl *dd) {

    if(dd->getNameAsString()==classToExpand)
      {
	      ctx=dd;
	      GenerateFunctions();
      }
     return true;
  }

  virtual bool VisitVarDecl(VarDecl* var)
    {
      if(var->getDeclContext()==ctx &&
         var->isStaticDataMember() && 
         !var->getOutOfLineDefinition()){

        off<<var->getType().getAsString()+" "+var->getQualifiedNameAsString()+";"
           <<std::endl;
      }
      return true;
    }
  
};

 //namespace AA {namespace BB{...}} instead of AA::BB::
class PrettyVisitor : public BaseVisitor {

class NamespaceHandler
{
	PrettyVisitor& upper_this;
public :
	NamespaceHandler(PrettyVisitor& up):upper_this(up)
	{
		if(!upper_this.firstNS)
		{
		      for(auto ns  : upper_this.namespaces)
		      {
			upper_this.off<<"namespace "<<ns<<" {"<<std::endl;
		      }    
		}
	}
	~NamespaceHandler()
	{
		if(!upper_this.firstNS)
		{
		      //closse namespace
		      for(std::size_t i =0;i <upper_this.namespaces.size();++i)
		      {
			upper_this.off<<"}"<<std::endl;
		      }    
		}
		else
		{
			upper_this.rewriter.overwriteChangedFiles();
		}
	}
};
protected:
  CXXRecordDecl* topClass;
  NamespaceDecl* firstNS;
  Rewriter rewriter;
  std::vector<std::string> namespaces;

  //check for nested-namespace
  //requieres that NestedClassesCheck have ben performed before
  std::vector<std::string> NestedNamespaceCheck()
     {
       std::vector<std::string> ns;

       if(topClass)
       {

         NamespaceDecl* current_ns=dyn_cast<NamespaceDecl>(topClass->getParent());
	 
	if(current_ns!=nullptr)
         {
	 int count=0;
	 for(auto it = current_ns->redecls_begin();it != current_ns->redecls_end();++it,++count);
         firstNS=(count>1) ? current_ns->getMostRecentDecl() : nullptr;
	 }

         while(current_ns)
         {
          //namesp.insert(0,current_ns->getNameAsString()+"::");
	  ns.push_back(current_ns->getNameAsString());
          current_ns=dyn_cast<NamespaceDecl>(current_ns->getParent());
        }
       }
	std::reverse(ns.begin(),ns.end());
	return ns;
     }

   //check for nested-namespace
   std::string NestedClassesCheck()
     {
       std::string classes;	
       topClass=dyn_cast<CXXRecordDecl>(ctx);
       CXXRecordDecl* current_cl=dyn_cast<CXXRecordDecl>(ctx->getParent());
       while(current_cl)
       {
         topClass=current_cl;
         classes.insert(0,current_cl->getNameAsString()+"::");
         current_cl=dyn_cast<CXXRecordDecl>(current_cl->getParent());
       }
       return classes;
     }

  void GenerateFunctions()
    { 
      std::string classes = NestedClassesCheck();
      namespaces= NestedNamespaceCheck();

      //print namespace in cts and close them in dst
      NamespaceHandler handler(*this);

      const std::string base=classes+ctx->getNameAsString()+"::";      
      for(auto fct = ctx->method_begin();fct!=ctx->method_end();++fct)
      {
        if(!fct->hasBody() && fct->isUserProvided() && !fct->isPure())
        {
 	std::string r2=generateFunction(base,*fct);

	if(firstNS)
          rewriter.InsertTextBefore(firstNS->getLocEnd(),r2);
	else 
	  off<<r2<<std::endl;

        }
      }
	
    }
public: 
  explicit PrettyVisitor(CompilerInstance *CI,StringRef file) 
    : BaseVisitor(CI,file),
      topClass(nullptr)
    {
	rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());  
    }
};

class RawVisitor : public BaseVisitor {
private:
    void GenerateFunctions()
    {      
      const std::string base=ctx->getQualifiedNameAsString()+"::";      
      for(auto fct = ctx->method_begin();fct!=ctx->method_end();++fct)
      {
        if(!fct->hasBody() && fct->isUserProvided() && !fct->isPure())
        {
          std::string r2=generateFunction(base,*fct);
          off<<r2<<std::endl;
	
          std::cout<<"----------"<<std::endl;
        }
      }
    }
public: 
  explicit RawVisitor(CompilerInstance *CI,StringRef file) 
    : BaseVisitor(CI,file)
    {}
};
//===================================================================
class ExampleASTConsumer : public ASTConsumer {
private:
  BaseVisitor *visitor; // doesn't have to be private

public:
  // override the constructor in order to pass CI
  explicit ExampleASTConsumer(CompilerInstance *CI, StringRef file)
    : visitor(new PrettyVisitor(CI,file)) // initialize the visitor
    {}

  // override this to call our ExampleVisitor on the entire source file
  virtual void HandleTranslationUnit(ASTContext &Context) {
    // we can use ASTContext to get the TranslationUnitDecl, which is
    //    a single Decl that collectively represents the entire source file 
    visitor->TraverseDecl(Context.getTranslationUnitDecl());
  }
};



class ExampleFrontendAction : public ASTFrontendAction {
public:
  virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI, StringRef file) {
    return new ExampleASTConsumer(&CI,file); // pass CI pointer to ASTConsumer
  }
};


int main(int argc, const char **argv) {
  cl::opt<std::string> optClassToExpand("class-to-expand", cl:: NotHidden,cl::desc("Class to Expand"));
  cl::opt<bool> optAddDefaultValue("add-default-value", cl:: NotHidden,cl::desc("If true, add a comment to remind the default value"));
  cl::opt<bool> optAddVirtual("add-remind-virtual", cl:: NotHidden,cl::desc("If true, add a comment to remind the function is virtual"));

  // parse the command-line args passed to your code
  CommonOptionsParser op(argc, argv);    
  classToExpand=optClassToExpand;
  addVirtual= optAddVirtual;
  addDefaultValue= optAddDefaultValue;
   
  std::cout<<"===Go==="<<std::endl;

  const auto& sources=op.getSourcePathList();
  if(sources.size()!=1){
    return -1;
  }
  
  // create a new Clang Tool instance (a LibTooling environment)
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    
  // run the Clang Tool, creating a new FrontendAction (explained below)
  int result = Tool.run(newFrontendActionFactory<ExampleFrontendAction>());
  


  return result;
}
