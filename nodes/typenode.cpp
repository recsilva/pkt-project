#include "typenode.h"
#include "visitor.h" // Include your main Visitor interface

// --- Constructor Implementation ---
TypeNode::TypeNode(int line, TypeId id) 
    : Node(line), typeId(id) {}

// --- Visitor Implementation ---
// Even if this is just a data node, it needs an accept implementation.
void TypeNode::accept(Visitor &v) {
    // Call the specific visitor method for TypeNode
    v.visit(this); 
}

// You might also need the implementation of the destructor if it wasn't defaulted.
// TypeNode::~TypeNode() {
//     // Nothing to free here since we only store an enum.
// }