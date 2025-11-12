#include "nodes/programnode.h"
#include "nodes/visitor.h"

using namespace std;

extern ProgramNode* program;
extern int yyparse();

int main(int argc, char **argv) {
    yyparse();
    Visitor v;
    program->accept(v);
    return 0;
}