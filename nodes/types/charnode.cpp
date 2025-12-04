#include <iostream>

#include "charnode.h"
#include "../visitor.h"

CharNode::CharNode(int line, char value) :
    ExpNode(line), value(value) {}

char CharNode::getValue(){
    return value;
}

void CharNode::accept(Visitor &v){
    v.visit(this);
}