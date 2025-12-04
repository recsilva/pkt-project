// --- nodes/functioncallnode.h ---

#ifndef FUNCTIONCALLNODE_H
#define FUNCTIONCALLNODE_H

#include "base/expnode.h"
#include <vector>
#include <string>

class FunctionCallNode : public ExpNode {
public:
    FunctionCallNode(int line, 
                     char *id, 
                     std::vector<ExpNode*> *args);
    ~FunctionCallNode();

    const std::string& getName() const { return name; }
    const std::vector<ExpNode*>* getArgs() const { return args; }

    void accept(class Visitor &v) override;

private:
    std::string name;
    std::vector<ExpNode*> *args;
};

#endif