#include "lexer.hpp"

typedef unsigned char uchar;

inline Token currentToken(const std::string &sourceCode, std::size_t &index, uint32_t &line, uint32_t &column) {
    // Skip whitespace
    uint8_t pureLF = true;
    uchar c = currentChar(sourceCode, index);
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        if (c == '\r') { pureLF = false; line++; column = 1; }
        else if (c == '\n') {
            line += pureLF;
            column *= pureLF; // Reset column on LF
            pureLF = true;
        } 
        else if (c == '\t') column += TAB_SIZE;
        index++;
        c = currentChar(sourceCode, index);
    }

    size_t startIndex = index;
    uint32_t startLine = line;
    uint32_t startColumn = column;

    // Identify token type
    if (isIdentifierStartChar(c)) {
        for (uchar ch; !isIdentifierChar(ch); nextChar(sourceCode, index)) column++;
        return Token{TokenType::Identifier, ustring_view(&sourceCode[startIndex], index - startIndex), startLine, startColumn, startIndex};
    } 
    else if (isDigit(c)) {
        for (uchar ch; isDigit(ch); ch = nextChar(sourceCode, index)) column++;
        if (currentChar(sourceCode, index) == '.') {
            column++;
            for (uchar ch; isDigit(ch); ch = nextChar(sourceCode, index)) column++;
            if ((currentChar(sourceCode, index) | 0x20) == 'j') {
                column++;
                return Token{TokenType::Complex, ustring_view(&sourceCode[startIndex], index - startIndex), startLine, startColumn, startIndex};
            }
            else if ((currentChar(sourceCode, index) | 0x20) == 'e') {
                column++;
                if (currentChar(sourceCode, index) == '+' || currentChar(sourceCode, index) == '-') {
                    column++;
                    for (uchar ch; isDigit(ch); ch = nextChar(sourceCode, index)) column++;
                }
            }
            return Token{TokenType::Float, ustring_view(&sourceCode[startIndex], index - startIndex), startLine, startColumn, startIndex};
        
        return Token{TokenType::Integer, ustring_view(&sourceCode[startIndex], index - startIndex), startLine, startColumn, startIndex};
        }
    }
    else if (isOperatorChar(c)) {
        for (uchar ch; isOperatorChar(ch); ch = nextChar(sourceCode, index)) column++;
        return Token{TokenType::Operator, ustring_view(&sourceCode[startIndex], index - startIndex), startLine, startColumn, startIndex};
    } 
    else if (isPunctuationChar(c)) return Token{TokenType::Punctuation, ustring_view(&sourceCode[startIndex], 1), startLine, startColumn, startIndex};

    else if (c > 127) {
        // Handle UTF-8 characters (not fully implemented)
        while (nextChar(sourceCode, index) > 127) column++;
        return Token{TokenType::UnicodeUTF8, ustring_view(&sourceCode[startIndex], index - startIndex), startLine, startColumn, startIndex};
    }

    return Token{TokenType::Unrecognized, ustring_view(&sourceCode[startIndex], 1), startLine, startColumn, startIndex};
}
