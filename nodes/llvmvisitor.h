#ifndef LLVMVISITOR_H
#define LLVMVISITOR_H

#include <fstream>
#include <map>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

#include "visitor.h"

class LLVMVisitor : public Visitor {
public:
    LLVMVisitor(llvm::raw_fd_ostream &out,
               llvm::LLVMContext &context,
               std::unique_ptr<llvm::Module> &mod);
    void visit(ProgramNode *node);
    void visit(StatementNode *node);
    
    void visit(IntegerNode *node);
    void visit(FloatNode *node);

    void visit(PlusNode *node);
    void visit(UMinusNode *node);
    void visit(MinusNode *node);
    void visit(MultNode *node);
    void visit(DivNode *node);

    void visit(IdentifierNode *node);
    void visit(AssignmentNode *node);
    void visit(ArrayDefNode *node);
    void visit(ArrayAccessNode *node);
    void visit(ArrayAssignNode *node);

    void visit(ComparisonNode *node);
    void visit(IfNode *node);

private:
    llvm::raw_fd_ostream &out;
    llvm::LLVMContext &context;
    std::unique_ptr<llvm::Module> &mod;
    llvm::IRBuilder<> builder;

    // Symbol table mapping variable name to its memory allocation (AllocaInst*)
    std::map<std::string, llvm::AllocaInst *> symbolTable; // <-- NEW

    // The "return value" from the previous visit method.
    llvm::Value *ret;
    bool floatInst;

    // Helper function to create stack allocation in the entry block
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function,
                                             const std::string &varName,
                                             llvm::Type *type); // <-- NEW
};

#endif