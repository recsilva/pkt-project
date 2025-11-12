#include "nodes/programnode.h"
#include "nodes/cppvisitor.h"

using namespace std;

extern ProgramNode* program;
extern int yyparse();

int main(int argc, char **argv) {
    yyparse();
    CPPVisitor v("src.cpp");
    program->accept(v);
    return 0;
}