#ifndef UMINUS_NODE_H
#define UMINUS_NODE_H

#include "../base/expnode.h"

class UMinusNode: public ExpNode {
public:
    UMinusNode(int line, ExpNode *exp);
    ExpNode* getExp();
    void accept(class Visitor &v);

private:
    ExpNode *exp;
};

#endif