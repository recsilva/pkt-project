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

    declareSystemFunctions();
}


void LLVMVisitor::declareSystemFunctions(){

    llvm::FunctionType *strerrorType = llvm::FunctionType::get(
        builder.getInt8Ty()->getPointerTo(),
        {builder.getInt32Ty()},
        false
    );
    llvm::Function::Create(strerrorType, llvm::Function::ExternalLinkage, "strerror",mod.get());

    //printf function
    llvm::FunctionType *printfType = llvm::FunctionType::get(
        builder.getInt32Ty(), //return type
        builder.getInt8Ty()->getPointerTo(),//param types
        true //isVarArg | idk who dis
    );
    llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf",mod.get());


    //open function
    llvm::FunctionType *openType = llvm::FunctionType::get(
        builder.getInt32Ty(),
        {builder.getInt8Ty()->getPointerTo(),builder.getInt32Ty(),builder.getInt32Ty() },
        false
    );
    llvm::Function::Create(openType, llvm::Function::ExternalLinkage, "open",mod.get());
    
    //read function
    llvm::FunctionType *readType = llvm::FunctionType::get(
        builder.getInt32Ty(),
        {builder.getInt32Ty(),builder.getInt8Ty()->getPointerTo(),builder.getInt32Ty()},
        false
    );
    llvm::Function::Create(readType, llvm::Function::ExternalLinkage, "read",mod.get());
    
    //write function
    llvm::FunctionType *writeType = llvm::FunctionType::get(
        builder.getInt32Ty(),
        {builder.getInt32Ty(),builder.getInt8Ty()->getPointerTo(),builder.getInt32Ty() },
        false
    );
    llvm::Function::Create(writeType, llvm::Function::ExternalLinkage, "write",mod.get());
    
    //close function
    llvm::FunctionType *closeType = llvm::FunctionType::get(
        builder.getInt32Ty(),
        builder.getInt32Ty(),
        false
    );
    llvm::Function::Create(closeType, llvm::Function::ExternalLinkage, "close",mod.get());
    

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
    
    //IDK WHAT THE BIG MAN DO NOW, THERES A PRINT NODE, BROS LEFT WITHOUT A JOB

    node->getExp()->accept(*this);

    // // Call printf with our returned value.
    // std::vector<llvm::Value *> printArgs;

    // if (!ret){return;}

    // llvm::Value *formatStr;
    // llvm::Value *printVal = ret;

    // if (floatInst) {
    //     //if float make sure its a double and change to %f
    //     if (!printVal->getType()->isDoubleTy())
    //         printVal = builder.CreateSIToFP(printVal, builder.getDoubleTy());

            
    //     formatStr = builder.CreateGlobalStringPtr("%f\n");

    //     //if its a pointer and i8 length change to %s for strings
    // } else if (printVal->getType()->isPointerTy()){
    //     llvm::PointerType *ptrType = llvm::dyn_cast<llvm::PointerType>(printVal->getType());

    //     if (printVal->getType() == builder.getInt8Ty()->getPointerTo()){
    //         formatStr = builder.CreateGlobalStringPtr("%s\n");
    //     }

    //     //if its length of i8 change to %c for chars
    // } else if (printVal->getType()->isIntegerTy(8)){
    //     formatStr = builder.CreateGlobalStringPtr("%c\n");

    //     //default for integers, make sure its int and change to %d
    // } else {

    //     if (printVal->getType()->isDoubleTy()){
    //         printVal = builder.CreateFPToSI(printVal,builder.getInt32Ty());
    //     }

    //     formatStr = builder.CreateGlobalStringPtr("%d\n");
    // }

    // printArgs.push_back(formatStr);
    // printArgs.push_back(printVal);

    // //builder.CreateCall(mod->getFunction("magic"),printArgs);
    // builder.CreateCall(mod->getFunction("printf"), printArgs);

    // // Reset the float instruction marker.
    // floatInst = false;
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

void LLVMVisitor::visit(TypeNode *node) {
    // Reset the type result before processing
    currentType = nullptr; 
    printf("arg-type: ");
    switch (node->getTypeId()) {
        case TypeNode::INT:
            currentType = builder.getInt32Ty(); // Use i32 for simplicity
            printf("INT ");
            break;
        case TypeNode::FLT:
            currentType = builder.getDoubleTy();
            printf("FLOAT ");
            break;
        case TypeNode::INT_ARRAY:
            // Arrays are passed as pointers to their element type (i32*)
            currentType = llvm::PointerType::get(builder.getInt32Ty(), 0);
            printf("INT[] ");
            break;
        case TypeNode::FLT_ARRAY:
            // Arrays are passed as pointers to their element type (float*)
            currentType = llvm::PointerType::get(builder.getDoubleTy(), 0);
            printf("FLOAT[] ");
            break;
        case TypeNode::CHR:
            // Characters are often represented as a small integer (i8)
            currentType = builder.getInt8Ty();
            printf("CHAR ");
            break;
        case TypeNode::STRING:
            // Strings are usually passed as a pointer to i8 (i8*)
            currentType = llvm::PointerType::get(builder.getInt8Ty(), 0);
            printf("STRING ");
            break;
        case TypeNode::VOID:
            // Used for function return types that don't return a value
            currentType = builder.getVoidTy();
            printf("VOID ");
            break;
        default:
            // currentType remains nullptr for error handling
            std::cerr << "Error: Unhandled TypeId in TypeNode visitor." << std::endl;
            break; 
    }
    printf("\n");
}

void LLVMVisitor::visit(FunctionDefNode *node) {
    // Save current state (required for nesting functions and statements)
    llvm::BasicBlock *oldBlock = builder.GetInsertBlock();
    auto outerSymbolTable = symbolTable;

    symbolTable.clear(); // Initialize fresh symbol table for new function scope

    // 1. Determine function signature
    // NOTE: If you add explicit return types (e.g., 'def int main(){...}'), 
    // you would also visit the return type here. Sticking to current float assumption.
    llvm::Type *retType = builder.getFloatTy(); 
    
    std::vector<llvm::Type*> paramTypes;
    
    // REPLACING THE HACK WITH EXPLICIT TYPE VISITING
    // Determine parameter types using the explicit TypeNode*
    for (ParamDefNode *paramDef : *node->getParams()) {
        
        // 1. Visit the TypeNode to populate currentType
        paramDef->getType()->accept(*this);
        
        // 2. Retrieve the result
        llvm::Type *paramLLVMType = currentType;
        
        // Error check
        if (!paramLLVMType) {
            // Handle error and return
            return; 
        }
        
        paramTypes.push_back(paramLLVMType); 
    }
    // END OF CLEAN TYPE RESOLUTION
    
    llvm::FunctionType *funcType = llvm::FunctionType::get(retType, paramTypes, false);
    
    // 2. Create the LLVM Function and register it (No Change)
    llvm::Function *F = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, node->getName(), mod.get());
    
    functionTable[node->getName()].push_back(F);
    
    // 3. Set up parameters and entry block (No Change)
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "entry", F);
    builder.SetInsertPoint(entryBlock);

    auto paramIt = F->arg_begin();
    auto paramDefIt = node->getParams()->begin();

    for (; paramIt != F->arg_end(); ++paramIt, ++paramDefIt) {
        ParamDefNode *paramDef = *paramDefIt;
        const std::string& paramName = paramDef->getName();
        paramIt->setName(paramName);
        
        // Allocate stack space and store the passed argument (making it a local variable)
        // The type (paramIt->getType()) is now guaranteed to be correct from step 1.
        llvm::AllocaInst *Alloca = createEntryBlockAlloca(F, paramName, paramIt->getType());
        builder.CreateStore(paramIt, Alloca);
        symbolTable[paramName] = Alloca;
    }

    // 4. Visit the function body (No Change)
    for (StatementNode *stmt : *node->getBody()) {
        stmt->accept(*this);
    }
    
    // 5. Ensure function is terminated (No Change)
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRet(llvm::ConstantFP::get(context, llvm::APFloat(0.0f)));
    }

    // 6. Restore previous context (No Change)
    symbolTable = outerSymbolTable;
    if (oldBlock) {
        builder.SetInsertPoint(oldBlock);
    }
    ret = nullptr; 
}

