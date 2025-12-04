#include <iostream>

#include "printnode.h"
#include "visitor.h"

PrintNode::PrintNode(int line, ExpNode* exp)
    : StatementNode(line, exp) {}

ExpNode *PrintNode::getExp(){
    return StatementNode::getExp();
}

void PrintNode::accept(class Visitor &v){
    v.visit(this);
}

