#ifndef ASSIGNMENT_NODE_H
#define ASSIGNMENT_NODE_H
#include <string>
#include "../base/statementnode.h"
#include "../base/expnode.h"

class AssignmentNode: public StatementNode {
public:
    AssignmentNode(int line, const std::string& name, ExpNode *exp);
    const std::string& getName() const;
    ExpNode* getExp() const;
    void accept(class Visitor &v);

private:
    std::string name;
    ExpNode *exp;
};

#endif