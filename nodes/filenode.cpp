#include <iostream>

#include "filenode.h"
#include "visitor.h"

OpenNode::OpenNode(int line,ExpNode *path, ExpNode *flags, ExpNode *mode)
    : ExpNode(line), pathExp(path), flagsExp(flags), modeExp(mode) {}

OpenNode::~OpenNode(){
    delete pathExp;
    delete flagsExp;
    delete modeExp;
}

ExpNode *OpenNode::getPathExp(){
    return pathExp;
}
ExpNode *OpenNode::getFlagsExp(){
    return flagsExp;
}
ExpNode *OpenNode::getModeExp(){
    return modeExp;
}

void OpenNode::accept(Visitor &v){
    v.visit(this);
}


ReadNode::ReadNode(int line,ExpNode *fd, ExpNode *buf, ExpNode *count)
    : ExpNode(line), fdExp(fd), bufExp(buf), countExp(count) {}

ReadNode::~ReadNode(){
    delete fdExp;
    delete bufExp;
    delete countExp;
}

ExpNode *ReadNode::getFdExp(){
    return fdExp;
}
ExpNode *ReadNode::getBufExp(){
    return bufExp;
}
ExpNode *ReadNode::getCountExp(){
    return countExp;
}

void ReadNode::accept(Visitor &v){
    v.visit(this);
}


WriteNode::WriteNode(int line,ExpNode *fd, ExpNode *buf, ExpNode *count)
    : ExpNode(line), fdExp(fd), bufExp(buf), countExp(count) {}

WriteNode::~WriteNode(){
    delete fdExp;
    delete bufExp;
    delete countExp;
}

ExpNode *WriteNode::getFdExp(){
    return fdExp;
}
ExpNode *WriteNode::getBufExp(){
    return bufExp;
}
ExpNode *WriteNode::getCountExp(){
    return countExp;
}

void WriteNode::accept(Visitor &v){
    v.visit(this);
}


CloseNode::CloseNode(int line,ExpNode *fd)
    : ExpNode(line), fdExp(fd) {}

CloseNode::~CloseNode(){
    delete fdExp;
}

ExpNode *CloseNode::getFdExp(){
    return fdExp;
}
void CloseNode::accept(Visitor &v){
    v.visit(this);
}