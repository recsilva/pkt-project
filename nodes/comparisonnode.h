#ifndef COMPARISONNODE_H
#define COMPARISONNODE_H

#include "base/expnode.h"

class ComparisonNode: public ExpNode {
public:

    enum OpType {
        LT, // Less Than
        GT, // Greater Than
        LE, // Less than or Equal
        GE, // Greater than or Equal
        EQ, // Equal
        NE  // Not Equal
    };

    ComparisonNode(int line, ExpNode *left, ExpNode *right, OpType OP_TYPE);
    ExpNode* getLeft() const;
    ExpNode* getRight() const;
    OpType getOp() const;
    void accept(class Visitor &v);

private:
    ExpNode *left, *right;
    OpType op;
};

#endif