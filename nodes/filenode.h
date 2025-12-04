#ifndef FILE_NODE_H
#define FILE_NODE_H

#include "base\expnode.h"


class OpenNode : public ExpNode{
public:
    OpenNode(int line, ExpNode *path, ExpNode *flags, ExpNode *mode);
    ~OpenNode();

    ExpNode *getPathExp();
    ExpNode *getFlagsExp();
    ExpNode *getModeExp();

    void accept(class Visitor &v);
private:
    ExpNode *pathExp;
    ExpNode *flagsExp;
    ExpNode *modeExp;
};

class ReadNode : public ExpNode{
public:
    ReadNode(int line, ExpNode *fd, ExpNode *buf, ExpNode *count);
    ~ReadNode();

    ExpNode *getFdExp();
    ExpNode *getBufExp();
    ExpNode *getCountExp();

    void accept(class Visitor &v);
private:
    ExpNode *fdExp;
    ExpNode *bufExp;
    ExpNode *countExp;
};

class WriteNode : public ExpNode{
public:
    WriteNode(int line, ExpNode *fd, ExpNode *buf, ExpNode *count);
    ~WriteNode();

    ExpNode *getFdExp();
    ExpNode *getBufExp();
    ExpNode *getCountExp();

    void accept(class Visitor &v);
private:
    ExpNode *fdExp;
    ExpNode *bufExp;
    ExpNode *countExp;
};

class CloseNode : public ExpNode{
public:
    CloseNode(int line, ExpNode *fd);
    ~CloseNode();

    ExpNode *getFdExp();

    void accept(class Visitor &v);
private:
    ExpNode *fdExp;
};

#endif