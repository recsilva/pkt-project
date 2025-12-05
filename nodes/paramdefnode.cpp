#include "paramdefnode.h"

ParamDefNode::ParamDefNode(int line, char *name, TypeNode *type)
    : Node(line), name(name), type(type) 
{
    // Free the dynamically allocated name from yacc/lex
    free(name);
}

ParamDefNode::~ParamDefNode() {
    delete type;
}