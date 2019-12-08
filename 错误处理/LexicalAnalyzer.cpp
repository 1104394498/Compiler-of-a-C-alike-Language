#include "LexicalAnalyzer.h"
#include <string>
#include "TypeDefine.h"

using namespace std;

#define ADD_CHAR curString += c

#define POINTER_BACK fseek(fin, -1, SEEK_CUR)

#define OUTPUT(type)\
    string temp_s = type;\
    temp_s += " ";\
    result.back().push_back(temp_s + curString);\
    curString = "";\
    status = BEGIN

#define COMSTR(str) (curString == str)

#define LASTCHAR curString[curString.length()-1]

// Input the file name
LexicalAnalyzer::LexicalAnalyzer(const string &fin_name, vector<Error>* _allErrors) : allErrors(_allErrors) {
    const char *f = fin_name.c_str();
    fin = fopen(f, "r");
    curPos = 0;
    curLine = 1;
    result.emplace_back();

    analyze();

    fclose(fin);
}

void LexicalAnalyzer::analyze() {
    char c = get_char();
    while (true) {
        if (status == BEGIN) {
            if (isLetter(c)) {
                status = STRING;
                ADD_CHAR;
            } else if (isdigit(c)) {
                status = NUMCONST;
                ADD_CHAR;
            } else if (c == '\'') {
                status = CHARCONST;
            } else if (c == '\"') {
                status = STRINGCONST;
            } else if (isSymbol(c)) {
                status = SYMBOL;
                ADD_CHAR;
                if (c != '!' && c != '>' && c != '<' && c != '=') {
                    output_SYMBOL(curString, status);
                }
            }
        } else if (status == STRING) {
            if (isLetter(c) || isdigit(c)) {
                ADD_CHAR;
            } else {
                POINTER_BACK;
                output_STRING(curString, status);
            }
        } else if (status == NUMCONST) {
            if (isdigit(c)) {
                ADD_CHAR;
            } else {
                POINTER_BACK;
                output_NUMSTR(curString, status);
            }
        } else if (status == CHARCONST) {
            ADD_CHAR;
            c = get_char();
            if (curString[curString.length() - 1] == '\\') {
                // 转义字符的情况
                ADD_CHAR;
                c = get_char();
            }
            if (c != '\'') {
                handle_errors(QUOTATION_MARK_NOT_MATCH);
            }
            // handle errors: invalid character in char constant
            if (curString.size() != 1 ||
                (curString != "+" && curString != "-" && curString != "*" && curString != "/" &&
                 !isLetter(curString[0]) && !isdigit(curString[0]))) {
                handle_errors(INVALID_CHAR);
            }

            output_CHARCONST(curString, status);
        } else if (status == STRINGCONST) {
            if (c == '\"' && LASTCHAR != '\\') {
                output_STRINGCONST(curString, status);
            } else {
                ADD_CHAR;
            }
        } else {  // status == SYMBOL
            if (c == '=' && (LASTCHAR == '!' || LASTCHAR == '>' || LASTCHAR == '<' || LASTCHAR == '=')) {
                ADD_CHAR;
            } else if (!feof(fin)) {
                POINTER_BACK;
            }

            output_SYMBOL(curString, status);
        }

        if (feof(fin)) {
            break;
        }

        c = get_char();
    }
}

void LexicalAnalyzer::output_STRING(string &curString, LexicalAnalysisStatus &status) {
    if (COMSTR("const")) {
        OUTPUT("CONSTTK");
    } else if (COMSTR("int")) {
        OUTPUT("INTTK");
    } else if (COMSTR("char")) {
        OUTPUT("CHARTK");
    } else if (COMSTR("void")) {
        OUTPUT("VOIDTK");
    } else if (COMSTR("main")) {
        OUTPUT("MAINTK");
    } else if (COMSTR("if")) {
        OUTPUT("IFTK");
    } else if (COMSTR("else")) {
        OUTPUT("ELSETK");
    } else if (COMSTR("do")) {
        OUTPUT("DOTK");
    } else if (COMSTR("while")) {
        OUTPUT("WHILETK");
    } else if (COMSTR("for")) {
        OUTPUT("FORTK");
    } else if (COMSTR("scanf")) {
        OUTPUT("SCANFTK");
    } else if (COMSTR("printf")) {
        OUTPUT("PRINTFTK");
    } else if (COMSTR("return")) {
        OUTPUT("RETURNTK");
    } else {
        OUTPUT("IDENFR");
    }
}

void LexicalAnalyzer::output_NUMSTR(string &curString, LexicalAnalysisStatus &status) {
    OUTPUT("INTCON");
}

void LexicalAnalyzer::output_CHARCONST(string &curString, LexicalAnalysisStatus &status) {
    OUTPUT("CHARCON");
}

void LexicalAnalyzer::output_STRINGCONST(string &curString, LexicalAnalysisStatus &status) {
    OUTPUT("STRCON");
}

