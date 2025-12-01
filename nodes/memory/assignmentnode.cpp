#include "assignmentnode.h"
#include "../visitor.h"

AssignmentNode::AssignmentNode(int line, const std::string& name, ExpNode *exp)
    : StatementNode(line, exp), name(name), exp(exp) {}

const std::string& AssignmentNode::getName() const {
    return name;
}

ExpNode* AssignmentNode::getExp() const {
    return exp;
}

void AssignmentNode::accept(class Visitor &v) {
    v.visit(this);
}