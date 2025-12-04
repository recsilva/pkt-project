
#ifndef PARAMDEFNODE_H
#define PARAMDEFNODE_H

#include "base/node.h"
#include "base/expnode.h"
#include <string>

class ParamDefNode : public Node {
public:
    ParamDefNode(int line, char *name, ExpNode *defaultValue);
    ~ParamDefNode();

    const std::string& getName() const { return name; }
    ExpNode* getDefaultValue() const { return defaultValue; }

    // Note: This node won't have a visitor method itself, 
    // it's just data consumed by FunctionDefNode::visit
    void accept(class Visitor &v) override { /* No-op, consumed by FunctionDefNode */ }

private:
    std::string name;
    ExpNode *defaultValue; // Nullptr if no default value provided
};

#endif