void LLVMVisitor::visit(FunctionCallNode *node) {
    printf("--------- FUNCTION CALL VISIT -----------\n");
    // 1. Evaluate all arguments and collect values/types
    std::vector<llvm::Value*> argValues;
    std::vector<llvm::Type*> argTypes;

    std::cout<< " | ";
    for (ExpNode *arg : *node->getArgs()) {
        // 1. Visit the argument expression
        arg->accept(*this); 
        if (!ret) { /* ... error handling ... */ return; }

         printf("arg name: \"%s\" ", ret->getName());

        llvm::Value* argVal = ret;

        // check if array
        if (argVal->getType()->isArrayTy()){
            printf("deference array");
            
            llvm::Type *arrType = argVal->getType()->getArrayElementType();
            
            std::vector<llvm::Value*> indices;
            indices.push_back(builder.getInt32(0)); // Dereference array ptr
            indices.push_back(builder.getInt32(0)); // Index 0

            argVal = builder.CreateInBoundsGEP(arrType, argVal, indices, "arraydecay");
        }

        // currentArgType = llvm::PointerType::getUnqual(elemTy);
        
        argValues.push_back(argVal);
        argTypes.push_back(argVal->getType());
        // --- END: New Logic for Array-to-Pointer Decay ---
        
        // ... (Debug printing and ret = nullptr) ...
        std::string type_str;
        llvm::raw_string_ostream rso(type_str);
        argVal->print(rso); // Print the DECAYED type
        std::cout << rso.str() << " ";
        
        ret = nullptr; 
    }
    std::cout<< "\n";

    // 2. Resolve Overload
    // This function must find the best match in your functionTable based on name and exact type signature.
    llvm::Function *F = getFunctionOverload(node->getName(), argTypes);

    if (!F) {
        std::cerr << "Error (Line " << node->getLine() << "): No matching function overload found for '" << node->getName() << "' with the provided positional argument types." << std::endl;
        // Set error return value (e.g., 0.0f) and exit
        ret = llvm::ConstantFP::get(context, llvm::APFloat(0.0f)); 
        floatInst = true;
        return;
    }

    // 3. Generate the Call Instruction with necessary coercion
    std::vector<llvm::Value*> finalArgs;
    
    // Get iterator for the target function's parameters
    auto paramIt = F->arg_begin();

    for (size_t i = 0; i < argValues.size(); ++i) {
        llvm::Value* argVal = argValues[i];
        llvm::Type* paramType = paramIt->getType();
        
        // Check for type mismatch that needs coercion (e.g., i32 -> float)
        if (argVal->getType() != paramType) {
            
            // Coerce integer arguments to float parameters (SIToFP: Signed Integer to Float)
            if (argVal->getType()->isIntegerTy() && paramType->isFloatingPointTy()) {
                argVal = builder.CreateSIToFP(argVal, paramType, "castToFloat");
            } 
            else {
                 std::cerr << "Warning (Line " << node->getLine() << "): Unhandled type coercion needed for argument " << i << "." << std::endl;
            }
        }
        
        finalArgs.push_back(argVal);
        
        // Move to the next parameter for the next argument
        paramIt++; 
    }
    
    // After the loop, check if the argument count matches the parameter count
    if (paramIt != F->arg_end()) {
        std::cerr << "Error (Line " << node->getLine() << "): Too few arguments provided for function '" << node->getName() << "'." << std::endl;
        // ... Error cleanup ...
        return;
    }


    // 4. Generate the Call Instruction
    ret = builder.CreateCall(F, finalArgs, "calltmp");
    floatInst = ret->getType()->isFloatingPointTy();
    printf("----------------------------------------\n");
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

    bool alreadyStored = false;
    // Check if the variable is already allocated
    if (symbolTable.count(node->getName())) {
        allocation = symbolTable[node->getName()];
        
        // If the type changes (e.g., from int to float), we should re-allocate or cast.
        // For simplicity, we assume we want to store the *current* value's type.
        if (type != allocation->getAllocatedType()) {


            if (type == llvm::ArrayType::get(builder.getInt8Ty(),type->getArrayNumElements()) && allocation->getAllocatedType()->isPointerTy()){
                
                // this if means that a char[] is being asigned to a string, time to cook some magic #no ai used here XD!


                llvm::AllocaInst *arrAllocation =  symbolTable[dynamic_cast<IdentifierNode*>(node->getExp())->getName()];

                

                std::cout << node->getLine() << " " << dynamic_cast<IdentifierNode*>(node->getExp())->getName() << std::endl;

                std::vector<llvm::Value*> indices1;
                indices1.push_back(builder.getInt64(0)); 
                indices1.push_back(builder.getInt64(0)); 

                llvm::Value* buffer_ptr = builder.CreateInBoundsGEP(
                    arrAllocation->getAllocatedType(),
                    arrAllocation,
                    indices1,
                    "buffer_ptr"
                );


                builder.CreateStore(
                    buffer_ptr,
                    allocation
                );

                alreadyStored = true;





            }
            


             // In a real compiler, type mismatch would be an error or require explicit casting.
             // Here, we proceed to store the current value (LLVM allows storing value of T into pointer to T).
        }
    } else {
        // If it doesn't exist, create a new stack allocation in the entry block.
        allocation = createEntryBlockAlloca(parentFunction, node->getName(), type);
        symbolTable[node->getName()] = allocation;
    }
    
    // Store the result of the expression into the allocated memory.
    if (!alreadyStored){
    builder.CreateStore(value, allocation);

    }
    
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

    llvm::Value* elemVal = builder.CreateLoad(arrayType, arrayAllocation);

    // symbolTable["ar"]->getAllocatedType()->print(llvm::errs());

    printf(" - array created\n");

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

    // std::cout << "hello" << std::endl;

    // llvm::ArrayType* aT = llvm::dyn_cast<llvm::ArrayType>(elemPtr->getType());   
    // std::cout << "array type: " << aT << std::endl;
    // arrayType->print(llvm::errs());

    // std::cout << "hello" << std::endl;
    
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

    printf("--------- ARRAY ACCESS VISIT -----------\n");

    // Look up the array in the symbol table
    if (symbolTable.find(node->getName()) == symbolTable.end()) {
        std::cerr << "Error: Array '" << node->getName() << "' not defined." << std::endl;
        ret = builder.getInt32(0); // Return dummy value
        return;
    }

    std::cout << "array name: " << node->getName() << std::endl;

    std::cout << "table entry count: " << symbolTable.size() << std::endl;

    for (const auto& pair : symbolTable) {
        // printf("iterating");
        try{
            std::cout << " (" << pair.first << ") ";
            pair.second->print(llvm::errs());
            std::cout << "\n";
        }
        catch(int err){
            printf("access to unassigned value");
        }
    }   

    llvm::AllocaInst *arrayAlloc = symbolTable[node->getName()];
    llvm::Value *ptr = arrayAlloc;

    if (arrayAlloc->getAllocatedType()->isPointerTy()){

        ptr = builder.CreateLoad(arrayAlloc->getAllocatedType(), arrayAlloc, "dereferenced");
        arrayAlloc->getAllocatedType()->print(llvm::errs());
        arrayAlloc->print(llvm::errs());

        ptr->print(llvm::errs());
    }

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

    llvm::Type *arrayType = arrayAlloc->getAllocatedType();

    // Create the GEP instruction to get the pointer to the element
    llvm::Value *elementPtr = builder.CreateInBoundsGEP(
        arrayType, 
        arrayAlloc, 
        indices, 
        "elem.ptr"
    );

    llvm::Type *elementType = arrayType;
    if (!elementType){
        printf("failed to get data from pointer\n");
    }
    ret = builder.CreateLoad(elementType, elementPtr, "elem.val");

    // Update floatInst flag so parent knows what type 'ret' is
    floatInst = elementType->isDoubleTy();
    printf("\n----------------------------------------\n");
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
    llvm::AllocaInst *allocation = symbolTable[node->getName()];
    if (!allocation) {
        std::cerr << "Error: Undefined variable " << node->getName() << std::endl;
        ret = builder.getInt32(0);
        return;
    }

    llvm::Type *allocType = allocation->getAllocatedType();

    // CRITICAL FIX:
    // If it is an array (e.g., [5 x i32]), do NOT load it. Return the pointer.
    if (allocType->isArrayTy()) {
        ret = allocation; 
        floatInst = false; 
    } 
    // If it is a regular variable or a pointer (like function params), load it.
    else {
        ret = builder.CreateLoad(allocType, allocation, node->getName());
        floatInst = ret->getType()->isDoubleTy();
    }
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

void LLVMVisitor::visit(CharNode *node){

    ret = llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), node->getValue());
}

