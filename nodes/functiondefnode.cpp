// --- nodes/functiondefnode.cpp ---

#include "functiondefnode.h"
#include "visitor.h" 

FunctionDefNode::FunctionDefNode(int line, 
                                 TypeNode *type,
                                 char *id, 
                                 std::vector<ParamDefNode*> *params, 
                                 std::vector<StatementNode*> *body)
    : StatementNode(line, nullptr),
      returnType(type),
      name(id), 
      params(params), 
      body(body)
{
    // Free the dynamically allocated ID name passed from the parser
    free(id);
}

FunctionDefNode::~FunctionDefNode() {
    // 1. Clean up parameter definitions
    if (params) {
        for (ParamDefNode* param : *params) {
            // ParamDefNode destructor handles its own name and default expression cleanup
            delete param; 
        }
        delete params;
    }
    
    // 2. Clean up function body statements
    if (body) {
        for (StatementNode* stmt : *body) {
            delete stmt;
        }
        delete body;
    }
}

void FunctionDefNode::accept(class Visitor &v) {
    v.visit(this);
}