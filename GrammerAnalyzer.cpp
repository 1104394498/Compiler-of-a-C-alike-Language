#include "GrammerAnalyzer.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

#define SYMTYPE_ASSERT(str)\
    if (sym_type != str) {\
        handle_errors();\
    }

#define SYMTYPE_TWO_ASSERT(str1, str2)\
    if (sym_type != str1 && sym_type != str2) {\
        handle_errors();\
    }

#define CHECK_IDENFR_DEFINE\
    try {\
            if (symbolTable.searchName(sym) == nullptr) {\
            throw UndefinedName{};\
        }\
    } catch (UndefinedName &exception) {\
        handle_errors(exception);\
    }

#define ASSERT_THROW(type, exceptionName)\
    try {\
        if (sym_type != type) {\
            if (ptr_lexicalAnalyzer->getCurPos() == 1)\
                throw exceptionName{curLine - 1};\
            else\
                throw exceptionName{};\
        }\
    } catch (exceptionName &exception) {\
        pointer_back(1);\
        handle_errors(exception);\
    }

GrammarAnalyzer::GrammarAnalyzer(const string &fin_name, vector<Error> *_allErrors,
                                 vector<IntermediateCmd> *_intermediateCodes) : allErrors{_allErrors},
                                                                                intermediateCodes{_intermediateCodes} {
    ptr_lexicalAnalyzer = new LexicalAnalyzer{fin_name, _allErrors};

    // start recursive descent
    getsym();
    output_current_sym();
    program();

    delete ptr_lexicalAnalyzer;
}

// When ptr of lexicalAnalyzer is at the end of the vector, return 0; else return 1.
int GrammarAnalyzer::getsym() {
    static bool reach_end = false;
    if (reach_end) {
        // If try to get the next element while reaching end, go to handle_errors
        handle_errors();
    }
    sym = ptr_lexicalAnalyzer->currentElem(curLine);
    int pos = sym.find(' ');
    sym_type = sym.substr(0, pos);
    sym.erase(0, pos + 1);
    int ret = ptr_lexicalAnalyzer->pointerNext();
    if (ret == 0) {
        reach_end = true;
    }
    return ret;
}

void GrammarAnalyzer::output_current_sym() {
    result.emplace_back(sym_type + ' ' + sym);
}

void GrammarAnalyzer::pointer_back(int times) {
    for (int i = 0; i < times; i++) {
        ptr_lexicalAnalyzer->pointerLast();
    }
}

void GrammarAnalyzer::handle_errors(ErrorException &exception) {
    if (exception.getErrorLine() == -1) {
        allErrors->emplace_back(curLine, exception.getErrorType());
    } else {
        allErrors->emplace_back(exception.getErrorLine(), exception.getErrorType());
    }
    // skip current line
    /*
    int previousLine = curLine;
    do {
        getsym();
    } while (curLine == previousLine);
     */
}

void GrammarAnalyzer::handle_errors() {
    exit(1);
}

void GrammarAnalyzer::program() {
    // At the beginning of a program, create a new symbol stack
    symbolTable.pushStack();

    if (sym_type == "CONSTTK") {
        constant_statement();
    }

    bool variable_statement_finish = false;
    while (true) {
        if (sym_type == "INTTK" || sym_type == "CHARTK") {
            if (variable_statement_finish) {
                function_with_return_value();
            } else {
                // distinguish whether function with return value or variable statement
                getsym();
                // output_current_sym();
                SYMTYPE_ASSERT("IDENFR")

                getsym();
                // output_current_sym();
                if (sym_type == "LPARENT") {
                    // functions with return value
                    variable_statement_finish = true;
                    pointer_back(3);
                    getsym();
                    function_with_return_value();
                } else {
                    // variable statement
                    pointer_back(3);
                    getsym();
                    variable_statement();
                    variable_statement_finish = true;
                }
            }
        } else if (sym_type == "VOIDTK") {
            // distinguish the main function and the functions having return value
            getsym();
            output_current_sym();
            if (sym_type == "MAINTK") {
                // main function
                break;
            } else {
                // functions without return value
                function_without_return_value();
            }
        } else {
            // error
            handle_errors();
        }
    }

    main_function();

    result.emplace_back("<程序>");

    symbolTable.popStack();
}

void GrammarAnalyzer::constant_statement() {
    while (true) {
        getsym();
        output_current_sym();
        constant_definition();
        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)

        getsym();
        if (sym_type == "CONSTTK") {
            output_current_sym();
        } else {
            break;
        }
    }
    result.emplace_back("<常量说明>");
    output_current_sym();
}

