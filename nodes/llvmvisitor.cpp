#include "llvmvisitor.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

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
        llvm::FunctionType::get(builder.getInt32Ty(), false);
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
    {
        // printf("statement at: %i\n", statement->getLine());
        statement->accept(*this);
    }

    // Return
    llvm::Value *zero = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
    builder.CreateRet(zero);

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

    if (!ret){return;}
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

void LLVMVisitor::visit(IfNode *node) {
    // 1. Evaluate Condition
    node->getCondition()->accept(*this);
    llvm::Value *conditionValue = ret;

    if (!conditionValue->getType()->isIntegerTy(1)) {
        conditionValue = builder.CreateICmpNE(conditionValue, llvm::ConstantInt::get(conditionValue->getType(), 0), "tobool");
    }

    llvm::Function *parentFunction = builder.GetInsertBlock()->getParent();
    
    // 2. Create Blocks
    llvm::BasicBlock *trueBlock = llvm::BasicBlock::Create(context, "if.true", parentFunction);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context, "if.merge", parentFunction);
    
    // 3. Branch to True/Merge
    builder.CreateCondBr(conditionValue, trueBlock, mergeBlock);
    
    // 4. Generate True Block
    builder.SetInsertPoint(trueBlock);
    
    for (StatementNode *stmt : *node->getTrueBlock()) {
        stmt->accept(*this);
    }

    // *** THE FIX ***
    // Instead of checking 'trueBlock', check the builder's CURRENT insert block.
    // If the last statement was a nested IF, the builder is now sitting in 'inner.merge',
    // which needs to be connected to 'outer.merge'.
    llvm::BasicBlock *currentBlock = builder.GetInsertBlock();
    
    if (!currentBlock->getTerminator()) {
        builder.CreateBr(mergeBlock);
    }
    
    // 5. Continue at Merge Block
    builder.SetInsertPoint(mergeBlock);
    ret = nullptr;
}

void LLVMVisitor::visit(ComparisonNode *node) {
    // 1. Evaluate LHS and RHS
    node->getLeft()->accept(*this);
    llvm::Value *L = ret;

    node->getRight()->accept(*this);
    llvm::Value *R = ret;

    llvm::Value *L_final = L;
    llvm::Value *R_final = R;

    // 2. Check if this should be a floating point comparison
    // If EITHER operand is a double, we must promote the other one to double.
    bool useFloatComparison = L->getType()->isDoubleTy() || R->getType()->isDoubleTy();

    if (useFloatComparison) {
        // Ensure L is a double
        if (!L->getType()->isDoubleTy()) {
            L_final = builder.CreateSIToFP(L, builder.getDoubleTy(), "castL");
        }
        
        // Ensure R is a double
        if (!R->getType()->isDoubleTy()) {
            R_final = builder.CreateSIToFP(R, builder.getDoubleTy(), "castR");
        }

        // 3a. Perform Floating Point Comparison
        llvm::CmpInst::Predicate pred;
        switch (node->getOp()) {
            case ComparisonNode::OpType::LT: pred = llvm::CmpInst::FCMP_OLT; break;
            case ComparisonNode::OpType::GT: pred = llvm::CmpInst::FCMP_OGT; break;
            case ComparisonNode::OpType::LE: pred = llvm::CmpInst::FCMP_OLE; break;
            case ComparisonNode::OpType::GE: pred = llvm::CmpInst::FCMP_OGE; break;
            case ComparisonNode::OpType::EQ: pred = llvm::CmpInst::FCMP_OEQ; break;
            case ComparisonNode::OpType::NE: pred = llvm::CmpInst::FCMP_ONE; break;
        }
        ret = builder.CreateFCmp(pred, L_final, R_final, "cmpF");

    } else {
        // 3b. Perform Integer Comparison (Both are i32)
        llvm::CmpInst::Predicate pred;
        switch (node->getOp()) {
            case ComparisonNode::OpType::LT: pred = llvm::CmpInst::ICMP_SLT; break;
            case ComparisonNode::OpType::GT: pred = llvm::CmpInst::ICMP_SGT; break;
            case ComparisonNode::OpType::LE: pred = llvm::CmpInst::ICMP_SLE; break;
            case ComparisonNode::OpType::GE: pred = llvm::CmpInst::ICMP_SGE; break;
            case ComparisonNode::OpType::EQ: pred = llvm::CmpInst::ICMP_EQ; break;
            case ComparisonNode::OpType::NE: pred = llvm::CmpInst::ICMP_NE; break;
        }
        ret = builder.CreateICmp(pred, L_final, R_final, "cmpI");
    }
    
    // Result is always i1 (boolean), so future operations shouldn't treat it as float
    floatInst = false;
}

