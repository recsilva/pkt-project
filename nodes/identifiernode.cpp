#include "identifiernode.h"
#include "visitor.h"

IdentifierNode::IdentifierNode(int line, const std::string& name)
    : ExpNode(line), name(name) {}

const std::string& IdentifierNode::getName() const {
    return name;
}

void IdentifierNode::accept(class Visitor &v) {
    v.visit(this);
}