void GrammarAnalyzer::constant_definition() {
    IntermediateCmd *cmd;
    if (sym_type == "INTTK") {
        while (true) {
            cmd = new IntermediateCmd{ConstDef};
            cmd->addOperands("int");
            getsym();
            output_current_sym();
            SYMTYPE_ASSERT("IDENFR");

            cmd->addOperands(sym);

            // add into the symbol table
            try {
                symbolTable.addName(sym, constInt, nullptr);
            } catch (DuplicateNameDefinition &exception) {
                // duplicate name definition
                handle_errors(exception);
            }

            getsym();
            output_current_sym();
            if (sym_type != "ASSIGN") {
                handle_errors();    // miss assign
            }
            getsym();
            output_current_sym();
            // check whether is integer constant
            bool hasSign = false;
            if (sym_type == "MINU" || sym_type == "PLUS") {
                getsym();
                hasSign = true;
            }
            if (sym_type != "INTCON") {
                try {
                    throw TypeErrorInConstantDefine{};
                } catch (TypeErrorInConstantDefine &exception) {
                    handle_errors(exception);
                    getsym();
                }
            } else {
                if (hasSign) {
                    pointer_back(2);
                } else {
                    pointer_back(1);
                }
                getsym();
                string integer_str = integer();
                cmd->addOperands(integer_str);
            }

            intermediateCodes->emplace_back(*cmd);
            delete cmd;

            if (sym_type != "COMMA") {
                break;
            }
        }
    } else if (sym_type == "CHARTK") {
        while (true) {
            cmd = new IntermediateCmd{ConstDef};
            cmd->addOperands("char");
            getsym();
            output_current_sym();
            SYMTYPE_ASSERT("IDENFR")
            cmd->addOperands(sym);

            // add into the symbol table
            try {
                symbolTable.addName(sym, constChar, nullptr);
            } catch (DuplicateNameDefinition &exception) {
                // duplicate name definition
                handle_errors(exception);
            }

            getsym();
            output_current_sym();
            SYMTYPE_ASSERT("ASSIGN");
            getsym();
            output_current_sym();

            // check whether is char constant
            if (sym_type != "CHARCON") {
                try {
                    throw TypeErrorInConstantDefine{};
                } catch (TypeErrorInConstantDefine &exception) {
                    handle_errors(exception);
                }
            }
            cmd->addOperands("\'" + sym + "\'");

            intermediateCodes->emplace_back(*cmd);
            delete cmd;
            // SYMTYPE_ASSERT("CHARCON");

            getsym();
            output_current_sym();
            if (sym_type != "COMMA") {
                break;
            }
        }
    } else {
        // miss int or char
        handle_errors();
    }
    result.insert(result.end() - 1, "<常量定义>");
}

void GrammarAnalyzer::variable_statement() {
    while (true) {
        variable_definition();
        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)

        // judge whether continue reading
        getsym();

        if (sym_type == "INTTK" || sym_type == "CHARTK") {
            getsym();
            SYMTYPE_ASSERT("IDENFR");
            getsym();
            if (sym_type == "LPARENT") {
                // functions with return value
                pointer_back(3);
                getsym();
                break;
            } else {
                // variable definition
                pointer_back(3);
                getsym();
                output_current_sym();
            }
        } else {
            break;
        }
    }
    result.emplace_back("<变量说明>");
    output_current_sym();
}

void GrammarAnalyzer::variable_definition() {
    SYMTYPE_TWO_ASSERT("INTTK", "CHARTK")
    VariableType variableType = (sym_type == "INTTK") ? intType : charType;
    const string typeOperand = (variableType == intType) ? "int" : "char";
    while (true) {
        getsym();
        output_current_sym();
        SYMTYPE_ASSERT("IDENFR")
        const string identifierName = sym;

        try {
            symbolTable.addName(sym, variableType, nullptr);
        } catch (DuplicateNameDefinition &exception) {
            // duplicate name definition
            handle_errors(exception);
        }

        getsym();
        output_current_sym();
        if (sym_type == "LBRACK") {
            getsym();
            output_current_sym();
            const string& size = unsigned_integer();
            ASSERT_THROW("RBRACK", ShouldHaveRBRACK)

            IntermediateCmd cmd{VarArrayStatement};
            cmd.addOperands(typeOperand);
            cmd.addOperands(identifierName);
            cmd.addOperands(size);
            intermediateCodes->push_back(cmd);

            getsym();
            output_current_sym();
            if (sym_type != "COMMA") {
                break;
            }
        } else {
            IntermediateCmd cmd{VarStatement};
            cmd.addOperands(typeOperand);
            cmd.addOperands(identifierName);
            intermediateCodes->push_back(cmd);
            if (sym_type != "COMMA")
                break;
        }
    }

    result.insert(result.end() - 1, "<变量定义>");
}

void GrammarAnalyzer::main_function() {
    intermediateCodes->emplace_back(FuncDefStart);
    intermediateCodes->back().addOperands("void");  // The label of main function is MAIN
    intermediateCodes->back().addOperands("main");

    auto *mainFuncInfo = new FuncInfo{voidType, vector<VariableType>{}};
    try {
        // the name "main" belongs to the outer stack in symbolTable
        symbolTable.addName("main", functionType, mainFuncInfo);
    } catch (DuplicateNameDefinition &exception) {
        handle_errors(exception);
    }


    symbolTable.pushStack();

    // because merge with non-returnValue function
    // SYMTYPE_ASSERT("VOIDTK")
    SYMTYPE_ASSERT("MAINTK")

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LPARENT")

    getsym();
    output_current_sym();
    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LBRACE")

    getsym();
    output_current_sym();

    VariableType returnType;
    compound_statement(returnType);
    try {
        if (returnType != voidType) {
            throw SurplusReturn{curLine - 1};     // revise here, assume return statement is always the last statement
        }
    } catch (SurplusReturn &exception) {
        handle_errors(exception);
    }

    SYMTYPE_ASSERT("RBRACE")

    result.emplace_back("<主函数>");

    /* because main function is the last element
    getsym();
    output_current_sym();
    */

    symbolTable.popStack();

    intermediateCodes->emplace_back(OperatorType::Exit);
}

