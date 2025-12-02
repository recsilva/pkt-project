#include "llvmvisitor.h"

// Helper function to create a stack allocation in the entry block of a function
llvm::AllocaInst *LLVMVisitor::createEntryBlockAlloca(llvm::Function *function,
                                             const std::string &varName,
                                             llvm::Type *type) {
    // Create a temporary builder pointing to the start of the entry block.
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(),
                             function->getEntryBlock().begin());
    // Create the alloca instruction
    return tmpBuilder.CreateAlloca(type, nullptr, varName);
}

LLVMVisitor::LLVMVisitor(llvm::raw_fd_ostream &out,
                       llvm::LLVMContext &context,
                       std::unique_ptr<llvm::Module> &mod) :
        out(out), context(context), mod(mod), builder(context) {
    ret = nullptr;
    floatInst = false;
}

void LLVMVisitor::visit(ProgramNode *node) {
    // Function returns void.
    llvm::FunctionType *functionReturnType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);

    // Our main function.
    llvm::Function *mainFunction =
        llvm::Function::Create(functionReturnType,
                               llvm::Function::ExternalLinkage,
                               "main",
                               mod.get());

    // Create entry block (for allocations) and body block (for execution)
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", mainFunction);
    llvm::BasicBlock *body = llvm::BasicBlock::Create(context, "body", mainFunction);
    
    // Set the builder insertion point to the start of the function body
    builder.SetInsertPoint(body);

    llvm::IRBuilder<> entryBuilder(entry);
    entryBuilder.CreateBr(body);

    // Setup printf prototype.
    std::vector<llvm::Type *> params;
    // Pointer to int8 would be like char *
    params.push_back(llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0));

    llvm::FunctionType *printfType =
            llvm::FunctionType::get(builder.getInt32Ty(), params, /*isVarArg=*/true);
    llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf",
                       mod.get());

    // Visit all of the statements.
    for (auto statement : node->getStatements())
        statement->accept(*this);

    // Return
    builder.CreateRetVoid();

    // Print to the file!
    mod->print(out, nullptr);
    out.close();
}

void LLVMVisitor::visit(StatementNode *node) {
    node->getExp()->accept(*this);

    // Call printf with our returned value.
    std::vector<llvm::Value *> printArgs;

    // Change the format string depending on if we're dealing with a float or not.
    llvm::Value *formatStr;
    llvm::Value *printVal = ret;

    if (floatInst) {
        // Ensure the value is a double for printing
        if (!printVal->getType()->isDoubleTy())
            printVal = builder.CreateSIToFP(printVal, builder.getDoubleTy());
            
        formatStr = builder.CreateGlobalStringPtr("%f\n");
    } else {
        // Ensure the value is an int32 for printing
        if (printVal->getType()->isDoubleTy()) {
            // If it was a float/double, cast it back to int for %d printing (truncation)
            printVal = builder.CreateFPToSI(printVal, builder.getInt32Ty());
        }
        // If it's already an i32, no change.
        formatStr = builder.CreateGlobalStringPtr("%d\n");
    }

    printArgs.push_back(formatStr);
    printArgs.push_back(printVal);
    builder.CreateCall(mod->getFunction("printf"), printArgs);

    // Reset the float instruction marker.
    floatInst = false;
}

void LLVMVisitor::visit(AssignmentNode *node) {
    // Evaluate the expression on the right-hand side (RHS)
    node->getExp()->accept(*this);
    
    llvm::Value *value = ret;
    llvm::Type *type = value->getType();

    llvm::AllocaInst *allocation = nullptr;
    llvm::Function *parentFunction = builder.GetInsertBlock()->getParent();

    // Check if the variable is already allocated
    if (symbolTable.count(node->getName())) {
        allocation = symbolTable[node->getName()];
        
        // If the type changes (e.g., from int to float), we should re-allocate or cast.
        // For simplicity, we assume we want to store the *current* value's type.
        if (type != allocation->getAllocatedType()) {
             // In a real compiler, type mismatch would be an error or require explicit casting.
             // Here, we proceed to store the current value (LLVM allows storing value of T into pointer to T).
        }
    } else {
        // If it doesn't exist, create a new stack allocation in the entry block.
        allocation = createEntryBlockAlloca(parentFunction, node->getName(), type);
        symbolTable[node->getName()] = allocation;
    }
    
    // Store the result of the expression into the allocated memory.
    builder.CreateStore(value, allocation);
    
    // Assignment is a statement, so we don't need a meaningful return value.
    ret = nullptr; 
}

