#pragma once
#include <cstdint>
#include <string>
#include <vector>

typedef unsigned char uchar_t;

typedef uint32_t symbol_t;

enum CharClass : uint8_t {
    CC_UNKNOWN    = 0,
    CC_NEWLINE    = 2,
    CC_PUNCT      = 32,
    CC_OPERATOR   = 64,
    CC_DIGIT      = 128,
    CC_IDENT_CONT = 128,
    CC_IDENT_START = 129,
    CC_ALPHA      = 129,
    CC_UNDERSCORE = 129,
};

enum TokenType : uint16_t {
    TT_UNKNOWN, TT_ERROR,
    TT_IDENT,
    TT_NUMBER, TT_STRING,

    TT_LPAREN = 32, TT_RPAREN, 
    TT_COMMA, TT_COLON, TT_SEMICOLON,
    TT_LBRACE, TT_RBRACE, 
    TT_LBRACKET, TT_RBRACKET, 

    TT_CUSTOM_OP = 64,

    // Single op
    TT_EXCL = 65,
    TT_PERCENT,
    TT_AMPERSAND,
    TT_STAR,
    TT_PLUS,
    TT_MINUS,
    TT_SLASH,
    TT_LT,  
    TT_EQUAL,    
    TT_GT,
    TT_AT,
    TT_CARET, 
    TT_PIPE = 77, 
    TT_TILDE = 78,
    TT_QUOT,
    TT_QUESTION,
    TT_DOT,

    TT_AND = 82, 
    TT_OR = 83,

    // Double op (single + 19)
    TT_NEQ = 84, 
    TT_IMOD,
    TT_IAND, 
    TT_IMUL, 
    TT_IADD, 
    TT_ISUB, 
    TT_IDIV, 
    TT_LE, 
    TT_EQ,
    TT_GE, 
    TT_IAT,
    TT_IXOR,
    TT_IOR = 96, 
    
    TT_LSHIFT, TT_RSHIFT, TT_LARROW, TT_RARROW,

    TT_INDENT, TT_DEDENT, TT_NEWLINE, TT_EOF
};

enum Format : uint16_t {
    F_NONE,
    F_INT, F_FLOAT, F_COMPLEX,
};

struct Token {
    symbol_t lexeme;
    uint32_t offset;
    uint32_t length;
    TokenType type;
    Format format = F_NONE;
}; // 16 bytes

struct Entry {
    uint64_t hash = 0;  // 0 = empty
    uint32_t offset;
    uint32_t length;
}; // 16 bytes

class Lexer {
public:
    Lexer(std::string src);
    std::vector<Token> tokenize();
    std::vector<char> pool;
private:
    // Source memory
    std::string source;
    const char* begin;
    const char* current;
    const char* end;

    // Indentation memory
    std::vector<uint16_t> indentStack;
    bool atLineStart = true;

    // Intern table memory and logic
    std::vector<Entry> table;
    uint32_t capacity;  // Power of two required
    uint32_t size = 0;
    uint32_t threshold;

    symbol_t intern(const char* string, uint32_t length);
    void grow();
    void insert(Entry entry);
}; // 128 bytes