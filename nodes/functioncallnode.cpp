// --- nodes/functioncallnode.cpp ---

#include "functioncallnode.h"
#include "visitor.h" 
#include <iostream>

FunctionCallNode::FunctionCallNode(int line, 
                                   char *id, 
                                   std::vector<ExpNode*> *args)
    : ExpNode(line),
      name(id), 
      args(args) {}

FunctionCallNode::~FunctionCallNode() {
    if (args) {
        for (ExpNode* arg : *args) {
            delete arg;
        }
        delete args;
    }
}

void FunctionCallNode::accept(class Visitor &v) {
    v.visit(this);
}