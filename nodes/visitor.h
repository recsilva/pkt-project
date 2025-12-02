#ifndef VISITOR_H
#define VISITOR_H

#include <iostream>

#include "base/programnode.h"
#include "base/statementnode.h"

#include "arithmetic/plusnode.h"
#include "arithmetic/uminusnode.h"
#include "arithmetic/minusnode.h"
#include "arithmetic/multnode.h"
#include "arithmetic/divnode.h"

#include "types/integernode.h"
#include "types/floatnode.h"
#include "types/charnode.h"

#include "memory/identifiernode.h"
#include "memory/assignmentnode.h"

#include "memory/arraydefnode.h"
#include "memory/arrayaccessnode.h"
#include "memory/arrayassignnode.h"

class Visitor {
public:
    virtual void visit(ProgramNode *node);
    virtual void visit(StatementNode *node);
    virtual void visit(IntegerNode *node);
    virtual void visit(FloatNode *node);
    virtual void visit(CharNode *node);
    virtual void visit(PlusNode *node);
    virtual void visit(UMinusNode *node);
    virtual void visit(MinusNode *node);
    virtual void visit(MultNode *node);
    virtual void visit(DivNode *node);
    virtual void visit(IdentifierNode *node); 
    virtual void visit(AssignmentNode *node); 
    virtual void visit(ArrayDefNode *node);
    virtual void visit(ArrayAccessNode *node);
    virtual void visit(ArrayAssignNode *node);
};

#endif