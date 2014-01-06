#include "PrettyVisitor.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

//check for nested-namespace
//requieres that NestedClassesCheck have ben performed before
std::vector<std::string> PrettyVisitor::NestedNamespaceCheck()
{
  std::vector<std::string> ns;

  if(topClass)
  {

    NamespaceDecl* current_ns=dyn_cast<NamespaceDecl>(topClass->getParent());
	 
    //check if the englobing namespace of the class to expand
    //has been redeclarated later
    //If yes, that means we can add at the end
    if(current_ns!=nullptr)
    {
      int count=0;
      for(auto it = current_ns->redecls_begin();it != current_ns->redecls_end();++it,++count);

      enclosingNSRedeclaration=(count>1) ? current_ns->getMostRecentDecl() : nullptr;
    }

    while(current_ns)
    {
      ns.push_back(current_ns->getNameAsString());
      current_ns=dyn_cast<NamespaceDecl>(current_ns->getParent());
    }
  }
  std::reverse(ns.begin(),ns.end());
  return ns;
}

//check for nested-namespace
std::string PrettyVisitor::NestedClassesCheck()
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

void PrettyVisitor::generateAFunctions()
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
      std::string r2=generateAFunction(base,*fct);

      if(enclosingNSRedeclaration)
        rewriter.InsertTextBefore(enclosingNSRedeclaration->getLocEnd(),r2);
      else 
        outputFile<<r2<<std::endl;
    }
  }
	
}

PrettyVisitor::PrettyVisitor(CompilerInstance *CI,StringRef file) 
  : BaseVisitor(CI,file),
    topClass(nullptr)
{
  rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());  
}
