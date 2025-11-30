#include "arraydefnode.h"
#include "visitor.h"

ArrayDefNode::ArrayDefNode(int line, 
                           const std::string& name, 
                           ExpNode *sizeExp, 
                           ExpNode *initValue)
    : StatementNode(line, initValue), // We inherit from StatementNode
      name(name), 
      sizeExp(sizeExp), 
      initValue(initValue) {}

const std::string& ArrayDefNode::getName() const {
    return name;
}

ExpNode* ArrayDefNode::getSizeExpression() const {
    return sizeExp;
}

ExpNode* ArrayDefNode::getInitialValue() const {
    return initValue;
}

void ArrayDefNode::accept(class Visitor &v) {
    v.visit(this);
}