// Helper function to search for the best function overload
llvm::Function* LLVMVisitor::getFunctionOverload(const std::string& name, const std::vector<llvm::Type*>& argTypes) {
    auto it = functionTable.find(name);
    if (it == functionTable.end()) {
        return nullptr;
    }

    for (llvm::Function* F : it->second) {
        if (F->arg_size() != argTypes.size()) {
            continue; 
        }

        bool match = true;
        unsigned i = 0;
        for (auto &Arg : F->args()) {
            llvm::Type* paramType = Arg.getType();
            llvm::Type* argType = argTypes[i];

            // For simplicity, we check for exact match OR int->float coercion possibility
            if (paramType != argType) {
                if (!(paramType->isFloatingPointTy() && argType->isIntegerTy())) {
                    match = false;
                    break;
                }
            }
            i++;
        }

        if (match) {
            return F; // Found a matching overload
        }
    }
    return nullptr; // No matching overload found
}


// --- Function Definition ---
void LLVMVisitor::visit(FunctionDefNode *node) {
    // printf("FUNCTION DEFINITION, line no: %i\n", node->getLine());
    // std::cout << "     NAME: " << node->getName() << std::endl;

    // Save current state (required for nesting functions and statements)
    llvm::BasicBlock *oldBlock = builder.GetInsertBlock();
    auto outerSymbolTable = symbolTable;
    symbolTable.clear(); // Initialize fresh symbol table for new function scope

    // 1. Determine function signature (We assume 'float' return type for simplicity)
    llvm::Type *retType = builder.getFloatTy();
    std::vector<llvm::Type*> paramTypes;
    

    // Determine parameter types based on default values
    for (ParamDefNode *paramDef : *node->getParams()) {
        llvm::Type *inferredType = builder.getFloatTy(); // Default assumption (float)

        if (paramDef->getDefaultValue()) {
            // Temporarily evaluate the default expression *without* side effects
            // This is complex in a real compiler, here we'll simulate type inference:
            
            // A clean way would be to create a temporary visitor to evaluate 
            // the default expression and observe its type. Since we can't easily 
            // spin up a new visitor, we'll run the expression and check its type 
            // before generating the function.

            // HACK: Re-run the expression visitor temporarily to see the type
            // This requires careful state management to avoid polluting `ret`.
            
            llvm::Value *tempRet = nullptr;
            bool tempFloatInst = false;
            
            // Save global state
            llvm::Value *originalRet = ret;
            bool originalFloatInst = floatInst;

            paramDef->getDefaultValue()->accept(*this);
            tempRet = ret;
            tempFloatInst = floatInst;

            // Restore global state
            ret = originalRet;
            floatInst = originalFloatInst;
            builder.SetInsertPoint(oldBlock); // Restore builder position too

            if (tempRet) {
                inferredType = tempRet->getType();
            }
        }
        
        paramTypes.push_back(inferredType); 
    }
    
    llvm::FunctionType *funcType = llvm::FunctionType::get(retType, paramTypes, false);
    
    // 2. Create the LLVM Function and register it
    llvm::Function *F = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, node->getName(), mod.get());
    
    functionTable[node->getName()].push_back(F);
    
    // 3. Set up parameters and entry block
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "entry", F);
    builder.SetInsertPoint(entryBlock);

    auto paramIt = F->arg_begin();
    auto paramDefIt = node->getParams()->begin();

    for (; paramIt != F->arg_end(); ++paramIt, ++paramDefIt) {
        ParamDefNode *paramDef = *paramDefIt;
        const std::string& paramName = paramDef->getName();
        paramIt->setName(paramName);
        
        // Allocate stack space and store the passed argument (making it a local variable)
        llvm::AllocaInst *Alloca = createEntryBlockAlloca(F, paramName, paramIt->getType());
        builder.CreateStore(paramIt, Alloca);
        symbolTable[paramName] = Alloca;
    }

    // 4. Visit the function body
    for (StatementNode *stmt : *node->getBody()) {
        stmt->accept(*this);
    }
    
    // 5. Ensure function is terminated
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRet(llvm::ConstantFP::get(context, llvm::APFloat(0.0f)));
    }

    // 6. Restore previous context
    symbolTable = outerSymbolTable;
    if (oldBlock) {
        builder.SetInsertPoint(oldBlock);
    }
    ret = nullptr; 
}