void LexicalAnalyzer::output_SYMBOL(string &curString, LexicalAnalysisStatus &status) {
    if (curString.length() == 1) {
        if (COMSTR("+")) {
            OUTPUT("PLUS");
        } else if (COMSTR("-")) {
            OUTPUT("MINU");
        } else if (COMSTR("*")) {
            OUTPUT("MULT");
        } else if (COMSTR("/")) {
            OUTPUT("DIV");
        } else if (COMSTR("<")) {
            OUTPUT("LSS");
        } else if (COMSTR(">")) {
            OUTPUT("GRE");
        } else if (COMSTR("=")) {
            OUTPUT("ASSIGN");
        } else if (COMSTR(";")) {
            OUTPUT("SEMICN");
        } else if (COMSTR(",")) {
            OUTPUT("COMMA");
        } else if (COMSTR("(")) {
            OUTPUT("LPARENT");
        } else if (COMSTR(")")) {
            OUTPUT("RPARENT");
        } else if (COMSTR("[")) {
            OUTPUT("LBRACK");
        } else if (COMSTR("]")) {
            OUTPUT("RBRACK");
        } else if (COMSTR("{")) {
            OUTPUT("LBRACE");
        } else if (COMSTR("}")) {
            OUTPUT("RBRACE");
        } else {
            // �Ƿ�SYMBOL
            handle_errors(INVALID_CHAR);
        }
    } else {
        if (COMSTR("<=")) {
            OUTPUT("LEQ");
        } else if (COMSTR(">=")) {
            OUTPUT("GEQ");
        } else if (COMSTR("==")) {
            OUTPUT("EQL");
        } else if (COMSTR("!=")) {
            OUTPUT("NEQ");
        } else {
            handle_errors(INVALID_CHAR);
        }
    }
}

void LexicalAnalyzer::handle_errors(ErrorTypes errorType) {
    // exit(1);
    allErrors->emplace_back(lineNO, 'a');
}

// Get current element
string LexicalAnalyzer::currentElem(int &line) {
    line = curLine;
    return result[curLine - 1][curPos];
}

// Get last element
string LexicalAnalyzer::lastElem(int &line) {
    if (curPos == 0) {
        curLine--;
        if (curLine <= 0) {
            return nullptr;
        }
        curPos = (int) result[curLine - 1].size() - 1;
    }

    while (result[curLine].empty()) {
        curLine--;
        if (curLine <= 0) {
            return nullptr;
        }
        curPos = (int) result[curLine - 1].size() - 1;
    }

    line = curLine;
    return result[curLine - 1][curPos];
}

// Get next element
string LexicalAnalyzer::nextElem(int &line) {
    // if curPos points to the last element of current line, add curLine
    if (curPos >= result[curLine - 1].size() - 1) {
        curLine++;
        if (curLine > result.size()) {
            return nullptr;
        }
        curPos = 0;
    }

    // skip empty line
    while (result[curLine - 1].empty()) {
        curLine++;
        if (curLine > result.size()) {
            return nullptr;
        }
        curPos = 0;
    }

    line = curLine;
    return result[curLine - 1][curPos];
}

int LexicalAnalyzer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int LexicalAnalyzer::isSymbol(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '<' || c == '='
           || c == '>' || c == '!' || c == ';' || c == ',' || c == '(' || c == ')'
           || c == '[' || c == ']' || c == '{' || c == '}';
}

int LexicalAnalyzer::pointerLast() {
    if (curPos == 0) {
        curLine--;
        if (curLine <= 0) {
            return 0;
        }
        curPos = (int) result[curLine - 1].size() - 1;
        while (result[curLine - 1].empty()) {
            curLine--;
            if (curLine <= 0) {
                return 0;
            }
            curPos = (int) result[curLine - 1].size() - 1;
        }
    } else {
        curPos--;
    }

    return 1;
}

int LexicalAnalyzer::pointerNext() {
    // if curPos points to the last element of current line, add curLine
    if (curPos >= result[curLine - 1].size() - 1) {
        curLine++;
        if (curLine > result.size()) {
            return 0;
        }
        curPos = 0;
        // skip empty line
        while (result[curLine - 1].empty()) {
            curLine++;
            if (curLine > result.size()) {
                return 0;
            }
            curPos = 0;
        }
    } else {
        curPos++;
    }
    return 1;
}

char LexicalAnalyzer::get_char() {
    char c = (char) getc(fin);
    if (c == '\r') {
        // 判断是否为\r\n
        c = (char) getc(fin);
        if (c != '\n') {
            POINTER_BACK;
        } else {
            c = (char) getc(fin);
        }

        // output the remaining curString
        if (curString.length() > 0) {
            if (status == STRING) {
                output_STRING(curString, status);
            } else if (status == NUMCONST) {
                output_NUMSTR(curString, status);
            } else if (status == CHARCONST) {
                output_CHARCONST(curString, status);
            } else if (status == STRINGCONST) {
                output_STRINGCONST(curString, status);
            } else if (status == SYMBOL) {
                output_SYMBOL(curString, status);
            }
        }
        lineNO++;
        result.emplace_back();
    } else if (c == '\n') {
        // output the remaining curString
        if (curString.length() > 0) {
            if (status == STRING) {
                output_STRING(curString, status);
            } else if (status == NUMCONST) {
                output_NUMSTR(curString, status);
            } else if (status == CHARCONST) {
                output_CHARCONST(curString, status);
            } else if (status == STRINGCONST) {
                output_STRINGCONST(curString, status);
            } else if (status == SYMBOL) {
                output_SYMBOL(curString, status);
            }
        }

        lineNO++;
        result.emplace_back();
    }
    return c;
}