void GrammarAnalyzer::function_with_return_value() {
    // funcInfo will be filled later
    VariableType returnType;
    string funcName;
    vector<VariableType> inputTypes;
    auto *newStack = new vector<Item *>{};

    SYMTYPE_TWO_ASSERT("INTTK", "CHARTK");
    statement_head(returnType, funcName);

    SYMTYPE_ASSERT("LPARENT")
    getsym();
    output_current_sym();

    parameter_table(inputTypes, newStack);

    // first, add the function name and info into the outer stack
    try {
        symbolTable.addName(funcName, functionType, new FuncInfo(returnType, inputTypes));
    } catch (DuplicateNameDefinition &exception) {
        handle_errors(exception);
    }

    // then add the function's parameters into the new stack
    try {
        symbolTable.pushStack(newStack);
    } catch (DuplicateNameDefinition &exception) {
        handle_errors(exception);
    }

    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LBRACE")

    getsym();
    output_current_sym();
    VariableType actualReturnType;
    compound_statement(actualReturnType);
    try {
        if (actualReturnType != returnType) {
            throw LackReturn{curLine - 1};  // revise here, assume that return statement is always the last statement
        }
    } catch (LackReturn &exception) {
        handle_errors(exception);
    }


    SYMTYPE_ASSERT("RBRACE")

    result.emplace_back("<有返回值函数定义>");

    getsym();
    output_current_sym();

    symbolTable.popStack();
}

void GrammarAnalyzer::statement_head(VariableType &returnType, string &funcName) {
    SYMTYPE_TWO_ASSERT("INTTK", "CHARTK")
    returnType = (sym_type == "INTTK") ? intType : charType;

    getsym();
    output_current_sym();

    SYMTYPE_ASSERT("IDENFR")
    funcName = sym;

    result.emplace_back("<声明头部>");

    getsym();
    output_current_sym();

    // intermediate code
    intermediateCodes->emplace_back(OperatorType::FuncDefStart);
    if (returnType == intType) {
        intermediateCodes->back().addOperands("int");
    } else {
        intermediateCodes->back().addOperands("char");
    }
    intermediateCodes->back().addOperands(funcName);
}

void GrammarAnalyzer::parameter_table(vector<VariableType> &inputTypes, vector<Item *> *newStack) {
    if (sym_type == "RPARENT") {
        // parameter table is empty
        result.insert(result.end() - 1, "<参数表>");
        return;
    }

    while (true) {
        SYMTYPE_TWO_ASSERT("INTTK", "CHARTK")
        VariableType variableType;
        if (sym_type == "INTTK") {
            variableType = intType;
        } else {
            variableType = charType;
        }
        inputTypes.emplace_back(variableType);

        getsym();
        output_current_sym();

        SYMTYPE_ASSERT("IDENFR")
        string variableName = sym;
        //newStack.emplace_back(variable);
        newStack->push_back(new Item{variableName, variableType, nullptr});

        // intermediate codes
        intermediateCodes->emplace_back(OperatorType::FuncPara);
        if (variableType == intType) {
            intermediateCodes->back().addOperands("int");
        } else {
            intermediateCodes->back().addOperands("char");
        }
        intermediateCodes->back().addOperands(variableName);

        getsym();
        if (sym_type == "COMMA") {
            output_current_sym();
            getsym();
            output_current_sym();
        } else {
            break;
        }
    }

    result.emplace_back("<参数表>");
    output_current_sym();
}

void GrammarAnalyzer::compound_statement(VariableType &returnType) {
    returnType = voidType;
    if (sym_type == "CONSTTK") {
        constant_statement();
    }

    if (sym_type == "INTTK" || sym_type == "CHARTK") {
        variable_statement();
    }

    statement_array(returnType);
    result.insert(result.end() - 1, "<复合语句>");
}

void GrammarAnalyzer::statement_array(VariableType &returnType) {
    returnType = voidType;
    while (sym_type == "IFTK" || sym_type == "WHILETK" || sym_type == "DOTK" || sym_type == "FORTK"
           || sym_type == "LBRACE" || sym_type == "IDENFR" || sym_type == "SCANFTK" || sym_type == "PRINTFTK" ||
           sym_type == "RETURNTK" || sym_type == "SEMICN") {
        VariableType statementReturnType;
        statement(statementReturnType);
        if (statementReturnType != voidType) {
            returnType = statementReturnType;
        }
    }
    result.insert(result.end() - 1, "<语句列>");
}

