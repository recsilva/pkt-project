#ifndef CHAR_NODE_H
#define CHAR_NODE_H

#include "../base/expnode.h"

class CharNode: public ExpNode{
public:
    CharNode(int line, char value);
    void accept(class Visitor &v);
    char getValue();

private:
    char value;
};

#endif