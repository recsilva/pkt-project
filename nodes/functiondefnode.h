#ifndef FUNCTIONDEFNODE_H
#define FUNCTIONDEFNODE_H

#include "base/statementnode.h"
#include "paramdefnode.h"
#include "typenode.h"
#include <vector>
#include <string>

class FunctionDefNode : public StatementNode {
public:
    FunctionDefNode(int line, 
                    TypeNode *type,
                    char *id, 
                    std::vector<ParamDefNode*> *params,
                    std::vector<StatementNode*> *body);
    ~FunctionDefNode();

    const std::string& getName() const { return name; }
    const std::vector<ParamDefNode*>* getParams() const { return params; }
    const std::vector<StatementNode*>* getBody() const { return body; }
    TypeNode* getReturnType() const {return returnType;}

    void accept(class Visitor &v) override;

private:
    std::string name;
    std::vector<ParamDefNode*> *params;
    std::vector<StatementNode*> *body;
    TypeNode* returnType;
};

#endif