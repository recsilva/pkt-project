#include "returnnode.h"
#include "visitor.h"

ReturnNode::ReturnNode(int line, ExpNode *exp) 
: StatementNode(line, exp) {}

void ReturnNode::accept(class Visitor &v){
    v.visit(this);
}