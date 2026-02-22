#include "lexer.hpp"
#include <cstdint>
#include <cstring>

alignas(64)
static const uint8_t charClassDict[256] = {
    /* 0x00 - 0x0F */
    0,0,0,0,0,0,0,0,0,
    0,                // \t  0x09
    CC_NEWLINE,    // \n  0x0A
    0,0,
    CC_NEWLINE,    // \r  0x0D
    0,0,

    /* 0x10 - 0x1F */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    /* 0x20 - 0x2F */
    0,   // ' '
    TT_EXCL,// !
    0,          // "
    0,   // #
    0,          // $
    TT_PERCENT,// %
    TT_AMPERSAND,// &
    TT_QUOT,// '
    TT_LPAREN,   // (
    TT_RPAREN,   // )
    TT_STAR,// *
    TT_PLUS,// +
    TT_COMMA,   // ,
    TT_MINUS,// -
    TT_DOT,// .
    TT_SLASH,// /

    /* 0x30 - 0x39 '0'-'9' */
    CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,
    CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,

    /* 0x3A - 0x40 */
    TT_COLON,   // :
    TT_SEMICOLON,   // ;
    TT_LT,// <
    TT_EQUAL,// =
    TT_GT,// >
    TT_QUESTION,// ?
    TT_AT,// @

    /* 0x41 - 0x5A 'A'-'Z' */
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,

    /* 0x5B - 0x60 */
    TT_LBRACE,   // [
    CC_OPERATOR,// backslash
    TT_RBRACE,   // ]
    TT_CARET,// ^
    CC_UNDERSCORE,// _
    0,          // `

    /* 0x61 - 0x7A 'a'-'z' */
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,

    /* 0x7B - 0x7F */
    TT_LBRACKET,   // {
    TT_PIPE,// |
    TT_RBRACKET,   // }
    TT_TILDE,// ~
    0,          // DEL

    /* 0x80 - 0xFF (all unknown for ASCII lexer) */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

inline uint8_t info(char c) {
    return charClassDict[static_cast<uchar_t>(c)];
};

Lexer::Lexer(std::string src) : source(std::move(src)) {
    source.reserve(src.size() + 2);
    source.push_back('\0');
    source.push_back('\0');
    begin = source.data();
    current = begin;
    end = begin + source.size() - 2;
    indentStack.reserve(64);
    indentStack.push_back(0);

    size_t estimatedSymbols = source.size() >> 3;
    capacity = 1;
    while (capacity < estimatedSymbols) capacity <<= 1;
    if (capacity < 64) capacity = 64;

    threshold = capacity - (capacity >> 2);
    table = std::vector<Entry>(capacity);
};

void Lexer::insert(Entry entry) {
    size_t mask = capacity - 1;
    size_t index = entry.hash & mask;
    size_t distance = 0;

    while (true) {
        Entry& current = table[index];

        if (current.hash == 0) {
            table[index] = entry;
            ++size;
            return;
        }
        // Robin-hood logic
        size_t ideal = current.hash & mask;
        size_t currentDistance = (index - ideal) & mask;

        if (currentDistance < distance) {
            std::swap(current, entry);
            distance = currentDistance;
        }

        index = (index + 1) & mask;
        ++distance;
    }
}

symbol_t Lexer::intern(const char* string, uint32_t length) {
    if (size >= threshold) grow();

    // Hash
    uint64_t hash = 1469598103934665603ull;
    for (uint32_t i = 0; i < length; ++i) {
        hash ^= static_cast<uchar_t>(string[i]);
        hash *= 1099511628211ull;
    }
    if (!hash) hash = 1;

    size_t mask = capacity - 1;
    size_t index = hash & mask;
    size_t distance = 0;

    // Lookup first
    while (true) {
        Entry& current = table[index];

        if (current.hash == 0)
            break;

        if (current.hash == hash && current.length == length) {
            const char* stored =
                pool.data() + static_cast<size_t>(current.offset);

            if (memcmp(stored, string, length) == 0)
                return current.offset;
        }

        size_t ideal = current.hash & mask;
        size_t currentDistance = (index - ideal) & mask;

        if (currentDistance < distance) break; // Robin-hood invariant: not present

        index = (index + 1) & mask;
        ++distance;
    }

    // Not found: allocate string
    uint32_t offset = pool.size();
    pool.insert(pool.end(), string, string + length);
    pool.push_back('\0');

    insert(Entry {hash, offset, length});

    return offset;
}

void Lexer::grow() {
    std::vector<Entry> old = std::move(table);
    capacity <<= 1;
    threshold = capacity - (capacity >> 2);
    table.clear();
    table.resize(capacity);
    size = 0;

    for (auto& e : old) {
        if (e.hash != 0) {
            insert(e);
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(1024);
    while (current < end) {
        // Handle indentation
        if (atLineStart) {
            const char* indentStart = current;
            uint32_t indent = 0;
            atLineStart = false;
            uint32_t offset = static_cast<uint32_t>(indentStart - begin);
            
            while (*current == ' ') {
                ++current;
                ++indent;
            }
            
            if (info(*current) == CC_NEWLINE) {
                ++current;
                atLineStart = true;
                continue;
            }
            
            uint32_t previous = indentStack.back();

            if (indent > previous) {
                indentStack.push_back(indent);
                tokens.push_back(Token {0, offset, indent, TT_INDENT}); 
            }
            else if (indent < previous) {
                while (indentStack.size() > 0 && indent < indentStack.back()) {
                    indentStack.pop_back();
                    tokens.push_back(Token {0, offset, 0, TT_DEDENT});
                }
                // Handle indent mismatch
                if (indent != indentStack.back()) 
                    tokens.push_back(Token {0, offset, 0, TT_ERROR});
                
            }
        }
        // Skip space in the non-indent context
        while (*current == ' ') ++current;

        // Throw EOF
        if (current >= end) break;

        // Skip the comment
        if (*current == '#') {
            while (info(*current) != CC_NEWLINE) ++current;
            while (info(*current) == CC_NEWLINE) ++current;
            atLineStart = true;
            continue;
        }

        // Start lexing
        const char* lexemeStart = current;
        uchar_t c = *current;
        uint8_t cls = info(c);
        uint32_t offset = static_cast<uint32_t>(lexemeStart - begin);
        ++current;  // Optimize for repetition in branches

        // Check if newline
        if (cls == CC_NEWLINE) {
            // ++current;
            if (c == '\r' && *current == '\n') ++current;
            atLineStart = true;
            tokens.push_back(Token {0, offset, static_cast<uint32_t>(current - lexemeStart), TT_NEWLINE});
            continue;
        }

        // Check if identifier
        if (cls == CC_IDENT_START) {
            // ++current;
            while (info(*current) & CC_IDENT_CONT) ++current;
            uint32_t length = static_cast<uint32_t>(current - lexemeStart);
            symbol_t identifier = intern(lexemeStart, length);
            tokens.push_back(Token {identifier, offset, length, TT_IDENT});
            continue;
        }

        // Check if number
        if (cls == CC_DIGIT) {
            // ++current;
            while (info(*current) == CC_DIGIT) ++current;

            bool isFloat = *current == '.' && info(*(current + 1)) == CC_DIGIT;
            current += isFloat;
            if (isFloat) while (info(*current) == CC_DIGIT) ++current;

            uint32_t length = static_cast<uint32_t>(current - lexemeStart);
            symbol_t number = intern(lexemeStart, length);
            
            tokens.push_back(Token {number, offset, length, TT_NUMBER, isFloat ? F_FLOAT : F_INT});
            continue;
        }

        // Check if operator (max munch for custom ops later)
        if (cls & CC_OPERATOR) {
            // ++current;
            while (info(*current) & CC_OPERATOR) ++current;
            uint32_t length = static_cast<uint32_t>(current - lexemeStart);
            TokenType type;
            symbol_t op = 0;

            if (length == 1) 
                type = static_cast<TokenType>(info(c));
            else if (length == 2) {
                char c1 = lexemeStart[0];
                char c2 = lexemeStart[1];

                if (c2 == '=') 
                    type = static_cast<TokenType>(info(c1) + 19);
                else if (c1 == '>' && c2 == '>')
                    type = TT_RSHIFT;
                else if (c1 == '<' && c2 == '<')
                    type = TT_LSHIFT;
                else if (c1 == '-' && c2 == '>')
                    type = TT_RARROW;
                else if (c1 == '<' && c2 == '-')
                    type = TT_LARROW;
                else if (c1 == '&' && c2 == '&')
                    type = TT_AND;
                else if (c1 == '|' && c2 == '|')
                    type = TT_OR;
                else
                    type = TT_CUSTOM_OP;
            }
            else {
                op = intern(lexemeStart, length);
                type = TT_CUSTOM_OP;
            }
                
            tokens.push_back(Token {op, offset, length, type});
            continue;
        }

        // Check if punctuation
        // ++current;
        if (cls & CC_PUNCT) {
            TokenType type = static_cast<TokenType>(info(c));
            tokens.push_back(Token {0, offset, 1, type});
            continue;
        }

        // Group unknown characters
        if (!cls) {
            // ++current;
            while (info(*current) == CC_UNKNOWN) ++current;
            tokens.push_back(Token {0, offset, static_cast<uint32_t>(current - lexemeStart), TT_UNKNOWN});
            continue;
        }
    }
    // Handle EOF
    uint32_t offset = static_cast<uint32_t>(current - begin);
    while (indentStack.size() > 0) {
        indentStack.pop_back();
        tokens.push_back(Token {0, offset, 0, TT_DEDENT});
    }
    tokens.push_back(Token {0, offset, 0, TT_EOF});
    return tokens;
};
