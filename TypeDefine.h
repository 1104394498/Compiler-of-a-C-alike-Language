#pragma once

#ifndef INC_3___TYPEDEFINE_H
#define INC_3___TYPEDEFINE_H

#include <vector>
#include <string>
#include "SymbolTable.h"
/***** Lexical Analysis *****/
// STATUS in lexical analysis
enum LexicalAnalysisStatus {
    BEGIN,      // Initial Status
    STRING,     // String Status
    NUMCONST,   // Integer Const Status
    CHARCONST,  // Char Const Status
    STRINGCONST,    // String Const Status
    SYMBOL      // Operators and Other Symbols
};

enum ErrorTypes {
    QUOTATION_MARK_NOT_MATCH,
    INVALID_CHAR
};

class Error {
public:
    bool operator<(const Error &error) const {
        return lineNO < error.lineNO;
    }

    Error(int _lineNO, char _errorType) : lineNO(_lineNO), errorType(_errorType) {}

    int getLineNO() {
        return lineNO;
    }

    char getErrorType() {
        return errorType;
    }

private:
    int lineNO;
    char errorType;
};

/***** intermediate code ***********/
enum OperatorType {
    FuncDefStart,   // int foo()
    FuncPara,       // para int a
    CallFuncPush,   // push x
    CallFunc,       // call tar
    FuncRetInDef,   // ret x
    Add,            // t1 = b+c
    Minus,          // t1 = b-c
    Mul,            // t1 = b*c
    Div,            // t1 = b/c
    Neg,            // t1 = -a
    IsEqual,        // b == c
    IsLess,         // b < c
    IsLeq,          // b <= c
    IsGreater,      // b > c
    IsGeq,          // b >= c
    IsNeq,          // b != c
    Goto,           // GOTO label1
    BNZ,            // BNZ label1,
    BZ,             // BZ label1
    Label,          // label1:
    Assign,         // a = b
    SaveEnv,        // save the current environment before calling a function
    RestoreEnv,     // restore environment
    Exit,           // exit program
    AssignRetValue, // i = RET
    GetArrayValue,  // a = array[i]
    Printf,         // printf
    Scanf,          // scanf
    ArrayElemAssign,// array[i] = a
    ConstDef,       // const int variable, const char variable
    VarStatement,         // int variable, char variable
    VarArrayStatement,     // int array[10], char array[10]
    LoopSaveRegStatus,      // fix bugs in loop
    LoopRestoreRegStatus,    // fix bugs in loop
    DoWhileBNZ      // special BNZ for do-while
};

class IntermediateCmd {
private:
    OperatorType operatorType;
    std::vector<std::string> operands;
public:
    explicit IntermediateCmd(OperatorType _operatorType) : operatorType{_operatorType} {}

    void addOperands(const std::string &operand) {
        operands.push_back(operand);
    }

    OperatorType getOperatorType() const { return operatorType; }

    const vector<string> &getOperands() const { return operands; }

