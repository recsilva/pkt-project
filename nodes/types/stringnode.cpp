#include <iostream>

#include "stringnode.h"
#include "../visitor.h"

StringNode::StringNode(int line, char *value)
    :ExpNode(line), value(value) {}

char* StringNode::getValue(){
    return value;
}

StringNode::~StringNode(){
    delete[] value;
}

void StringNode::accept(Visitor &v){
    v.visit(this);
}