#include "paramdefnode.h"

ParamDefNode::ParamDefNode(int line, char *name, ExpNode *defaultValue)
    : Node(line), name(name), defaultValue(defaultValue) 
{
    // Free the dynamically allocated name from yacc/lex
    free(name);
}

ParamDefNode::~ParamDefNode() {
    delete defaultValue;
}