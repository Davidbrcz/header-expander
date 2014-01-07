#ifndef BASE_VISITOR
#define BASE_VISITOR

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

#include <fstream>
#include <string>


#include "options.h"

class BaseVisitor : public clang::RecursiveASTVisitor<BaseVisitor> {
	struct TemplateInfoHolding
	{
		bool isTemplateClass=false;
  		clang::ClassTemplateDecl* templateClass=nullptr;
		std::string baseTemplate="template ";
		std::string listTemplate="<";
	};

	TemplateInfoHolding templateInfos;
protected:
  // hold additionnal info
  // is used to extract source code from file
  // and to check if a function is noexcept	
  clang::ASTContext& astContext; 

  //pointer to the class we have to expand
  clang::CXXRecordDecl* ctx;
 
 
  //file where we can write changes
  std::ofstream outputFile;

  //return the code between begin and end
  //main idea was taken from SO
  std::string pos2str(clang::SourceLocation begin,clang::SourceLocation end);

  	
  virtual void generateAFunctions() =0;
  
 //generate the return type for fct
  std::string ReturnType(clang::CXXMethodDecl* fct);

  //generate the list of aguments for fct
  std::string Arguments(clang::CXXMethodDecl* fct, bool addDefaultValue);

   //generate the exception specifier for fct
   std::string ExceptionSpecification(clang::CXXMethodDecl* fct);
   std::string generateAFunction(std::string base,clang::CXXMethodDecl* fct);
   void GenerateTemplateInfos();

   std::string TemplateList(clang::CXXMethodDecl* fct);
public: 
  explicit BaseVisitor(clang::CompilerInstance *CI,llvm::StringRef file);
  virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *dd);

  //add a definition for static member 
  virtual bool VisitVarDecl(clang::VarDecl* var);
  
};

#endif
