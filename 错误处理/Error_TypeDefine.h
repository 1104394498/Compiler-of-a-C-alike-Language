#pragma once

#ifndef INC_3___TYPEDEFINE_H
#define INC_3___TYPEDEFINE_H

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


#endif //INC_3___TYPEDEFINE_H