// --- Function Call ---
void LLVMVisitor::visit(FunctionCallNode *node) {
    // printf("FUNCTION INVOCATION, line no: %i\n", node->getLine());
    // std::cout << "     NAME: " << node->getName() << std::endl;

    std::vector<llvm::Value*> argValues;
    std::vector<llvm::Type*> argTypes;

    // 1. Evaluate all arguments and collect types
    for (ExpNode *arg : *node->getArgs()) {
        arg->accept(*this);
        argValues.push_back(ret);
        argTypes.push_back(ret->getType());
    }

    // 2. Resolve Overload
    llvm::Function *F = getFunctionOverload(node->getName(), argTypes);

    if (!F) {
        std::cerr << "Error (Line " << node->getLine() << "): No matching function overload found for '" << node->getName() << "'." << std::endl;
        ret = llvm::ConstantFP::get(context, llvm::APFloat(0.0f)); 
        floatInst = true;
        return;
    }

    // 3. Generate the Call Instruction with necessary coercion
    std::vector<llvm::Value*> finalArgs;
    auto paramIt = F->arg_begin();

    for (size_t i = 0; i < argValues.size(); ++i) {
        llvm::Value* argVal = argValues[i];
        llvm::Type* paramType = paramIt->getType();
        
        if (argVal->getType() != paramType) {
            // Coerce integer arguments to float parameters
            if (argVal->getType()->isIntegerTy() && paramType->isFloatingPointTy()) {
                argVal = builder.CreateSIToFP(argVal, paramType, "castToFloat");
            } 
        }
        finalArgs.push_back(argVal);
        paramIt++;
    }

    ret = builder.CreateCall(F, finalArgs, "calltmp");
    floatInst = ret->getType()->isFloatingPointTy();
    ret = nullptr; 
}

void LLVMVisitor::visit(WhileNode *node) {
    llvm::Function *parentFunction = builder.GetInsertBlock()->getParent();

    // 1. Create the three core blocks
    llvm::BasicBlock *loopCondBlock = llvm::BasicBlock::Create(context, "loop.cond", parentFunction);
    llvm::BasicBlock *loopBodyBlock = llvm::BasicBlock::Create(context, "loop.body", parentFunction);
    llvm::BasicBlock *loopAfterBlock = llvm::BasicBlock::Create(context, "loop.after", parentFunction);
    
    // 2. Terminate the block immediately preceding the loop (jump to condition)
    builder.CreateBr(loopCondBlock);
    
    // 3. Emit the Loop Condition Block
    builder.SetInsertPoint(loopCondBlock);
    
    // Evaluate the condition expression
    node->getCondition()->accept(*this);
    llvm::Value *conditionValue = ret;

    // Coerce the condition result to i1 (boolean) if necessary
    if (!conditionValue->getType()->isIntegerTy(1)) {
        conditionValue = builder.CreateICmpNE(conditionValue, 
                                             llvm::ConstantInt::get(conditionValue->getType(), 0), 
                                             "tobool.loop");
    }
    
    // Branch based on condition: to body if true, to after if false
    builder.CreateCondBr(conditionValue, loopBodyBlock, loopAfterBlock);

    // 4. Emit the Loop Body Block
    builder.SetInsertPoint(loopBodyBlock);

    // Generate code for all statements in the loop body
    for (StatementNode *stmt : *node->getLoopBody()) {
        stmt->accept(*this);
    }
    
    // CRITICAL: Ensure the body block loops back to the condition!
    // Check if the loop body already contains a terminator (e.g., a nested if/return)
    llvm::BasicBlock *currentBlock = builder.GetInsertBlock();
    if (!currentBlock->getTerminator()) {
        builder.CreateBr(loopCondBlock);
    }

    // 5. Emit the After Block
    // The rest of the program continues from here
    builder.SetInsertPoint(loopAfterBlock);
    ret = nullptr; 
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