void LLVMVisitor::visit(ArrayDefNode *node) {
    std::cout << "DEBUG: Visiting Array Definition for " << node->getName() << std::endl;

    // Visit the size expression
    node->getSizeExpression()->accept(*this);
    
    // VERIFY: Check if ret holds the correct size
    if (ret) {
        // We expect ret to be a ConstantInt because array sizes are usually constants
        if (auto *constantInt = llvm::dyn_cast<llvm::ConstantInt>(ret)) {
            int actualValue = constantInt->getSExtValue(); // Extract the raw C++ int
            std::cout << "DEBUG: Size evaluated to: " << actualValue << std::endl;
        } else {
            std::cout << "DEBUG: Size is not a constant integer! It is: ";
            ret->print(llvm::errs()); // Print the LLVM IR representation
            std::cout << std::endl;
        }
    } else {
        std::cout << "DEBUG: Size evaluation returned nullptr!" << std::endl;
    }

    // Visit the initial value
    node->getInitialValue()->accept(*this);
    
    // VERIFY: Check initial value
    if (ret) {
        if (auto *constantInt = llvm::dyn_cast<llvm::ConstantInt>(ret)) {
             std::cout << "DEBUG: Init value is: " << constantInt->getSExtValue() << std::endl;
        }
    }

    // Evaluate Size
    node->getSizeExpression()->accept(*this);
    llvm::Value *sizeValue = ret;
    
    // Ensure size is a constant int (simplified for this step)
    if (!sizeValue->getType()->isIntegerTy()) {
        sizeValue = builder.CreateFPToSI(sizeValue, builder.getInt32Ty());
    }
    llvm::ConstantInt *constantSize = llvm::dyn_cast<llvm::ConstantInt>(sizeValue);
    uint64_t arraySize = constantSize ? constantSize->getZExtValue() : 1;
    
    // Evaluate Initial Value
    node->getInitialValue()->accept(*this);
    llvm::Value *initialValue = ret;
    
    // Allocate Array
    llvm::ArrayType *arrayType = llvm::ArrayType::get(initialValue->getType(), arraySize);
    llvm::Function *parentFunction = builder.GetInsertBlock()->getParent();
    
    llvm::AllocaInst *arrayAllocation = createEntryBlockAlloca(
        parentFunction, 
        node->getName(), 
        arrayType
    );
    symbolTable[node->getName()] = arrayAllocation;

    // --- LOOP SETUP ---
    
    // Create Basic Blocks
    llvm::BasicBlock *loopHeader = llvm::BasicBlock::Create(context, "loop.header", parentFunction);
    llvm::BasicBlock *loopBody = llvm::BasicBlock::Create(context, "loop.body", parentFunction);
    llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(context, "loop.end", parentFunction);

    // Create the counter 'i' on the stack
    llvm::AllocaInst *iPtr = createEntryBlockAlloca(parentFunction, "i", builder.getInt32Ty());
    
    // FIX: Initialize i = 0 BEFORE jumping to the header
    builder.CreateStore(builder.getInt32(0), iPtr);
    
    // Now jump to the start of the loop
    builder.CreateBr(loopHeader);

    // --- LOOP HEADER ---
    builder.SetInsertPoint(loopHeader);
    
    // Load current value of i
    llvm::Value *iCurrent = builder.CreateLoad(builder.getInt32Ty(), iPtr, "i.val");
    
    // Check if i < size
    llvm::Value *condition = builder.CreateICmpSLT(iCurrent, sizeValue, "cmp");
    builder.CreateCondBr(condition, loopBody, loopEnd);

    // --- LOOP BODY ---
    builder.SetInsertPoint(loopBody);
    
    // Get pointer to array[i]
    llvm::Value *iBodyVal = builder.CreateLoad(builder.getInt32Ty(), iPtr);
    llvm::Value *indices[] = { builder.getInt32(0), iBodyVal };
    llvm::Value *elemPtr = builder.CreateInBoundsGEP(arrayType, arrayAllocation, indices, "elem.ptr");
    
    // Store initial value
    builder.CreateStore(initialValue, elemPtr);
    
    // Increment i
    llvm::Value *nextI = builder.CreateAdd(iBodyVal, builder.getInt32(1), "i.inc");
    builder.CreateStore(nextI, iPtr);
    
    // Jump back to header
    builder.CreateBr(loopHeader);

    // --- LOOP END ---
    builder.SetInsertPoint(loopEnd);
    
    ret = nullptr;
}

