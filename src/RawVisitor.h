#ifndef RAW_VISITOR
#define RAW_VISITOR

#include "BaseVisitor.h"
class RawVisitor : public BaseVisitor {
private:
    void generateAFunctions()
    {      
      const std::string base=ctx->getQualifiedNameAsString()+"::";      
      for(auto fct = ctx->method_begin();fct!=ctx->method_end();++fct)
      {
        if(!fct->hasBody() && fct->isUserProvided() && !fct->isPure())
        {
          std::string r2=generateAFunction(base,*fct);
          outputFile<<r2<<std::endl;
        }
      }
    }
public: 
  explicit RawVisitor(clang::CompilerInstance *CI,llvm::StringRef file) 
    : BaseVisitor(CI,file)
    {}
};

#endif
