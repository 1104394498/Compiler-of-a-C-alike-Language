﻿#pragma once

#ifndef INC_3___GRAMMARANALYZER_H
#define INC_3___GRAMMARANALYZER_H

#include  <string>
#include <vector>
#include <set>
#include "LexicalAnalyzer.h"
#include "SymbolTable.h"

using namespace std;

class GrammarAnalyzer {
public:
    explicit GrammarAnalyzer(const string &fin_name, vector<Error> *_allErrors,
                             vector<IntermediateCmd> *_intermediateCodes);

    void print(const string &fout_name);

    vector<IntermediateCmd> getIntermediateCodes() { return *intermediateCodes; }

private:
    int curLine = 1;    // record line NO.

    vector<Error> *allErrors;

    vector<IntermediateCmd> *intermediateCodes;

    void handle_errors(ErrorException &exception);

    static void handle_errors();

    int getsym();

    LexicalAnalyzer *ptr_lexicalAnalyzer;

    vector<string> result;

    string sym;
    string sym_type;

    // substituted by symbol table
    // record function names
    // set<string> functionName_with_returnValue;
    // set<string> functionName_without_returnValue;

    // Symbol Table
    SymbolTable symbolTable{};

    TempVarGenerator tempVarGenerator{&symbolTable};

    LabelGenerator labelGenerator{};

    void output_current_sym();

    void pointer_back(int times = 1);

    // recursive descent functions
    void program(); // add symbol stack
    void constant_statement();

    void constant_definition(); // duplicate name definition

    void variable_statement();

    void variable_definition(); // duplicate name definition
    void
    main_function();   // add symbol stack, duplicate name definition (for function name), surplus return  // add Label and Exit
    void
    function_with_return_value();  // add symbol stack, duplicate name definition (for function name and input variables' names), lack (dismatch) return
    void
    function_without_return_value();   // add symbol stack, duplicate name definition (for function name and input variables' names), surplus return
    string integer();

    string unsigned_integer();

    void statement_head(VariableType &returnType, string &funcName);    // add FuncDefStart

    void parameter_table(vector<VariableType> &inputTypes, vector<Item *> *newStack);   // add FuncPara

    void compound_statement(VariableType &returnType);

    void statement_array(VariableType &returnType);

    void statement(VariableType &returnType);   // undefined name

    void conditional_statement(VariableType &returnType);

    void condition();   // wrong type in condition

    void loop_statement(VariableType &returnType);  // undefined name

    string step();

    // assignName = nullptr means needn't return value
    void call_with_returnValue(VariableType &returnType,
                               const string *assignName);   // undefined name, function variables dismatch

    void call_without_returnValue();    // undefined name, function variables dismatch

    void assign_statement();    // undefined name, change constant value

    void value_parameter_table(vector<VariableType> *call_parameters_type);

    void read_statement();

    void print_statement();

    void return_statement(VariableType &returnType);

    void mystring();

    void relational_operator();

    // assign the result to assignVarName (can be optimized)
    void expression(VariableType &expressionType, const string &assignVarName);

    void item(VariableType &itemType, const string &assignVarName);

    void factor(VariableType &factorType, const string &assignVarName);
};


#endif //INC_3___GRAMMARANALYZER_H
