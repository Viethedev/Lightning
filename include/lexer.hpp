#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

enum class TokenType : std::uint8_t {
    // Literals
    Number, String, Identifier,

    // Keywords
    If, Else, For, While, Return, Function,

    // Operators
    Plus, Minus, Star, Slash, Percent,
    Assign,
    Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
    And, Or, Not,

    // Delimiters
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Semicolon, Comma, Dot, Colon,

    // Special
    EndOfFile, Unknown
};

struct Token {
    TokenType type = TokenType::Unknown;
    std::string_view value;  // Zero-copy, UTF-8 friendly
    std::size_t index = 0;
    std::uint32_t line = 1;
    std::uint32_t column = 1;
};

class Lexer {
public:
    explicit Lexer(std::string source) noexcept;

    std::vector<Token> tokenize();
    Token nextToken();

private:
    std::string source_;  // UTF-8 string
    std::size_t pos_ = 0;
    std::size_t length_ = 0;
    std::uint32_t line_ = 1;
    std::uint32_t column_ = 1;

    // Inline for speed
    inline char currentChar() const noexcept {
        return pos_ < length_ ? source_[pos_] : '\0';
    }
    inline char peekChar(std::size_t ahead = 1) const noexcept {
        auto idx = pos_ + ahead;
        return idx < length_ ? source_[idx] : '\0';
    }
    inline void advance() noexcept {
        if (pos_ < length_) {
            if (source_[pos_] == '\n') {
                ++line_;
                column_ = 1;
            } else {
                ++column_;
            }
            ++pos_;
        }
    }

    void skipWhitespace() noexcept;
    void skipComment() noexcept;

    Token readNumber();
    Token readString();
    Token readIdentifier();
};