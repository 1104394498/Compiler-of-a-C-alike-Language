#pragma once

#ifndef INC_3___Error_TYPEDEFINE_H
#define INC_3___Error_TYPEDEFINE_H

/***** Lexical Analysis *****/
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


#endif //INC_3___Error_TYPEDEFINE_H