void GrammarAnalyzer::statement(VariableType &returnType) {
    returnType = voidType;
    if (sym_type == "IFTK") {
        // conditional statement
        conditional_statement(returnType);
    } else if (sym_type == "WHILETK" || sym_type == "DOTK" || sym_type == "FORTK") {
        // loop statement
        loop_statement(returnType);
    } else if (sym_type == "LBRACE") {
        getsym();
        output_current_sym();
        // statement array
        statement_array(returnType);

        SYMTYPE_ASSERT("RBRACE");

        getsym();
        output_current_sym();
    } else if (sym_type == "IDENFR") {
        getsym();
        if (sym_type == "ASSIGN" || sym_type == "LBRACK") {
            // assign statement
            pointer_back(2);
            getsym();
            assign_statement();

            ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
            getsym();
            output_current_sym();
        } else {
            // call statement
            pointer_back(2);
            getsym();

            const Item *find_result = symbolTable.searchName(sym);
            try {
                if (find_result == nullptr) {
                    throw UndefinedName{};
                } else if (find_result->type != functionType) {
                    throw UndefinedName{};
                } else {
                    const auto *funcInfo = static_cast<const FuncInfo *>(find_result->info);
                    if (funcInfo->getReturnType() == voidType) {
                        call_without_returnValue();

                        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
                        getsym();
                        output_current_sym();
                    } else {
                        VariableType uselessType;   // useless
                        call_with_returnValue(uselessType, nullptr);

                        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
                        getsym();
                        output_current_sym();
                    }
                }
            } catch (UndefinedName &exception) {
                handle_errors(exception);
                getsym();
                output_current_sym();
                SYMTYPE_ASSERT("LPARENT")

                getsym();
                output_current_sym();
                value_parameter_table(nullptr);

                ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

                result.emplace_back("<函数名未定义调用语句>");    // maybe there's a problem
                getsym();
                output_current_sym();
                ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
                getsym();
                output_current_sym();
            }
        }
    } else if (sym_type == "SCANFTK") {
        // read statement
        read_statement();

        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
        getsym();
        output_current_sym();
    } else if (sym_type == "PRINTFTK") {
        // print statement
        print_statement();

        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
        getsym();
        output_current_sym();
    } else if (sym_type == "RETURNTK") {
        // return statement
        return_statement(returnType);

        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
        getsym();
        output_current_sym();
    } else {
        // empty
        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)
        getsym();
        output_current_sym();
    }

    result.insert(result.end() - 1, "<语句>");
}

void GrammarAnalyzer::conditional_statement(VariableType &returnType) {
    SYMTYPE_ASSERT("IFTK");

    getsym();
    output_current_sym();

    SYMTYPE_ASSERT("LPARENT")

    getsym();
    output_current_sym();

    condition();

    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    getsym();
    output_current_sym();
    statement(returnType);

    if (sym_type == "ELSETK") {
        getsym();
        output_current_sym();
        statement(returnType);
    }

    result.insert(result.end() - 1, "<条件语句>");

}

void GrammarAnalyzer::loop_statement(VariableType &returnType) {
    if (sym_type == "WHILETK") {
        string beginLabel = labelGenerator.getLabel();
        intermediateCodes->emplace_back(Label);
        intermediateCodes->back().addOperands(beginLabel);

        string endLabel = labelGenerator.getLabel();

        getsym();
        output_current_sym();

        SYMTYPE_ASSERT("LPARENT")
        getsym();
        output_current_sym();

        condition();
        intermediateCodes->emplace_back(BZ);
        intermediateCodes->back().addOperands(endLabel);


        ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

        getsym();
        output_current_sym();
        statement(returnType);

        intermediateCodes->emplace_back(Goto);
        intermediateCodes->back().addOperands(beginLabel);

        intermediateCodes->emplace_back(Label);
        intermediateCodes->back().addOperands(endLabel);
    } else if (sym_type == "DOTK") {
        string beginLabel = labelGenerator.getLabel();
        intermediateCodes->emplace_back(Label);
        intermediateCodes->back().addOperands(beginLabel);

        getsym();
        output_current_sym();
        statement(returnType);

        ASSERT_THROW("WHILETK", LackWhileInDoWhile)
        getsym();
        output_current_sym();
        SYMTYPE_ASSERT("LPARENT")

        getsym();
        output_current_sym();

        condition();

        intermediateCodes->emplace_back(BNZ);
        intermediateCodes->back().addOperands(beginLabel);

        ASSERT_THROW("RPARENT", ShouldHaveRPARENT)
        getsym();
        output_current_sym();
    } else if (sym_type == "FORTK") {
        string beginLabel = labelGenerator.getLabel();
        string endLabel = labelGenerator.getLabel();

        getsym();
        output_current_sym();
        SYMTYPE_ASSERT("LPARENT")

        getsym();
        output_current_sym();
        string identifier = sym;
        SYMTYPE_ASSERT("IDENFR")
        CHECK_IDENFR_DEFINE


        getsym();
        output_current_sym();
        SYMTYPE_ASSERT("ASSIGN")

        getsym();
        output_current_sym();
        expression(returnType, identifier);

        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)

        getsym();
        output_current_sym();

        intermediateCodes->emplace_back(Label);
        intermediateCodes->back().addOperands(beginLabel);

        condition();

        intermediateCodes->emplace_back(BZ);
        intermediateCodes->back().addOperands(endLabel);

        ASSERT_THROW("SEMICN", ShouldHaveSemicolon)

        string identifier1, identifier2;

        getsym();
        output_current_sym();
        SYMTYPE_ASSERT("IDENFR")
        CHECK_IDENFR_DEFINE
        identifier1 = sym;

        getsym();
        output_current_sym();
        SYMTYPE_ASSERT("ASSIGN")

        getsym();
        output_current_sym();
        SYMTYPE_ASSERT("IDENFR")
        CHECK_IDENFR_DEFINE
        identifier2 = sym;

        getsym();
        output_current_sym();
        SYMTYPE_TWO_ASSERT("MINU", "PLUS")

        IntermediateCmd *step_cmd = (sym_type == "PLUS") ? (new IntermediateCmd{Add}) : (new IntermediateCmd{Minus});

        getsym();
        output_current_sym();
        string step_str = step();

        step_cmd->addOperands(identifier1);
        step_cmd->addOperands(identifier2);
        step_cmd->addOperands(step_str);

        ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

        getsym();
        output_current_sym();
        statement(returnType);

        intermediateCodes->emplace_back(*step_cmd);
        delete step_cmd;

        intermediateCodes->emplace_back(Goto);
        intermediateCodes->back().addOperands(beginLabel);

        intermediateCodes->emplace_back(Label);
        intermediateCodes->back().addOperands(endLabel);
    } else {
        handle_errors();
    }

    result.insert(result.end() - 1, "<循环语句>");
}

