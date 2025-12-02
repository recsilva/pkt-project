#include "comparisonnode.h"
#include "visitor.h"

// The constructor parameter name is now correctly 'opType'
ComparisonNode::ComparisonNode(int line, ExpNode *left, ExpNode *right, OpType opType) :
    ExpNode(line), left(left), right(right), op(opType) {}

ExpNode* ComparisonNode::getLeft() const {
    return left;
}

ExpNode* ComparisonNode::getRight() const {
    return right;
}

// The getter returns the scoped enum
ComparisonNode::OpType ComparisonNode::getOp() const {
    return op;
}

void ComparisonNode::accept(Visitor &v) {
    v.visit(this);
}