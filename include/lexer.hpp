#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class TokenType : std::uint8_t {
    // Literals
    Number,
    String,
    Identifier,

    // Keywords
    If,
    Else,
    For,
    While,
    Return,
    Function,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Assign,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    And,
    Or,
    Not,

    // Delimiters
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Semicolon,
    Comma,
    Dot,
    Colon,

    // Special
    EndOfFile,
    Unknown
};

struct Token {
    TokenType type = TokenType::Unknown;
    std::string value; // owns the lexeme
    std::size_t index = 0; // byte index in source
    std::uint32_t line = 1;
    std::uint32_t column = 1;
};

class Lexer {
public:
    explicit Lexer(std::string source) noexcept;

    // Tokenize whole input and return tokens (including EOF token)
    std::vector<Token> tokenize();

    // Return the next token (advances internal position)
    Token nextToken();

private:
    std::string source_;
    std::size_t pos_ = 0;
    std::size_t length_ = 0;
    std::uint32_t line_ = 1;
    std::uint32_t column_ = 1;

    char currentChar() const noexcept;
    char peekChar(std::size_t ahead = 1) const noexcept;
    void advance() noexcept;
    void skipWhitespace() noexcept;
    void skipComment() noexcept;

    Token readNumber();
    Token readString();
    Token readIdentifier();
};