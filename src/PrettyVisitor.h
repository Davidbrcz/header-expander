#ifndef PRETTY_VISITOR
#define PRETTY_VISITOR

#include "BaseVisitor.h"

//Write pretty classes. If classe is namespace BB{ class C{void foo ();} }
// it will create  namespace BB{void C::foo(){}} instead of void BB::C::foo(){}
// Il will also create (nested) namespaces if it dont exist
// and add at the end the definitions of new functions
class PrettyVisitor : public BaseVisitor {
		//handle opening and closing namespace if necessary 
		// and otherwhise flushes the rewriter
		class NamespaceHandler
		{
			PrettyVisitor& upper_this;
		public:
			NamespaceHandler(PrettyVisitor& up):upper_this(up)
			{
				if(!upper_this.enclosingNSRedeclaration)
				{
				      for(auto ns  : upper_this.namespaces)
				      {
					upper_this.outputFile<<"namespace "<<ns<<" {"<<std::endl;
				      }    
				}
			}
			~NamespaceHandler()
			{
				if(!upper_this.enclosingNSRedeclaration)
				{
				      //closse namespace
				      for(std::size_t i =0;i <upper_this.namespaces.size();++i)
				      {
					upper_this.outputFile<<"}"<<std::endl;
				      }    
				}
				else
				{
					upper_this.rewriter.overwriteChangedFiles();
				}
			}
		};
protected:
  clang::CXXRecordDecl* topClass;
  clang::NamespaceDecl* enclosingNSRedeclaration;
  clang::Rewriter rewriter;
  std::vector<std::string> namespaces;

  //check for nested-namespace
  //requieres that NestedClassesCheck have ben performed before
  std::vector<std::string> NestedNamespaceCheck();

   //check for nested-namespace
   std::string NestedClassesCheck();

  void generateAFunctions();
public: 
  explicit PrettyVisitor(clang::CompilerInstance *CI,llvm::StringRef file);
};

#endif
