#ifndef TYPENODE_H
#define TYPENODE_H

#include "base/node.h" // Assuming all nodes inherit from Node
#include <string>

class TypeNode : public Node {
public:
    // Enumeration to represent all possible types in your language
    enum TypeId {
        INT,
        FLT,
        CHR,
        STRING,
        INT_ARRAY,
        FLT_ARRAY,
        VOID, // Useful for function return types
        UNKNOWN
    };

    // Constructor to set the type
    TypeNode(int line, TypeId id);
    
    // Destructor (simple, no complex memory management needed here)
    ~TypeNode() = default;

    // Accessor for the Type ID
    TypeId getTypeId() const { return typeId; }

    // Helper function to check if the type is an array
    bool isArray() const {
        return typeId == INT_ARRAY || typeId == FLT_ARRAY;
    }
    
    // Visitor method (optional, as it's mostly data, but good practice)
    void accept(class Visitor &v) override; 

private:
    TypeId typeId;
};

#endif // TYPENODE_H