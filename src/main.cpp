#include "clang/Driver/Options.h"


#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <memory>

#include "PrettyVisitor.h"
#include "RawVisitor.h"

namespace Myoptions{
	std::string classToExpand;
	bool addVirtual=false;
	bool addDefaultValue=false;
	bool pretty=false;
};


using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

//===================================================================
class HeaderExpanderConsumer : public ASTConsumer {
private:
  BaseVisitor *visitor; 
public:
  explicit HeaderExpanderConsumer(CompilerInstance *CI, StringRef file)
    {
	if(Myoptions::pretty)
	  visitor=new PrettyVisitor(CI,file);
	else
	  visitor=new RawVisitor(CI,file);
    }

  virtual void HandleTranslationUnit(ASTContext &Context) {
    visitor->TraverseDecl(Context.getTranslationUnitDecl());
  }
};



class HeaderExpanderFrontendAction : public ASTFrontendAction {
public:
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
	// pass CI pointer to ASTConsumer
	return std::make_unique<HeaderExpanderConsumer>(&CI,file); 
  }
};


int main(int argc, const char **argv) {


  cl::opt<std::string> optClassToExpand("class-to-expand", cl:: NotHidden,cl::desc("Class to Expand"));
  cl::opt<bool> optAddDefaultValue("add-default-value", cl:: NotHidden,cl::desc("If set, add a comment to remind the default value"));
  cl::opt<bool> optAddVirtual("add-remind-virtual", cl:: NotHidden,cl::desc("If set, add a comment to remind the function is virtual"));
  cl::opt<bool> optPretty("pretty-expand", cl:: NotHidden,cl::desc("If set, do a pretty expad"));
  cl::OptionCategory MyToolCategory("My tool options");
 
  // parse the command-line args passed to your code
  CommonOptionsParser op(argc, argv,MyToolCategory);   

  // get back options' values 
  Myoptions::classToExpand=optClassToExpand;
  Myoptions::addVirtual= optAddVirtual;
  Myoptions::addDefaultValue= optAddDefaultValue;
  Myoptions::pretty  = optPretty;

  std::cout<<"===Go==="<<std::endl;

  const auto& sources=op.getSourcePathList();
  if(sources.size()!=1){
    return -1;
  }
  
  // create a new Clang Tool instance (a LibTooling environment)
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    
  // run the Clang Tool, creating a new FrontendAction (explained below)
  int result = Tool.run(
      newFrontendActionFactory<HeaderExpanderFrontendAction>().get()
      );

  return result;
}