void LLVMVisitor::visit(ArrayAccessNode *node) {
    // Look up the array in the symbol table
    if (symbolTable.find(node->getName()) == symbolTable.end()) {
        std::cerr << "Error: Array '" << node->getName() << "' not defined." << std::endl;
        ret = builder.getInt32(0); // Return dummy value
        return;
    }
    llvm::AllocaInst *arrayAlloc = symbolTable[node->getName()];

    // Evaluate the Index Expression
    node->getIndexExpression()->accept(*this);
    llvm::Value *indexValue = ret;

    // Ensure Index is an Integer
    if (indexValue->getType()->isDoubleTy()) {
        // Cast float index to int (e.g., array[2.5] -> array[2])
        indexValue = builder.CreateFPToSI(indexValue, builder.getInt32Ty(), "idx.cast");
    }

    // Calculate Address using GEP (GetElementPtr)
    // We need two indices: 
    //   - 0: To dereference the pointer to the array itself.
    //   - indexValue: The actual index we want.
    std::vector<llvm::Value *> indices;
    indices.push_back(builder.getInt32(0)); 
    indices.push_back(indexValue);

    // Get the type of the array being stored (e.g., [100 x i32])
    llvm::Type *arrayType = arrayAlloc->getAllocatedType();

    // Create the GEP instruction to get the pointer to the element
    llvm::Value *elementPtr = builder.CreateInBoundsGEP(
        arrayType, 
        arrayAlloc, 
        indices, 
        "elem.ptr"
    );

    // Load the Value from that address
    // We need to know if we are loading an int or a float
    llvm::ArrayType *arrType = llvm::dyn_cast<llvm::ArrayType>(arrayType);
    llvm::Type *elementType = arrType->getElementType();

    ret = builder.CreateLoad(elementType, elementPtr, "elem.val");

    // Update floatInst flag so parent knows what type 'ret' is
    floatInst = elementType->isDoubleTy();
}

void LLVMVisitor::visit(ArrayAssignNode *node) {
    // Evaluate the RHS (Value to be stored)
    node->getValueExpression()->accept(*this);
    llvm::Value *valueToStore = ret;
    llvm::Type *valueType = valueToStore->getType();
    
    // Look up the array in the symbol table
    if (symbolTable.find(node->getName()) == symbolTable.end()) {
        std::cerr << "Error: Array '" << node->getName() << "' not defined." << std::endl;
        ret = nullptr; 
        return;
    }
    llvm::AllocaInst *arrayAlloc = symbolTable[node->getName()];

    // Evaluate the Index Expression
    node->getIndexExpression()->accept(*this);
    llvm::Value *indexValue = ret;

    // Ensure Index is an Integer
    if (indexValue->getType()->isDoubleTy()) {
        indexValue = builder.CreateFPToSI(indexValue, builder.getInt32Ty(), "idx.cast.assign");
    }

    // Calculate Address using GEP (Same logic as ArrayAccessNode)
    std::vector<llvm::Value *> indices;
    indices.push_back(builder.getInt32(0)); 
    indices.push_back(indexValue);
    
    llvm::Type *arrayType = arrayAlloc->getAllocatedType();

    llvm::Value *elementPtr = builder.CreateInBoundsGEP(
        arrayType, 
        arrayAlloc, 
        indices, 
        "elem.ptr.assign"
    );

    // Type Coercion Check: Ensure the value being stored matches the element type
    llvm::ArrayType *arrType = llvm::dyn_cast<llvm::ArrayType>(arrayType);
    llvm::Type *elementType = arrType->getElementType();

    llvm::Value *finalValueToStore = valueToStore;
    
    if (valueType != elementType) {
        if (valueType->isDoubleTy() && elementType->isIntegerTy(32)) {
            // Float to Int: Truncate/Cast
            finalValueToStore = builder.CreateFPToSI(valueToStore, elementType, "fptosi.coercion");
        } else if (valueType->isIntegerTy(32) && elementType->isDoubleTy()) {
            // Int to Float: Sign Extend to Float
            finalValueToStore = builder.CreateSIToFP(valueToStore, elementType, "sitofp.coercion");
        }
        // Add more coercion rules if needed (e.g., int32 to int64)
    }

    // Store the Value at that Address!
    builder.CreateStore(finalValueToStore, elementPtr);
    
    ret = nullptr; // Assignment is a statement, not an expression result
}

void LLVMVisitor::visit(IdentifierNode *node) {
    // Look up the variable in the symbol table.
    llvm::AllocaInst *allocation = symbolTable[node->getName()];

    if (!allocation) {
        // Error: Undefined variable.
        std::cerr << "Error: Undefined variable '" << node->getName() 
                  << "' (Line " << node->getLine() << ")" << std::endl;
        // Return a zero constant (i32) to prevent a crash
        ret = builder.getInt32(0);
        floatInst = false;
        return; 
    }

    // Load the value from the allocated memory.
    llvm::Type *type = allocation->getAllocatedType();
    
    ret = builder.CreateLoad(type, allocation, node->getName());
    
    // Update the float instruction marker based on the variable's type
    floatInst = type->isDoubleTy();
}

