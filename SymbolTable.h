#ifndef PROJECT2__3__SYMBOLTABLE_H
#define PROJECT2__3__SYMBOLTABLE_H

#include <string>
#include <utility>
#include <vector>

using namespace std;

enum VariableType {
    constInt,
    constChar,
    intType,
    charType,
    intArray,
    charArray,
    functionType,
    voidType
};

struct ArrayInfo {
    int beginAddr;
    int len;
};

/****************************************/

class FuncInfo {
public:
    FuncInfo(VariableType _returnType, vector<VariableType> _inputTypes) : returnType{_returnType},
                                                                           inputTypes{std::move(_inputTypes)} {
        // empty
    }

    void addInputTypes(const VariableType &type) {
        inputTypes.push_back(type);
    }

    VariableType getReturnType() const {
        return returnType;
    }

    vector<VariableType> getInputTypes() {
        return inputTypes;
    }

private:
    VariableType returnType = voidType;
    vector<VariableType> inputTypes;
    // Maybe should add the address of function entrance here
};

struct Item {
    string name;
    VariableType type;
    void *info = nullptr;

    Item(string _name, VariableType _type, void *_info) : name(std::move(_name)), type(_type), info(_info) {}

    ~Item() {
        if (info == nullptr)
            return;
        if (type == constInt || type == constChar || type == intType || type == charType) {
            delete (static_cast<int *>(info));
        } else if (type == intArray || type == charArray) {
            delete (static_cast<ArrayInfo *>(info));
        } else if (type == functionType) {
            delete (static_cast<FuncInfo *>(info));
        }
    }
};

class SymbolTable {
public:
    void pushStack();

    void pushStack(vector<Item *> *stack);

    void popStack();

    const Item *searchName(const string &name);   // find: return true; not find: return false
    void addName(const string &name, VariableType type, void *info);

    void checkFuncVariable(const string &name, const FuncInfo &funcInfo);

    ~SymbolTable();

private:
    vector<vector<Item *> *> table;
    int addr = 0;   // unit: byte. Assume that int: 4 bytes; char: 1 byte.
};


#endif //PROJECT2__3__SYMBOLTABLE_H
