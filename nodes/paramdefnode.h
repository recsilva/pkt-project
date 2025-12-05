
#ifndef PARAMDEFNODE_H
#define PARAMDEFNODE_H

#include "base/node.h"
#include "typenode.h"
#include <string>

class ParamDefNode : public Node {
public:
    ParamDefNode(int line, char *name, TypeNode *type);
    ~ParamDefNode();

    const std::string& getName() const { return name; }
    TypeNode* getType() const { return type; }

    // Note: This node won't have a visitor method itself, 
    // it's just data consumed by FunctionDefNode::visit
    void accept(class Visitor &v) override { /* No-op, consumed by FunctionDefNode */ }

private:
    std::string name;
    TypeNode *type; // Nullptr if no default value provided
};

#endif