void LLVMVisitor::visit(StringNode *node){

    
    const char *str = node->getValue();

    llvm::Constant *stringConst = llvm::ConstantDataArray::getString(context, str, true); 

    static int globalStringCounter = 0;
    std::string globalVarName = "__str_lit_" + std::to_string(globalStringCounter++);

    llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
        *(mod.get()),
        stringConst->getType(),
        true,
        llvm::GlobalValue::PrivateLinkage,
        stringConst,
        globalVarName
    );

    std::vector<llvm::Constant *> indices;
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(context);
    indices.push_back(llvm::ConstantInt::get(i32Ty,0));
    indices.push_back(llvm::ConstantInt::get(i32Ty,0));

    ret = llvm::ConstantExpr::getGetElementPtr(
        globalVar->getValueType(),
        globalVar,
        indices,
        "stringptr"
    );
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

void LLVMVisitor::visit(OpenNode *node){
    node->getPathExp()->accept(*this);
    llvm::Value* path_val = ret;


    node->getFlagsExp()->accept(*this);
    llvm::Value* flags_val = ret;

    node->getModeExp()->accept(*this);
    llvm::Value* mode_val = ret;

    //std::vector<llvm::Value*> args = {path_val,flags_val,mode_val};
    std::vector<llvm::Value*> args;
    args.push_back(path_val);
    args.push_back(flags_val);
    args.push_back(mode_val);

    ret = builder.CreateCall(mod->getFunction("open"),args,"open_result");

    
}