void GrammarAnalyzer::condition() {
    VariableType conditionType = intType;
    VariableType expressionType;
    IntermediateCmd *cmd;

    string tempVarName1 = tempVarGenerator.getTempVar();
    expression(expressionType, tempVarName1);
    if (expressionType != intType && expressionType != constInt) {
        conditionType = voidType;   // just make it wrong
    }

    if (sym_type == "LSS" || sym_type == "LEQ" || sym_type == "GRE"
        || sym_type == "GEQ" || sym_type == "EQL" || sym_type == "NEQ") {

        // tempVarName1 relational_operator tempVarName2
        if (sym_type == "LSS")
            cmd = new IntermediateCmd(IsLess);
        else if (sym_type == "LEQ")
            cmd = new IntermediateCmd(IsLeq);
        else if (sym_type == "GRE")
            cmd = new IntermediateCmd(IsGreater);
        else if (sym_type == "GEQ")
            cmd = new IntermediateCmd(IsGeq);
        else if (sym_type == "EQL")
            cmd = new IntermediateCmd(IsEqual);
        else
            cmd = new IntermediateCmd(IsNeq);

        relational_operator();

        string tempVarName2 = tempVarGenerator.getTempVar();
        expression(expressionType, tempVarName2);

        cmd->addOperands(tempVarName1);
        cmd->addOperands(tempVarName2);

        if (expressionType != intType && expressionType != constInt) {
            conditionType = voidType;   // just make it wrong
        }
    } else {
        // tempVarName1 != 0
        cmd = new IntermediateCmd(IsNeq);
        cmd->addOperands(tempVarName1);
        cmd->addOperands("0");
    }

    intermediateCodes->emplace_back(*cmd);
    delete (cmd);

    try {
        if (conditionType != intType) {
            throw InvalidConditionType{};
        }
    } catch (InvalidConditionType &exception) {
        handle_errors(exception);
    }

    // because expression has add the next word into result
    result.insert(result.end() - 1, "<条件>");
}

void GrammarAnalyzer::function_without_return_value() {
    // funcInfo will be filled later
    string funcName;
    vector<VariableType> inputTypes;
    auto *newStack = new vector<Item *>{};

    // 已经与main函数的void合并
    //SYMTYPE_ASSERT("VOIDTK")
    //getsym();
    //output_current_sym();
    SYMTYPE_ASSERT("IDENFR")
    funcName = sym;

    intermediateCodes->emplace_back(FuncDefStart);
    intermediateCodes->back().addOperands("void");
    intermediateCodes->back().addOperands(funcName);

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LPARENT")

    getsym();
    output_current_sym();
    parameter_table(inputTypes, newStack);

    // first, add the function name and info into the outer stack
    try {
        symbolTable.addName(funcName, functionType, new FuncInfo(voidType, inputTypes));
    } catch (DuplicateNameDefinition &exception) {
        handle_errors(exception);
    }

    // then add the function's parameters into the new stack
    try {
        symbolTable.pushStack(newStack);
    } catch (DuplicateNameDefinition &exception) {
        handle_errors(exception);
    }

    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LBRACE");

    getsym();
    output_current_sym();
    VariableType returnType;
    compound_statement(returnType);

    try {
        if (returnType != voidType) {
            throw SurplusReturn{
                    curLine - 1};   // revise here, assume that return statement is always the last statement
        }
    } catch (SurplusReturn &exception) {
        handle_errors(exception);
    }

    SYMTYPE_ASSERT("RBRACE")

    intermediateCodes->emplace_back(FuncRetInDef);

    result.emplace_back("<无返回值函数定义>");

    getsym();
    output_current_sym();

    symbolTable.popStack();
}

string GrammarAnalyzer::integer() {
    string ret;
    if (sym_type == "MINU" || sym_type == "PLUS") {
        if (sym_type == "MINU")
            ret += "-";
        getsym();
        output_current_sym();
    }
    ret += unsigned_integer();

    // because in unsigned integer, the next sym has been added into result
    result.insert(result.end() - 1, "<整数>");

    return ret;
}

string GrammarAnalyzer::unsigned_integer() {
    string ret;
    SYMTYPE_ASSERT("INTCON")
    if (sym.length() > 1 && !(sym[0] >= '1' && sym[0] <= '9')) {
        handle_errors();
    }
    ret = sym;

    result.emplace_back("<无符号整数>");
    getsym();
    output_current_sym();

    return ret;
}