    std::string print() const {
        std::string cmd;
        if (operatorType == FuncDefStart) {
            cmd += operands.at(0);
            cmd += ' ';
            cmd += operands.at(1);
            cmd += "()";
        } else if (operatorType == FuncPara) {
            cmd += "para ";
            cmd += operands.at(0);
            cmd += ' ';
            cmd += operands.at(1);
        } else if (operatorType == CallFuncPush) {
            cmd += "push ";
            cmd += operands.at(0);
        } else if (operatorType == CallFunc) {
            cmd += "call ";
            cmd += operands.at(0);
        } else if (operatorType == FuncRetInDef) {
            cmd += "ret ";
            if (!operands.empty())
                cmd += operands.at(0);
        } else if (operatorType == Add) {
            cmd += operands.at(0);
            cmd += " = ";
            cmd += operands.at(1);
            cmd += " + ";
            cmd += operands.at(2);
        } else if (operatorType == Minus) {
            cmd += operands.at(0);
            cmd += " = ";
            cmd += operands.at(1);
            cmd += " - ";
            cmd += operands.at(2);
        } else if (operatorType == Mul) {
            cmd += operands.at(0);
            cmd += " = ";
            cmd += operands.at(1);
            cmd += " * ";
            cmd += operands.at(2);
        } else if (operatorType == Div) {
            cmd += operands.at(0);
            cmd += " = ";
            cmd += operands.at(1);
            cmd += " / ";
            cmd += operands.at(2);
        } else if (operatorType == Neg) {
            cmd += operands.at(0);
            cmd += " -";
            cmd += operands.at(1);
        } else if (operatorType == IsEqual) {
            cmd += operands.at(0);
            cmd += " == ";
            cmd += operands.at(1);
        } else if (operatorType == Goto) {
            cmd += "GOTO ";
            cmd += operands.at(0);
        } else if (operatorType == BNZ) {
            cmd += "BNZ ";
            cmd += operands.at(0);
        } else if (operatorType == BZ) {
            cmd += "BZ ";
            cmd += operands.at(0);
        } else if (operatorType == Label) {
            cmd += operands.at(0);
            cmd += ':';
        } else if (operatorType == Assign) {
            cmd += operands.at(0);
            cmd += " = ";
            cmd += operands.at(1);
        } else if (operatorType == SaveEnv) {
            cmd += "SaveEnvironment";
        } else if (operatorType == RestoreEnv) {
            cmd += "RestoreEnvironment";
        } else if (operatorType == Exit) {
            cmd += "Exit";
        } else if (operatorType == AssignRetValue) {
            cmd += operands.at(0);
            cmd += " = RET";
        } else if (operatorType == GetArrayValue) {
            cmd += operands.at(0);
            cmd += " = ";
            cmd += operands.at(1);
            cmd += "[";
            cmd += operands.at(2);
            cmd += "]";
        } else if (operatorType == IsLess) {
            cmd += operands.at(0);
            cmd += " < ";
            cmd += operands.at(1);
        } else if (operatorType == IsLeq) {
            cmd += operands.at(0);
            cmd += " <= ";
            cmd += operands.at(1);
        } else if (operatorType == IsGreater) {
            cmd += operands.at(0);
            cmd += " > ";
            cmd += operands.at(1);
        } else if (operatorType == IsGeq) {
            cmd += operands.at(0);
            cmd += " >= ";
            cmd += operands.at(1);
        } else if (operatorType == IsNeq) {
            cmd += operands.at(0);
            cmd += " != ";
            cmd += operands.at(1);
        } else if (operatorType == Printf) {
            cmd += "printf";
            for (const auto &op : operands) {
                cmd += " ";
                cmd += op;
            }
        } else if (operatorType == Scanf) {
            cmd += "scanf";
            for (const auto &op : operands) {
                cmd += " ";
                cmd += op;
            }
        } else if (operatorType == ArrayElemAssign) {
            cmd += operands.at(0);
            cmd += "[";
            cmd += operands.at(1);
            cmd += "]";
            cmd += " = ";
            cmd += operands.at(2);
        } else if (operatorType == ConstDef) {
            cmd += "const ";
            cmd += operands.at(0);
            cmd += " ";
            cmd += operands.at(1);
            cmd += " = ";
            cmd += operands.at(2);
        } else if (operatorType == VarStatement) {
            cmd += "var ";
            cmd += operands.at(0);
            cmd += " ";
            cmd += operands.at(1);
        } else if (operatorType == VarArrayStatement) {
            cmd += "var ";
            cmd += operands.at(0);
            cmd += " ";
            cmd += operands.at(1);
            cmd += "[";
            cmd += operands.at(2);
            cmd += "]";
        } else if (operatorType == LoopSaveRegStatus) {
            cmd += "LoopSaveRegStatus";
        } else if (operatorType == LoopRestoreRegStatus) {
            cmd += "LoopRestoreRegStatus";
        } else if (operatorType == DoWhileBNZ) {
            cmd += "BNZ ";
            cmd += operands.at(0);
        }
        return cmd;
    };
};

class TempVarGenerator {
public:
    explicit TempVarGenerator(SymbolTable *_symbolTable) : symbolTable{_symbolTable} {}

    string getTempVar() {
        string varName = "t" + to_string(curNO);
        while (symbolTable->searchName(varName) != nullptr) {
            varName = "t" + to_string(++curNO);
        }
        curNO++;
        return varName;
    }

private:
    int curNO = 0;

    SymbolTable *symbolTable;
};

class LabelGenerator {
public:
    string getLabel() {
        return "Label_" + to_string(curNO++);
    }

private:
    int curNO = 0;
};

#endif //INC_3___TYPEDEFINE_H
