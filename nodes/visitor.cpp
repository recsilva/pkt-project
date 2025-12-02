#include "visitor.h"

void Visitor::visit(ProgramNode *node) {
    std::cout << "Program" << std::endl;
    for (auto statement : node->getStatements())
        statement->accept(*this);
}

void Visitor::visit(StatementNode *node) {
    std::cout << "STATEMENT" << std::endl;
    node->getExp()->accept(*this);
    std::cout << std::endl;
}

// --- Arithmetic Nodes ---
void Visitor::visit(IntegerNode *node) {
    std::cout << node->getValue();
}

void Visitor::visit(FloatNode *node) {
    std::cout << node->getValue();
}

void Visitor::visit(PlusNode *node) {
    node->getLeft()->accept(*this);
    std::cout << " + ";
    node->getRight()->accept(*this);
}

void Visitor::visit(UMinusNode *node){
    node->getExp()->accept(*this);
    std::cout << " what do we have here ?";
}

void Visitor::visit(MinusNode *node) {
    node->getLeft()->accept(*this);
    std::cout << " - ";
    node->getRight()->accept(*this);
}

void Visitor::visit(MultNode *node) {
    node->getLeft()->accept(*this);
    std::cout << " * ";
    node->getRight()->accept(*this);
}

void Visitor::visit(DivNode *node) {
    node->getLeft()->accept(*this);
    std::cout << " / ";
    node->getRight()->accept(*this);
}

// --- New Variable Nodes ---

void Visitor::visit(IdentifierNode *node) {
    // Prints the variable name
    std::cout << node->getName();
}

void Visitor::visit(AssignmentNode *node) {
    // Prints the assignment structure: Identifier = Expression
    std::cout << "ASSIGNMENT: " << node->getName() << " = ";
    node->getExp()->accept(*this);
}

void Visitor::visit(ArrayDefNode *node) {
    std::cout << "ARRAY DEF: " << node->getName() 
              << "[" ;
    node->getSizeExpression()->accept(*this);
    std::cout << "] := ";
    node->getInitialValue()->accept(*this);
    std::cout << std::endl;
}

void Visitor::visit(ArrayAccessNode *node) {
    // Prints: array_name[index_expr]
    std::cout << node->getName() << "[";
    node->getIndexExpression()->accept(*this);
    std::cout << "]";
}

void Visitor::visit(ArrayAssignNode *node) {
    // Prints: array_name[index_expr] = value_expr
    std::cout << node->getName() << "[";
    node->getIndexExpression()->accept(*this);
    std::cout << "] = ";
    node->getValueExpression()->accept(*this);
    std::cout << std::endl;
}

// --- Corrected ComparisonNode Visitor ---
void Visitor::visit(ComparisonNode *node) {
    std::cout << "(";
    node->getLeft()->accept(*this);
    
    // Print the actual comparison symbol
    std::cout << " " << node->getOp() << " "; 
    
    node->getRight()->accept(*this);
    std::cout << ")";
}

// --- Corrected IfNode Visitor ---
void Visitor::visit(IfNode *node) {
    std::cout << "IF (";
    
    // 1. Print the condition expression
    node->getCondition()->accept(*this);
    
    std::cout << ") {" << std::endl;
    
    // 2. Iterate over and print all statements in the true block
    const std::vector<StatementNode*>* trueBlock = node->getTrueBlock();
    for (StatementNode *stmt : *trueBlock) {
        // Add indentation for readability
        std::cout << "    "; 
        stmt->accept(*this);
        std::cout << ";" << std::endl; 
    }
    
    std::cout << "}" << std::endl;
}