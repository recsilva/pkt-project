#ifndef ARRAYDEFNODE_H
#define ARRAYDEFNODE_H

#include "../base/statementnode.h"
#include "../base/expnode.h"
#include <string>

class ArrayDefNode : public StatementNode {
public:
    ArrayDefNode(int line, 
                 const std::string& name, 
                 ExpNode *sizeExp, 
                 ExpNode *initValue);
    
    // Accessors
    const std::string& getName() const;
    ExpNode* getSizeExpression() const;
    ExpNode* getInitialValue() const;

    void accept(class Visitor &v);

private:
    std::string name;
    ExpNode *sizeExp;
    ExpNode *initValue;
};

#endif