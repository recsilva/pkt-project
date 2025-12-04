#include "whilenode.h"
#include "visitor.h"

WhileNode::WhileNode(int line, 
                     ExpNode *condition, 
                     std::vector<StatementNode*> *loopBody)
    : StatementNode(line, nullptr),
      condition(condition), 
      loopBody(loopBody) {}

ExpNode* WhileNode::getCondition() const {
    return condition;
}

const std::vector<StatementNode*>* WhileNode::getLoopBody() const {
    return loopBody;
}

void WhileNode::accept(class Visitor &v) {
    v.visit(this);
}