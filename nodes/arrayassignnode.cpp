#include "arrayassignnode.h"
#include "visitor.h"
#include "expnode.h"

ArrayAssignNode::ArrayAssignNode(int line, 
                                 const std::string& name, 
                                 ExpNode *indexExp, 
                                 ExpNode *valueExp)
    : StatementNode(line, valueExp), // Inherit from StatementNode
      name(name), 
      indexExp(indexExp), 
      valueExp(valueExp) {}

const std::string& ArrayAssignNode::getName() const {
    return name;
}

ExpNode* ArrayAssignNode::getIndexExpression() const {
    return indexExp;
}

ExpNode* ArrayAssignNode::getValueExpression() const {
    return valueExp;
}

void ArrayAssignNode::accept(class Visitor &v) {
    v.visit(this);
}