#include <iostream>

#include "uminusnode.h"
#include "../visitor.h"

UMinusNode::UMinusNode(int line, ExpNode *exp) :
    ExpNode(line), exp(exp) {}

ExpNode* UMinusNode::getExp(){
    return exp;
}

void UMinusNode::accept(Visitor &v){
    v.visit(this);
}
