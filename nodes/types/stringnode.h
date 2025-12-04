#ifndef STRING_NODE_H
#define STRING_NODE_H

#include "../base/expnode.h"

class StringNode: public ExpNode {
public:
    StringNode(int line, char* value);
    char* getValue();
    void accept(class Visitor &v);
    ~StringNode();
private:
    char* value;
};

#endif