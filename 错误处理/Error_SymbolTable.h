#ifndef PROJECT2__3__ERROR_SYMBOLTABLE_H
#define PROJECT2__3__ERROR_SYMBOLTABLE_H

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


/* The errors related with symbol table */
class ErrorException : public exception {
public:
    char getErrorType() {
        return errorType;
    }

    int getErrorLine() {
        return errorLine;
    }

protected:
    char errorType;

    int errorLine;     // errorLine = -1 means the line error happens is the current line

    explicit ErrorException(int _errorLine = -1) : errorLine{_errorLine} {
        errorType = 'b';    // randomly assign
    }
};

class DuplicateNameDefinition : public ErrorException {
public:
    explicit DuplicateNameDefinition(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'b';
    }
};

class UndefinedName : public ErrorException {
public:
    explicit UndefinedName(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'c';
    }
};

class WrongFuncVariableNum : public ErrorException {
public:
    explicit WrongFuncVariableNum(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'd';
    }
};

class WrongFuncVariableType : public ErrorException {
public:
    explicit WrongFuncVariableType(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'e';
    }
};

class InvalidConditionType : public ErrorException {
public:
    explicit InvalidConditionType(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'f';
    }
};

class SurplusReturn : public ErrorException {
public:
    explicit SurplusReturn(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'g';
    }
};

class LackReturn : public ErrorException {
public:
    explicit LackReturn(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'h';
    }
};

class SubscriptNotInt : public ErrorException {
public:
    explicit SubscriptNotInt(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'i';
    }
};

class ChangeConstantValue : public ErrorException {
public:
    explicit ChangeConstantValue(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'j';
    }
};

class ShouldHaveSemicolon : public ErrorException {
public:
    explicit ShouldHaveSemicolon(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'k';
    }
};

class ShouldHaveRPARENT : public ErrorException {
public:
    explicit ShouldHaveRPARENT(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'l';
    }
};


class ShouldHaveRBRACK : public ErrorException {
public:
    explicit ShouldHaveRBRACK(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'm';
    }
};

class LackWhileInDoWhile : public ErrorException {
public:
    explicit LackWhileInDoWhile(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'n';
    }
};

class TypeErrorInConstantDefine : public ErrorException {
public:
    explicit TypeErrorInConstantDefine(int _errorLine = -1) : ErrorException(_errorLine) {
        errorType = 'o';
    }
};

/****************************************/

class FuncInfo {
public:
    FuncInfo(VariableType _returnType, vector<VariableType> _inputTypes) : returnType{_returnType},
                                                                           inputTypes{std::move(_inputTypes)} {
        // empty
    }

    void check(const FuncInfo &FuncInfo) {  // only check input types
        if (this->inputTypes.size() != FuncInfo.inputTypes.size()) {
            throw WrongFuncVariableNum{};
        }
        auto iter1 = this->inputTypes.cbegin();
        auto iter2 = FuncInfo.inputTypes.cbegin();
        while (iter1 != this->inputTypes.cend()) {
            if (*iter1 != *iter2) {
                throw WrongFuncVariableType{};
            }
            iter1++;
            iter2++;
        }
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

class Error_SymbolTable {
public:
    void pushStack();

    void pushStack(vector<Item *> *stack);

    void popStack();

    const Item *searchName(const string &name);   // find: return true; not find: return false
    void addName(const string &name, VariableType type, void *info);

    void checkFuncVariable(const string &name, const FuncInfo &funcInfo);

    ~Error_SymbolTable();

private:
    vector<vector<Item *> *> table;
    int addr = 0;   // unit: byte. Assume that int: 4 bytes; char: 1 byte.
};


#endif //PROJECT2__3__ERROR_SYMBOLTABLE_H
