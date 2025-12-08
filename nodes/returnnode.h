#ifndef RETURNNODE_H
#define RETURNNODE_H

#include "base/statementnode.h"

class ReturnNode : public StatementNode {
public:
    ReturnNode(int line, ExpNode *exp);
    void accept(class Visitor &v);
};

#endif