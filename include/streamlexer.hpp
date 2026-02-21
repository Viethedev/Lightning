/* DECAPRATED
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

typedef unsigned char uchar_t;

typedef enum : uint8_t {
    CC_UNKNOWN    = 0,
    CC_ALPHA      = 1,
    CC_DIGIT      = 2,
    CC_OPERATOR   = 4,
    CC_PUNCT      = 8,
    CC_SPACE      = 16,
    CC_NEWLINE    = 32,
    CC_UNDERSCORE = 64,
    CC_ANNOT      = 128,
    // Composition character classes
    CC_IDENT_START = 65,
    CC_IDENT_CONT = 67,
} CharClass;

typedef enum : uint8_t {
    TT_UNKNOWN, TT_ERROR,
    TT_IDENT, TT_KEYWORD,
    TT_INT, TT_FLOAT, TT_COMPLEX, TT_STRING,
    TT_OPERATOR,
    TT_PUNCT,
    TT_INDENT, TT_DEDENT, TT_NEWLINE, TT_EOF
} TokenType;

typedef struct {
    TokenType type;
    size_t index;
    size_t length;
} Token;

class Lexer {
public:
    Lexer(const std::string& sourceCode);
    Token nextToken();
    const std::string& source;
private:
    // Source memory
    const char* begin;
    const char* current;
    const char* end;

    // Indentation memory
    uint32_t indentStack[32] = {0};
    uint32_t stackTop = 0;

    uint32_t stackSize = 32;
    uint32_t pendingDedents = 0;
    bool atLineStart = true;

    inline bool isInstance(char c, CharClass cc);
};

*/