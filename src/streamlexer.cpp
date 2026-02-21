/* DECAPRATED
#include "streamlexer.hpp"
#include <cstdint>
#include <cstdio>

alignas(64)
static const uint8_t charClassDict[256] = {
    // 0x00 - 0x0F 
    0,0,0,0,0,0,0,0,0,
    CC_SPACE,                // \t  0x09
    CC_NEWLINE|CC_SPACE,    // \n  0x0A
    0,0,
    CC_NEWLINE|CC_SPACE,    // \r  0x0D
    0,0,

    // 0x10 - 0x1F
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    // 0x20 - 0x2F 
    CC_SPACE,   // ' '
    CC_OPERATOR,// !
    0,          // "
    CC_ANNOT,   // #
    0,0,0,      // $ % &
    CC_OPERATOR,// '
    CC_PUNCT,   // (
    CC_PUNCT,   // )
    CC_OPERATOR,// *
    CC_OPERATOR,// +
    CC_PUNCT,   // ,
    CC_OPERATOR,// -
    CC_OPERATOR,// .
    CC_OPERATOR,// /

    // 0x30 - 0x39 '0'-'9'
    CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,
    CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,CC_DIGIT,

    // 0x3A - 0x40 
    CC_PUNCT,   // :
    CC_PUNCT,   // ;
    CC_OPERATOR,// <
    CC_OPERATOR,// =
    CC_OPERATOR,// >
    CC_OPERATOR,// ?
    0,          // @

    // 0x41 - 0x5A 'A'-'Z' 
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,

    // 0x5B - 0x60 
    CC_PUNCT,   // [
    CC_OPERATOR,// \
    CC_PUNCT,   // ]
    CC_OPERATOR,// ^
    CC_UNDERSCORE,// _
    0,          // `

    // 0x61 - 0x7A 'a'-'z' 
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,
    CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,CC_ALPHA,

    // 0x7B - 0x7F 
    CC_PUNCT,   // {
    CC_OPERATOR,// |
    CC_PUNCT,   // }
    CC_OPERATOR,// ~
    0,          // DEL

    // 0x80 - 0xFF (all unknown for ASCII lexer) 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

inline bool Lexer::isInstance(char c, CharClass cc) {
    return charClassDict[static_cast<uchar_t>(c)] & cc;
};

Lexer::Lexer(const std::string& src) : source(src) {
    begin = source.data();
    current = begin;
    end = begin + source.size();
};

Token Lexer::nextToken() {
    while (true) {
        // Handle EOF
        if (current >= end) {
            if (stackTop > 0) {
                --stackTop;
                atLineStart = true;
                return Token {TT_DEDENT, static_cast<size_t>(current - begin), 0};
            }
            return Token {TT_EOF, static_cast<size_t>(current - begin), 0};
        }
        // Handle indentation
        if (atLineStart) {
            // Handle pending dedents
            if (pendingDedents > 0) {
                --pendingDedents;
                return Token {TT_DEDENT, static_cast<size_t>(current - begin), 0};
            }

            const char* indentStart = current;
            uint32_t indent = 0;
            atLineStart = false;
            
            while (current < end && *current == ' ') {
                ++current;
                ++indent;
            }
            
            if (isInstance(*current, CC_NEWLINE)) {
                ++current;
                atLineStart = true;
                continue;
            }

            if (current < end && !isInstance(*current, CC_NEWLINE)) {
                uint32_t previous = indentStack[stackTop];

                if (indent > previous) {
                    if (stackTop >= stackSize) {
                        printf("MemoryError: Indent stack overflow - initial design limitation\n(quick fix: nest your code less)\n");
                        return Token {TT_ERROR, static_cast<size_t>(indentStart - begin), static_cast<size_t>(indent)};
                    }

                    indentStack[++stackTop] = indent;
                
                    return Token {TT_INDENT, static_cast<size_t>(indentStart - begin), static_cast<size_t>(indent)};
                }

                if (indent < previous) {
                    while (stackTop > 0 && indent < indentStack[stackTop]) {
                        --stackTop;
                        ++pendingDedents;
                    }
                    if (indent != indentStack[stackTop]) {
                        printf("DedentationError: Indent mismatches dedent\n(quick fix: check indentation level or check if indent stack overflow happens)\n");
                        return Token {TT_ERROR, static_cast<size_t>(indentStart - begin), static_cast<size_t>(indent)};
                    }
                }
            }
        }
        // Skip space in the non-indent context
        while (current < end && *current == ' ') ++current;

        // Throw EOF
        if (current >= end) return Token {TT_EOF, static_cast<size_t>(current - begin), 0};

        // Start lexing
        const char* lexemeStart = current;
        uchar_t c = static_cast<uchar_t>(*current);
        uint8_t cls = charClassDict[c];

        // Skip the comment
        if (*current == '#') {
            while (current < end && !isInstance(*current, CC_NEWLINE)) ++current;
            while (current < end && isInstance(*current, CC_NEWLINE)) ++current;
            atLineStart = true;
            continue;
        }

        // Check if newline
        if (cls & CC_NEWLINE) {
            ++current;
            if (c == '\r' && current < end && *current == '\n') ++current;
            atLineStart = true;
            return Token {TT_NEWLINE, static_cast<size_t>(lexemeStart - begin), static_cast<size_t>(current - lexemeStart)};
        }

        // Check if identifier
        if (cls & CC_IDENT_START) {
            ++current;
            while (current < end && isInstance(*current,  CC_IDENT_CONT)) ++current;
            return Token {TT_IDENT, static_cast<size_t>(lexemeStart - begin), static_cast<size_t>(current - lexemeStart)};
        }

        // Check if number
        if (cls & CC_DIGIT) {
            ++current;
            while (current < end && isInstance(*current, CC_DIGIT)) ++current;
            bool isFloat = current < end && *current == '.' && current + 1 < end && isInstance(*(current + 1), CC_DIGIT);
            current += isFloat;
            if (isFloat) while (current < end && isInstance(*current, CC_DIGIT)) ++current;
            return Token {isFloat ? TT_FLOAT : TT_INT,
             static_cast<size_t>(lexemeStart - begin), static_cast<size_t>(current - lexemeStart)};
        }

        // Check if operator
        if (cls & CC_OPERATOR) {
            ++current;
            while (current < end && isInstance(*current, CC_OPERATOR)) ++current;
            return Token {TT_OPERATOR, static_cast<size_t>(lexemeStart - begin), static_cast<size_t>(current - lexemeStart)};
        }

        // Check if punctuation
        if (cls & CC_PUNCT) {
            ++current;
            return Token {TT_PUNCT, static_cast<size_t>(lexemeStart - begin), 1};
        }

        // Group unknown characters
        if (!cls) {
            ++current;
            while (current < end && charClassDict[static_cast<uchar_t>(*current)] == CC_UNKNOWN) ++current;
            return Token {TT_UNKNOWN, static_cast<size_t>(lexemeStart - begin), static_cast<size_t>(current - lexemeStart)};
        }
    }
};

*/