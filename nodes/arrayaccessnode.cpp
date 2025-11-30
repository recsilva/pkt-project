#include "arrayaccessnode.h"
#include "visitor.h"
#include "expnode.h"

ArrayAccessNode::ArrayAccessNode(int line, 
                                 const std::string& name, 
                                 ExpNode *indexExp)
    : ExpNode(line), name(name), indexExp(indexExp) {}

const std::string& ArrayAccessNode::getName() const {
    return name;
}

ExpNode* ArrayAccessNode::getIndexExpression() const {
    return indexExp;
}

void ArrayAccessNode::accept(class Visitor &v) {
    v.visit(this);
}