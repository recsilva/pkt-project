#ifndef ARRAYASSIGNNODE_H
#define ARRAYASSIGNNODE_H

#include "../base/statementnode.h"
#include "../base/expnode.h"
#include <string>

class ArrayAssignNode : public StatementNode {
public:
    ArrayAssignNode(int line, 
                    const std::string& name, 
                    ExpNode *indexExp, 
                    ExpNode *valueExp);
    
    const std::string& getName() const;
    ExpNode* getIndexExpression() const;
    ExpNode* getValueExpression() const;

    void accept(class Visitor &v);

private:
    std::string name;
    ExpNode *indexExp;
    ExpNode *valueExp;
};

#endif