void LLVMVisitor::visit(IntegerNode *node) {
    // Return the LLVM int value.
    ret = llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(context), node->getValue());
}

void LLVMVisitor::visit(FloatNode *node) {
    // Return the LLVM float value.
    ret = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), node->getValue());
    // Mark that we're now dealing with a float.
    floatInst = true;
}

void LLVMVisitor::visit(PlusNode *node) {
    // Get the return value from the left side.
    node->getLeft()->accept(*this);
    llvm::Value *lhs = ret;
    bool lhsIsFloat = floatInst;

    // Get the return value from the right side.
    node->getRight()->accept(*this);
    llvm::Value *rhs = ret;
    bool rhsIsFloat = floatInst;

    if (lhsIsFloat || rhsIsFloat) {
        floatInst = true; 
        // Promote RHS or LHS if we're dealing with floats and they're not a float.
        // (except we use doubles)
        if (!lhs->getType()->isDoubleTy())
            lhs = builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context));
        if (!rhs->getType()->isDoubleTy())
            rhs = builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context));

        ret = builder.CreateFAdd(lhs, rhs);
    } else {
        // Otherwise we're just doing an integer add.
        floatInst = false;
        ret = builder.CreateAdd(lhs, rhs);
    }
}

void LLVMVisitor::visit(UMinusNode *node){
    node->getExp()->accept(*this);
    
    if (ret->getType()->isDoubleTy()){
        ret = builder.CreateFNeg(ret);
    }
    else{
        ret = builder.CreateNeg(ret);
    }
    
}

void LLVMVisitor::visit(MinusNode *node) {
    node->getLeft()->accept(*this);
    llvm::Value *lhs = ret;
    bool lhsIsFloat = floatInst;

    node->getRight()->accept(*this);
    llvm::Value *rhs = ret;
    bool rhsIsFloat = floatInst;

    if (lhsIsFloat || rhsIsFloat) {
        floatInst = true;
        // Promote RHS or LHS if we're dealing with floats and they're not a float.
        // (except we use doubles)
        if (!lhs->getType()->isDoubleTy())
            lhs = builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context));
        if (!rhs->getType()->isDoubleTy())
            rhs = builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context));

        ret = builder.CreateFSub(lhs, rhs);
    } else {
        floatInst = false;
        // Otherwise we're just doing an integer add.
        ret = builder.CreateSub(lhs, rhs);
    }
}

void LLVMVisitor::visit(MultNode *node) {
    // Get the return value from the left side.
    node->getLeft()->accept(*this);
    llvm::Value *lhs = ret;
    bool lhsIsFloat = floatInst;

    // Get the return value from the right side.
    node->getRight()->accept(*this);
    llvm::Value *rhs = ret;
    bool rhsIsFloat = floatInst;

    if (lhsIsFloat || rhsIsFloat) {
        floatInst = true;
        // Promote RHS or LHS if we're dealing with floats and they're not a float.
        // (except we use doubles)
        if (!lhs->getType()->isDoubleTy())
            lhs = builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context));
        if (!rhs->getType()->isDoubleTy())
            rhs = builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context));

        ret = builder.CreateFMul(lhs, rhs);
    } else {
        floatInst = false;
        // Otherwise we're just doing an integer add.
        ret = builder.CreateMul(lhs, rhs);
    }
}

void LLVMVisitor::visit(DivNode *node) {
    // Get the return value from the left side.
    node->getLeft()->accept(*this);
    llvm::Value *lhs = ret;
    bool lhsIsFloat = floatInst;

    // Get the return value from the right side.
    node->getRight()->accept(*this);
    llvm::Value *rhs = ret;
    bool rhsIsFloat = floatInst;

    if (lhsIsFloat || rhsIsFloat) {
        floatInst = true;
        // Promote RHS or LHS if we're dealing with floats and they're not a float.
        // (except we use doubles)
        if (!lhs->getType()->isDoubleTy())
            lhs = builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context));
        if (!rhs->getType()->isDoubleTy())
            rhs = builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context));

        ret = builder.CreateFDiv(lhs, rhs);
    } else {
        floatInst = false;
        // Otherwise we're just doing an integer add.
        ret = builder.CreateSDiv(lhs, rhs);
    }
}