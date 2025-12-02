#include "ifnode.h"
#include "visitor.h"
#include "base/expnode.h"
#include "base/statementnode.h"

IfNode::IfNode(int line, 
               ExpNode *condition, 
               std::vector<StatementNode*> *trueBlock)
    : StatementNode(line, nullptr),
      condition(condition), 
      trueBlock(trueBlock) {}

ExpNode* IfNode::getCondition() const {
    return condition;
}

const std::vector<StatementNode*>* IfNode::getTrueBlock() const {
    return trueBlock;
}

void IfNode::accept(class Visitor &v) {
    v.visit(this);
}