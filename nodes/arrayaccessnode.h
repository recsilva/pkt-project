#ifndef ARRAYACCESSNODE_H
#define ARRAYACCESSNODE_H

#include "expnode.h"
#include <string>

class ArrayAccessNode : public ExpNode {
public:
    // Constructor: Takes line number, array name, and the expression for the index
    ArrayAccessNode(int line, const std::string& name, ExpNode *indexExp);
    
    // Accessors
    const std::string& getName() const;
    ExpNode* getIndexExpression() const;

    void accept(class Visitor &v);

private:
    std::string name;
    ExpNode *indexExp;
};

#endif