void GrammarAnalyzer::call_with_returnValue(VariableType &returnType, const string *assignName) {
    SYMTYPE_ASSERT("IDENFR")
    const string funcName = sym;
    const Item *find_result = symbolTable.searchName(sym);
    try {
        if (find_result == nullptr || find_result->type != functionType) {
            throw UndefinedName{};
        } else {
            const auto *funcInfo = static_cast<const FuncInfo *>(find_result->info);
            if (funcInfo->getReturnType() != intType && funcInfo->getReturnType() != charType) {
                throw UndefinedName{};
            } else {
                returnType = funcInfo->getReturnType();
            }
        }
    } catch (UndefinedName &exception) {
        handle_errors(exception);
        returnType = voidType;
    }


    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LPARENT")

    auto *call_parameters_type = new vector<VariableType>{};

    getsym();
    output_current_sym();
    value_parameter_table(call_parameters_type);

    // intermediate codes
    intermediateCodes->emplace_back(SaveEnv);
    intermediateCodes->emplace_back(CallFunc);
    intermediateCodes->back().addOperands(funcName);

    intermediateCodes->emplace_back(RestoreEnv);

    if (assignName != nullptr) {
        intermediateCodes->emplace_back(AssignRetValue);
        intermediateCodes->back().addOperands(*assignName);
    }

    if (find_result != nullptr && find_result->type == functionType) {
        try {
            auto funcInputTypes = static_cast<FuncInfo *>(find_result->info)->getInputTypes();
            if (call_parameters_type->size() != funcInputTypes.size()) {
                throw WrongFuncVariableNum{};
            } else {
                auto iter1 = call_parameters_type->cbegin();
                auto iter2 = funcInputTypes.cbegin();
                while (iter1 != call_parameters_type->cend()) {
                    if (*iter1 != *iter2) {
                        throw WrongFuncVariableType{};
                    }
                    iter1++;
                    iter2++;
                }
            }
        } catch (WrongFuncVariableNum &exception) {
            handle_errors(exception);
        } catch (WrongFuncVariableType &exception) {
            handle_errors(exception);
        }
    }

    delete (call_parameters_type);

    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    result.emplace_back("<有返回值函数调用语句>");
    getsym();
    output_current_sym();
}

void GrammarAnalyzer::call_without_returnValue() {
    SYMTYPE_ASSERT("IDENFR")
    string funcName = sym;

    const Item *find_result = symbolTable.searchName(sym);
    try {
        if (find_result == nullptr || find_result->type != functionType) {
            throw UndefinedName{};
        } else {
            const auto *funcInfo = static_cast<const FuncInfo *>(find_result->info);
            if (funcInfo->getReturnType() != voidType) {
                throw UndefinedName{};
            }
        }
    } catch (UndefinedName &exception) {
        handle_errors(exception);
    }

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LPARENT")

    auto *call_parameters_type = new vector<VariableType>{};

    getsym();
    output_current_sym();
    value_parameter_table(call_parameters_type);

    // intermediate codes
    intermediateCodes->emplace_back(SaveEnv);
    intermediateCodes->emplace_back(CallFunc);
    intermediateCodes->back().addOperands(funcName);
    intermediateCodes->emplace_back(RestoreEnv);

    if (find_result != nullptr && find_result->type == functionType) {
        try {
            auto funcInputTypes = static_cast<FuncInfo *>(find_result->info)->getInputTypes();
            if (call_parameters_type->size() != funcInputTypes.size()) {
                throw WrongFuncVariableNum{};
            } else {
                auto iter1 = call_parameters_type->cbegin();
                auto iter2 = funcInputTypes.cbegin();
                while (iter1 != call_parameters_type->cend()) {
                    if (*iter1 != *iter2) {
                        throw WrongFuncVariableType{};
                    }
                    iter1++;
                    iter2++;
                }
            }
        } catch (WrongFuncVariableNum &exception) {
            handle_errors(exception);
        } catch (WrongFuncVariableType &exception) {
            handle_errors(exception);
        }
    }

    delete (call_parameters_type);

    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    result.emplace_back("<无返回值函数调用语句>");
    getsym();
    output_current_sym();
}

void GrammarAnalyzer::read_statement() {
    SYMTYPE_ASSERT("SCANFTK")

    intermediateCodes->emplace_back(Scanf);

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LPARENT");

    getsym();
    output_current_sym();
    while (true) {
        SYMTYPE_ASSERT("IDENFR")
        CHECK_IDENFR_DEFINE

        intermediateCodes->back().addOperands(sym);

        getsym();
        output_current_sym();
        if (sym_type != "COMMA") {
            break;
        }
        getsym();
        output_current_sym();
    }

    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    result.emplace_back("<读语句>");
    getsym();
    output_current_sym();
}

void GrammarAnalyzer::print_statement() {
    SYMTYPE_ASSERT("PRINTFTK")

    getsym();
    output_current_sym();
    SYMTYPE_ASSERT("LPARENT")

    IntermediateCmd cmd{Printf};

    getsym();
    output_current_sym();
    if (sym_type == "STRCON") {
        cmd.addOperands("\"" + sym + "\"");
        mystring();
        if (sym_type == "COMMA") {
            getsym();
            output_current_sym();
            VariableType uselessType;
            string tempVarName = tempVarGenerator.getTempVar();
            expression(uselessType, tempVarName);
            cmd.addOperands(tempVarName);
        }
    } else {
        VariableType uselessType;
        string tempVarName = tempVarGenerator.getTempVar();
        expression(uselessType, tempVarName);
        cmd.addOperands(tempVarName);
    }
    ASSERT_THROW("RPARENT", ShouldHaveRPARENT)

    intermediateCodes->emplace_back(cmd);

    result.emplace_back("<写语句>");
    getsym();
    output_current_sym();
}

