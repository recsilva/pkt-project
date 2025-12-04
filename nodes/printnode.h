#ifndef PRINT_NODE_H
#define PRINT_NODE_H

#include "base/statementnode.h"
#include "base/expnode.h"

class PrintNode : public StatementNode{
public:
    PrintNode(int line, ExpNode *exp);
    ExpNode *getExp();
    void accept(class Visitor &v);
};

#endif