void LLVMVisitor::placeComent(){
    llvm::Function* dbgLabelFunc = mod->getFunction("llvm.dbg.label");
    if (!dbgLabelFunc) {
        llvm::Type* voidTy = builder.getVoidTy();
        llvm::Type* mdTy = llvm::Type::getMetadataTy(context);
        llvm::FunctionType* funcTy = llvm::FunctionType::get(voidTy, {mdTy}, false);
        
        dbgLabelFunc = llvm::Function::Create(
            funcTy, 
            llvm::Function::ExternalLinkage, 
            "llvm.dbg.label", 
            *mod 
        );
    }
    
    // 2. Create the string metadata node for the label.
    llvm::MDNode* labelNode = llvm::MDNode::get(context, 
        llvm::MDString::get(context, "--- Starting READ SYSCALL preparation ---")
    );

    // 3. Create a call to the intrinsic.
    llvm::Value* metadataValue = llvm::MetadataAsValue::get(context, labelNode);
    builder.CreateCall(dbgLabelFunc, metadataValue);

}

void LLVMVisitor::visit(ReadNode *node){
    node->getFdExp()->accept(*this);
    llvm::Value* fd_val = ret;

    //node->getBufExp()->accept(*this);

    
    llvm::Value* buffer_ptr = nullptr;
    //llvm::Value* raw_buffer_val = ret;


    llvm::AllocaInst *allocation =  symbolTable[dynamic_cast<IdentifierNode*>(node->getBufExp())->getName()];

    std::vector<llvm::Value*> indices1;
    indices1.push_back(builder.getInt64(0)); 
    indices1.push_back(builder.getInt64(0)); 

    buffer_ptr = builder.CreateInBoundsGEP(
        allocation->getAllocatedType(),
        allocation,
        indices1,
        "buffer_ptr"
    );

    // if (llvm::AllocaInst *arrayAlloc = llvm::dyn_cast<llvm::AllocaInst>(raw_buffer_val)) {
    //     std::vector<llvm::Value*> indices;
    //     indices.push_back(builder.getInt32(0)); 
    //     indices.push_back(builder.getInt32(0)); 
        
    //     // %buffer_ptr = getelementptr inbounds [4 x i8], ptr %read_buffer, i32 0, i32 0
    //     buffer_ptr = builder.CreateInBoundsGEP(
    //         arrayAlloc->getAllocatedType(), 
    //         arrayAlloc, 
    //         indices, 
    //         "read_buffer_ptr"
    //     );
    // } else {
    //     // Fallback: Assume the expression already returned a valid pointer.
    //     buffer_ptr = raw_buffer_val;
    // }

    // llvm::Function *parentFunction = builder.GetInsertBlock()->getParent();
    // llvm::AllocaInst *iPtr = createEntryBlockAlloca(parentFunction, "i", builder.getInt32Ty());

    // llvm::AllocaInst *arrayAllocation = createEntryBlockAlloca(
    //     parentFunction, 
    //     "read_data", 
    //     builder.getInt8Ty()->getPointerTo()
    // );
    // symbolTable["read_data"] = arrayAllocation;

    // llvm::Value *iBodyVal = builder.CreateLoad(builder.getInt32Ty(), iPtr);
    // llvm::Value *indices[] = { builder.getInt32(0), iBodyVal };
    // llvm::Value *elemPtr = builder.CreateInBoundsGEP(buffer_ptr->getType(), arrayAllocation, indices, "read_buffer");

    //================================================================


    // if (llvm::AllocaInst *arrayAlloc = llvm::dyn_cast<llvm::AllocaInst>(raw_buffer_val)) {
    //     std::vector<llvm::Value*> indices;
    //     indices.push_back(builder.getInt32(0)); 
    //     indices.push_back(builder.getInt32(0)); 
        
    //     // %buffer_ptr = getelementptr inbounds [4 x i8], ptr %read_buffer, i32 0, i32 0
    //     buffer_ptr = builder.CreateInBoundsGEP(
    //         arrayAlloc->getAllocatedType(), 
    //         arrayAlloc, 
    //         indices, 
    //         "read_buffer_ptr"
    //     );
    // } else {
    //     // Fallback: Assume the expression already returned a valid pointer.
    //     buffer_ptr = raw_buffer_val;
    // }

    // llvm::Function *parentFunction = builder.GetInsertBlock()->getParent();
    // llvm::AllocaInst *iPtr = createEntryBlockAlloca(parentFunction, "i", builder.getInt32Ty());

    // llvm::AllocaInst *arrayAllocation = createEntryBlockAlloca(
    //     parentFunction, 
    //     "read_data", 
    //     builder.getInt8Ty()->getPointerTo()
    // );
    // symbolTable["read_data"] = arrayAllocation;

    // llvm::Value *iBodyVal = builder.CreateLoad(builder.getInt32Ty(), iPtr);
    // llvm::Value *indices[] = { builder.getInt32(0), iBodyVal };
    // llvm::Value *elemPtr = builder.CreateInBoundsGEP(buffer_ptr->getType(), arrayAllocation, indices, "read_buffer");



    node->getCountExp()->accept(*this);
    llvm::Value* count_val = ret;


    std::vector<llvm::Value*> args = {fd_val,buffer_ptr,count_val};

    llvm::Value* read_bytes_value = builder.CreateCall(mod->getFunction("read"),args,"read_bytes");


    llvm::Value* read_bytes_zext = builder.CreateZExt(
        read_bytes_value,
        builder.getInt64Ty(),
        "read_bytes_zext"
    );

    llvm::Value* null_ptr = builder.CreateInBoundsGEP(
        builder.getInt8Ty(),
        buffer_ptr,
        read_bytes_zext,
        "null_terminator_ptr"
    );

    // std::string typestring;
    // llvm::raw_string_ostream abortas(typestring);
    // buffer_ptr->getType()->print(abortas);
    // std::cout << "##############" << typestring << "" << std::endl;

    builder.CreateStore(
        builder.getInt8(0),
        null_ptr
    );


    // builder.CreateLoad(
    //     buffer_ptr->getType(),
    //     elemPtr
    // );

    // builder.CreateStore(
    //     buffer_ptr,
    //     elemPtr
    // );
    ret = read_bytes_value;

}