void GrammarAnalyzer::return_statement(VariableType &returnType) {
    SYMTYPE_ASSERT("RETURNTK")

    getsym();
    if (sym_type == "LPARENT") {
        output_current_sym();
        getsym();
        output_current_sym();

        string tempVarName = tempVarGenerator.getTempVar();
        expression(returnType, tempVarName);
        intermediateCodes->emplace_back(FuncRetInDef);
        intermediateCodes->back().addOperands(tempVarName);

        ASSERT_THROW("RPARENT", ShouldHaveRPARENT)
        getsym();
    } else {
        returnType = voidType;
    }
    result.emplace_back("<返回语句>");
    output_current_sym();
}

void GrammarAnalyzer::mystring() {
    SYMTYPE_ASSERT("STRCON")
    result.emplace_back("<字符串>");

    getsym();
    output_current_sym();
}

void GrammarAnalyzer::expression(VariableType &expressionType, const string &assignVarName) {
    bool negStart = false;
    if (sym_type == "PLUS" || sym_type == "MINU") {
        if (sym_type == "MINU") {
            negStart = true;
        }
        getsym();
        output_current_sym();
    }
    string tempVar = tempVarGenerator.getTempVar();

    item(expressionType, tempVar);

    // intermediate codes
    if (negStart) {
        intermediateCodes->emplace_back(Neg);
        intermediateCodes->back().addOperands(assignVarName);
        intermediateCodes->back().addOperands(tempVar);
    } else {
        intermediateCodes->emplace_back(Assign);
        intermediateCodes->back().addOperands(assignVarName);
        intermediateCodes->back().addOperands(tempVar);
    }

    IntermediateCmd *cmd;
    while (sym_type == "PLUS" || sym_type == "MINU") {
        cmd = (sym_type == "PLUS") ? (new IntermediateCmd{Add}) : (new IntermediateCmd{Minus});
        getsym();
        output_current_sym();
        string newTempVar = tempVarGenerator.getTempVar();
        item(expressionType, newTempVar);

        // intermediate codes
        cmd->addOperands(assignVarName);
        cmd->addOperands(assignVarName);
        cmd->addOperands(newTempVar);
        intermediateCodes->emplace_back(*cmd);
        delete cmd;

        expressionType = intType;
    }

    result.insert(result.end() - 1, "<表达式>");
}

string GrammarAnalyzer::step() {
    string ret = unsigned_integer();
    result.insert(result.end() - 1, "<步长>");
    return ret;
}

void GrammarAnalyzer::assign_statement() {
    SYMTYPE_ASSERT("IDENFR")
    IntermediateCmd *cmd;
    string identifierName = sym;

    CHECK_IDENFR_DEFINE

    auto *searched_item = symbolTable.searchName(sym);
    if (searched_item != nullptr) {
        try {
            if (searched_item->type == constInt || searched_item->type == constChar) {
                throw ChangeConstantValue{};
            }
        } catch (ChangeConstantValue &exception) {
            handle_errors(exception);
        }
    }

    getsym();
    output_current_sym();

    if (sym_type == "LBRACK") {
        getsym();
        output_current_sym();
        VariableType subscriptType;
        string tempVarName = tempVarGenerator.getTempVar();
        expression(subscriptType, tempVarName);
        cmd = new IntermediateCmd{ArrayElemAssign};
        // intermediateCodes->emplace_back(ArrayElemAssign);
        cmd->addOperands(identifierName);
        cmd->addOperands(tempVarName);
        try {
            if (subscriptType != intType && subscriptType != constInt) {
                throw SubscriptNotInt{};
            }
        } catch (SubscriptNotInt &exception) {
            handle_errors(exception);
        }

        ASSERT_THROW("RBRACK", ShouldHaveRBRACK)

        getsym();
        output_current_sym();
    } else {
        cmd = new IntermediateCmd{Assign};
        //intermediateCodes->emplace_back(Assign);
        cmd->addOperands(identifierName);
    }

    SYMTYPE_ASSERT("ASSIGN");

    getsym();
    output_current_sym();
    VariableType uselessType;
    string tempVarName = tempVarGenerator.getTempVar();
    expression(uselessType, tempVarName);
    cmd->addOperands(tempVarName);
    intermediateCodes->emplace_back(*cmd);
    delete cmd;

    result.insert(result.end() - 1, "<赋值语句>");
}

void GrammarAnalyzer::value_parameter_table(vector<VariableType> *call_parameters_type) {
    vector<string> toPushVar;
    if (sym_type != "RPARENT") {
        string varName;
        while (true) {
            VariableType expressionType;
            varName = tempVarGenerator.getTempVar();
            expression(expressionType, varName);

            toPushVar.emplace_back(varName);

            if (call_parameters_type != nullptr)
                call_parameters_type->push_back(expressionType);
            if (sym_type != "COMMA") {
                break;
            }
            getsym();
            output_current_sym();
        }
    }

    // intermediate codes
    for (const string &name : toPushVar) {
        intermediateCodes->emplace_back(CallFuncPush);
        intermediateCodes->back().addOperands(name);
    }

    result.insert(result.end() - 1, "<值参数表>");
}

