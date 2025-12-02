#ifndef IFNODE_H
#define IFNODE_H

#include "base/statementnode.h"
#include "base/expnode.h"
#include <vector>

class IfNode : public StatementNode {
public:
    IfNode(int line, 
           ExpNode *condition, 
           std::vector<StatementNode*> *trueBlock);
    
    ExpNode* getCondition() const;
    const std::vector<StatementNode*>* getTrueBlock() const;

    void accept(class Visitor &v);

private:
    ExpNode *condition;
    std::vector<StatementNode*> *trueBlock; // Block of statements if condition is true
};

#endif