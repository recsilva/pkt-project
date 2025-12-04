#ifndef WHILENODE_H
#define WHILENODE_H

#include "base/statementnode.h"
#include "base/expnode.h"
#include <vector>

class WhileNode : public StatementNode {
public:
    WhileNode(int line, 
              ExpNode *condition, 
              std::vector<StatementNode*> *loopBody);
    
    ExpNode* getCondition() const;
    const std::vector<StatementNode*>* getLoopBody() const;

    void accept(class Visitor &v);

private:
    ExpNode *condition;
    std::vector<StatementNode*> *loopBody; 
};

#endif