void GrammarAnalyzer::relational_operator() {
    if (sym_type == "LSS" || sym_type == "LEQ" || sym_type == "GRE"
        || sym_type == "GEQ" || sym_type == "EQL" || sym_type == "NEQ") {
        getsym();
        output_current_sym();
    } else {
        handle_errors();
    }
}

void GrammarAnalyzer::item(VariableType &itemType, const string &assignVarName) {
    int iterTimes = 1;
    string lastOp;
    while (true) {
        string tempVarName = tempVarGenerator.getTempVar();
        factor(itemType, tempVarName);
        if (iterTimes == 1) {
            intermediateCodes->emplace_back(Assign);
            intermediateCodes->back().addOperands(assignVarName);
            intermediateCodes->back().addOperands(tempVarName);
        } else {
            if (lastOp == "MULT") {
                intermediateCodes->emplace_back(Mul);
            } else {
                intermediateCodes->emplace_back(Div);
            }
            intermediateCodes->back().addOperands(assignVarName);
            intermediateCodes->back().addOperands(assignVarName);
            intermediateCodes->back().addOperands(tempVarName);

            itemType = intType;
        }

        if (sym_type != "MULT" && sym_type != "DIV") {
            break;
        } else {
            lastOp = sym_type;
        }
        getsym();
        output_current_sym();
        iterTimes++;
    }

    result.insert(result.end() - 1, "<项>");
}

void GrammarAnalyzer::factor(VariableType &factorType, const string &assignVarName) {
    if (sym_type == "IDENFR") {
        string identifierName = sym;

        const Item *searched_item = symbolTable.searchName(sym);
        try {
            // Here I assume that int and const int are different, the same with char and const char
            // No problem
            if (searched_item == nullptr) {
                throw UndefinedName{};
            } else if (searched_item->type == intType || searched_item->type == intArray) {
                factorType = intType;
            } else if (searched_item->type == constInt) {
                factorType = constInt;
            } else if (searched_item->type == charType || searched_item->type == charArray) {
                factorType = charType;
            } else {
                factorType = constChar;
            }
        } catch (UndefinedName &exception) {
            handle_errors(exception);
            factorType = voidType;   // random assign
        }

        getsym();
        output_current_sym();
        if (sym_type == "LBRACK") {
            //identifier + [
            getsym();
            output_current_sym();
            VariableType subscriptType;

            string tempVarName = tempVarGenerator.getTempVar();
            expression(subscriptType, tempVarName);

            // assignVarName = identifierName[tempVarName]
            intermediateCodes->emplace_back(GetArrayValue);
            intermediateCodes->back().addOperands(assignVarName);
            intermediateCodes->back().addOperands(identifierName);
            intermediateCodes->back().addOperands(tempVarName);

            try {
                if (subscriptType != intType && subscriptType != constInt) {
                    throw SubscriptNotInt{};
                }
            } catch (SubscriptNotInt &exception) {
                handle_errors(exception);
            }

            ASSERT_THROW("RBRACK", ShouldHaveRBRACK)
            getsym();
            output_current_sym();
        } else if (sym_type == "LPARENT") {
            // call function with return value
            pointer_back(2);
            // because output one more LPARENT
            result.pop_back();
            getsym();

            // tempVarName = func(...)
            // assignVarName = tempVarName
            string tempVarName = tempVarGenerator.getTempVar();
            call_with_returnValue(factorType, &tempVarName);

            intermediateCodes->emplace_back(Assign);
            intermediateCodes->back().addOperands(assignVarName);
            intermediateCodes->back().addOperands(tempVarName);

        } else { // else: only identifier
            // assignVarName = identifierName
            intermediateCodes->emplace_back(Assign);
            intermediateCodes->back().addOperands(assignVarName);
            intermediateCodes->back().addOperands(identifierName);
        }
    } else if (sym_type == "LPARENT") {
        getsym();
        output_current_sym();

        // tempVarName = expression
        // assignVarName = tempVarName
        string tempVarName = tempVarGenerator.getTempVar();
        expression(factorType, tempVarName);

        intermediateCodes->emplace_back(Assign);
        intermediateCodes->back().addOperands(assignVarName);
        intermediateCodes->back().addOperands(tempVarName);

        ASSERT_THROW("RPARENT", ShouldHaveRPARENT)
        getsym();
        output_current_sym();
    } else if (sym_type == "CHARCON") {
        factorType = charType;
        intermediateCodes->emplace_back(Assign);
        intermediateCodes->back().addOperands(assignVarName);
        intermediateCodes->back().addOperands("\'" + sym + "\'");

        getsym();
        output_current_sym();
    } else if (sym_type == "PLUS" || sym_type == "MINU" || sym_type == "INTCON") {
        factorType = intType;
        string interger_str = integer();

        intermediateCodes->emplace_back(Assign);
        intermediateCodes->back().addOperands(assignVarName);
        intermediateCodes->back().addOperands(interger_str);
    } else {
        handle_errors();
    }

    result.insert(result.end() - 1, "<因子>");
}

void GrammarAnalyzer::print(const string &fout_name) {
    ofstream fout{fout_name};

    for (const string &s : result) {
        fout << s << endl;
    }

    fout.close();
}