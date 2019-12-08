#pragma once
#ifndef INC_3___Error_LEXICALANALYZER_H
#define INC_3___Error_LEXICALANALYZER_H

//#include <cstdlib>
#include <string>
#include <vector>
#include "Error_TypeDefine.h"
#include "TypeDefine.h"

using namespace std;

class Error_LexicalAnalyzer {
public:
    // Input the file name
    Error_LexicalAnalyzer(const string &fname, vector<Error> *_allErrors);

    // Get current element
    string currentElem(int &line);

    // Get last element
    string lastElem(int &line);

    // Get next element
    string nextElem(int &line);

    // Move the read pointer to the next position. If success, return 1; else return 0.
    int pointerNext();

    // Move the read pointer to the last position. If success, return 1; else return 0.
    int pointerLast();

    int getCurPos() {
        return curPos;
    }

private:
    LexicalAnalysisStatus status = BEGIN;
    string curString;

    // Analyze the file
    void analyze();

    // handle errors
    void handle_errors(ErrorTypes errorType);

    static int isLetter(char c);

    static int isSymbol(char c);

    void output_STRING(string &curString, LexicalAnalysisStatus &status);

    void output_NUMSTR(string &curString, LexicalAnalysisStatus &status);

    void output_CHARCONST(string &curString, LexicalAnalysisStatus &status);

    void output_STRINGCONST(string &curString, LexicalAnalysisStatus &status);

    void output_SYMBOL(string &curString, LexicalAnalysisStatus &status);

    char get_char();

    // The file read from
    FILE *fin;
    // The lexical analysis result of the file
    vector<vector<string> > result;
    // The read pointer in result
    int curLine;
    int curPos;

    vector<Error> *allErrors;

    int lineNO = 1;
};


#endif //INC_3___Error_LEXICALANALYZER_H
