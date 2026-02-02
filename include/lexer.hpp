#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

typedef enum : uint8_t {
    Unrecognized, UnicodeUTF8,
    Identifier, Keyword,
    Integer, Float, Complex, String,
    Operator, Punctuation,
    Indent, Dedent, Newline, EOFToken
} TokenType;

#define TAB_SIZE 4

inline unsigned char currentChar(const std::string& sourceCode, std::size_t index) { 
    return static_cast<unsigned char>(sourceCode[index]); 
};

inline unsigned char peekChar(const std::string& sourceCode, std::size_t index, std::size_t ahead = 1) { 
    return static_cast<unsigned char>(sourceCode[index + ahead]); 
};

inline unsigned char nextChar(const std::string& sourceCode, std::size_t& index) { 
    return static_cast<unsigned char>(sourceCode[index++]); 
};

inline bool isAlpha(unsigned char c) { 
    unsigned char lowerC = c | 0x20;
    return (lowerC >= 'a' && lowerC <= 'z');
};

inline bool isDigit(unsigned char c) { 
    return c - '0' <= 9; 
};

inline bool isIdentifierChar(unsigned char c) { 
    return isAlpha(c) || isDigit(c) || c == '_'; 
};

inline bool isOperatorChar(unsigned char c) { 
    switch (c) {
        case '+': case '-': case '*': case '/': case '%':
        case '=': case '!': case '<': case '>': case '?':
        case '&': case '|': case '^': case '~':
            return true;
        default:
            return false;
    }
};

inline bool isPunctuationChar(unsigned char c) { 
    switch (c) {
        case '(': case ')': case '{': case '}':
        case '[': case ']': case ',': case ';': case ':':
            return true;
        default:
            return false;
    }
};

struct ustring_view {
    const char* data;
    uint32_t size;
    ustring_view(const char* d, std::size_t s) : data(d), size(s) {}
};

struct Token {
    TokenType type;
    ustring_view lexeme;
    uint32_t line;
    uint32_t column;
    size_t index;
};

inline bool isIdentifierStartChar(unsigned char c) {
    return isAlpha(c) || c == '_';
}

// Special ASCIIs: '\0': 0, '@': 64, '#': 35, '$': 36
// Undefined ASCIIs: 0-31, 96, 127 except for '\n': 13, '\r': 10, '\t': 9

inline Token currentToken(const std::string& sourceCode, std::size_t& index, uint32_t& line, uint32_t& column);