void LLVMVisitor::visit(WriteNode *node){
    node->getFdExp()->accept(*this);
    llvm::Value* fd_val = ret;

    node->getBufExp()->accept(*this);
    llvm::Value* buffer_val = ret;

    node->getCountExp()->accept(*this);
    llvm::Value* count_val = ret;

    std::vector<llvm::Value*> args = {fd_val,buffer_val,count_val};

    ret = builder.CreateCall(mod->getFunction("write"),args,"write_bytes");
}

void LLVMVisitor::visit(CloseNode *node){
    node->getFdExp()->accept(*this);
    llvm::Value* fd_val = ret;


    std::vector<llvm::Value*> args = {fd_val};

    ret = builder.CreateCall(mod->getFunction("close"),args,"close_result");
}

void LLVMVisitor::visit(PrintNode *node){

    // printf("print node\n");

    node->getExp()->accept(*this);

    // Call printf with our returned value.
    std::vector<llvm::Value *> printArgs;

    // Change the format string depending on if we're dealing with a float or not.
    llvm::Value *formatStr;
    llvm::Value *printVal = ret;

    // std::string typestring;
    // llvm::raw_string_ostream abortas(typestring);
    // printVal->getType()->print(abortas);

    // std::string anotherone;
    // llvm::raw_string_ostream damn(anotherone);
    // llvm::ArrayType::get(builder.getInt8Ty(),4)->print(damn);

    // std::cout << node->getLine() << " " << typestring << " | " << anotherone <<std::endl;


    if (llvm::ArrayType *arrayType = llvm::dyn_cast<llvm::ArrayType>(printVal->getType())){
        llvm::Type *elementType = arrayType->getElementType();
        unsigned numElements = arrayType->getNumElements();

        llvm::Value *arrayPtr = builder.CreateAlloca(arrayType, nullptr, "array.ptr");
        builder.CreateStore(printVal,arrayPtr);

        llvm::Value *elementFormatStr;
        if (elementType->isIntegerTy(8)) {
            elementFormatStr = builder.CreateGlobalStringPtr("%c"); // Character, space separator
        } else if (elementType->isIntegerTy()) {
            elementFormatStr = builder.CreateGlobalStringPtr("%d"); // Integer, space separator
        } else if (elementType->isFloatTy() || elementType->isDoubleTy()) {
            elementFormatStr = builder.CreateGlobalStringPtr("%f"); // Float, space separator
        } else {
            // Handle other types if necessary, for now, skip array printing
            // Or default to a placeholder format
            elementFormatStr = builder.CreateGlobalStringPtr("%s");
        }

        llvm::BasicBlock *PreHeaderBlock = builder.GetInsertBlock();

        
        llvm::Function *TheFunction = builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *LoopHeader = llvm::BasicBlock::Create(context, "array.loop.header", TheFunction);
        llvm::BasicBlock *LoopBody = llvm::BasicBlock::Create(context, "array.loop.body", TheFunction);
        llvm::BasicBlock *LoopExit = llvm::BasicBlock::Create(context, "array.loop.exit", TheFunction);

        builder.CreateBr(LoopHeader);

        builder.SetInsertPoint(LoopHeader);

        llvm::PHINode *index = builder.CreatePHI(builder.getInt32Ty(), 2, "index.i");
        llvm::Value *initialIndex = builder.getInt32(0);
        index->addIncoming(initialIndex, PreHeaderBlock);

        llvm::Value *condition = builder.CreateICmpULT(index, builder.getInt32(numElements), "loop.cond");
        builder.CreateCondBr(condition, LoopBody, LoopExit);

        // --- Loop Body: Element Access and Print ---
        builder.SetInsertPoint(LoopBody);
        llvm::Value *elementPtr = builder.CreateInBoundsGEP(arrayType, arrayPtr, {builder.getInt32(0), index}, "element.ptr");
        llvm::Value *elementValue = builder.CreateLoad(elementType, elementPtr, "element.val");

        // Handle required type conversions for printf (e.g., extend i8 to i32 for printf or float to double)
        if (elementType->isIntegerTy(8)) {
            // printf handles i8 as i32, so zero-extend (or sign-extend depending on language)
            elementValue = builder.CreateZExt(elementValue, builder.getInt32Ty(), "char.to.int");
        } else if (elementType->isIntegerTy()) {
            // If it's a smaller integer, extend it to i32 for printf
            if (elementType->getIntegerBitWidth() < 32) {
                elementValue = builder.CreateSExt(elementValue, builder.getInt32Ty(), "smallint.to.int32");
            }
        } else if (elementType->isFloatTy()) {
            // Promote float to double for varargs functions like printf
            elementValue = builder.CreateFPExt(elementValue, builder.getDoubleTy(), "float.to.double");
        }

        // Call printf for the current element
        printArgs.clear();
        printArgs.push_back(elementFormatStr);
        printArgs.push_back(elementValue);
        builder.CreateCall(mod->getFunction("printf"), printArgs, "array.element.print");

        // Increment index: i + 1
        llvm::Value *nextIndex = builder.CreateAdd(index, builder.getInt32(1), "next.index");
        
        // Update PHI node and branch back to header
        llvm::BasicBlock *LoopBodyBlock = builder.GetInsertBlock();
        index->addIncoming(nextIndex, LoopBodyBlock);
        builder.CreateBr(LoopHeader);

        // --- Loop Exit: Final Print (Newline) and Cleanup ---
        builder.SetInsertPoint(LoopExit);
        
        // Print a final newline character after the array
        llvm::Value *newlineFormatStr = builder.CreateGlobalStringPtr("\n");
        printArgs.clear();
        printArgs.push_back(newlineFormatStr);
        builder.CreateCall(mod->getFunction("printf"), printArgs, "array.newline.print");

        // Clear ret to stop the regular print logic from running
        ret = nullptr;


    }


    if(ret){

        if (floatInst) {
            //if float make sure its a double and change to %f
            if (!printVal->getType()->isDoubleTy())
                printVal = builder.CreateSIToFP(printVal, builder.getDoubleTy());
                
            formatStr = builder.CreateGlobalStringPtr("%f\n");

            //if its a pointer and i8 length change to %s for strings
        } else if (printVal->getType()->isPointerTy()){
            llvm::PointerType *ptrType = llvm::dyn_cast<llvm::PointerType>(printVal->getType());

            if (printVal->getType() == builder.getInt8Ty()->getPointerTo()){
                formatStr = builder.CreateGlobalStringPtr("%s\n");
            }

            //if its length of i8 change to %c for chars
        } else if (printVal->getType()->isIntegerTy(8)){
            formatStr = builder.CreateGlobalStringPtr("%c\n");

            //default for integers, make sure its int and change to %d
        } else {

            if (printVal->getType()->isDoubleTy()){
                printVal = builder.CreateFPToSI(printVal,builder.getInt32Ty());
            }

            formatStr = builder.CreateGlobalStringPtr("%d\n");
        }
        
        

        printArgs.push_back(formatStr);
        printArgs.push_back(printVal);

        builder.CreateCall(mod->getFunction("printf"), printArgs);

        // Reset the float instruction marker.
        

    }

    floatInst = false;
    ret = nullptr;
}