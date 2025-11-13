#include <vector>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"

int main() {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> mod;
    mod = std::make_unique<llvm::Module>("ECTLang module", context);

    // Build IR for this block.
    llvm::IRBuilder<> builder(context);

    // Function returns void.
    llvm::FunctionType *functionReturnType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);

    // Our main function.
    llvm::Function *mainFunction =
        llvm::Function::Create(functionReturnType,
                               llvm::Function::ExternalLinkage,
                               "main",
                               mod.get());

    // Now make a basic block inside of mainFunction.
    llvm::BasicBlock *body = llvm::BasicBlock::Create(context, "body", mainFunction);
    builder.SetInsertPoint(body);
    llvm::Type *intTy = llvm::Type::getInt32Ty(context);
    // Make a simple add with two constants.
    llvm::Constant *x = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), 5.5);
    llvm::Constant *y = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), -6.5);
    llvm::Value *add = builder.CreateFAdd(x, y, "addtest");

    std::vector<llvm::Type*> params;
    // Pointer to int8 would be like char *
    params.push_back(llvm::PointerType::get(llvm::Type::getInt8Ty(context),0));

    llvm::FunctionType *printfType =
            llvm::FunctionType::get(builder.getInt32Ty(), params, /*isVarArg=*/true);
    llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf",
                       mod.get());

    // Function call arguments
    std::vector<llvm::Value *> printArgs;
    llvm::Value *formatStr = builder.CreateGlobalStringPtr("%f\n");
    printArgs.push_back(formatStr);
    printArgs.push_back(add);
    builder.CreateCall(mod->getFunction("printf"), printArgs);

    builder.CreateRetVoid();
    mod->print(llvm::outs(), nullptr);

    return 0;
}