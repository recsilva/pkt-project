#ifndef IDENTIFIER_NODE_H
#define IDENTIFIER_NODE_H

#include <string>
#include "expnode.h"

class IdentifierNode: public ExpNode {
public:
    IdentifierNode(int line, const std::string& name);
    const std::string& getName() const;
    void accept(class Visitor &v);

private:
    std::string name;
};

#endif