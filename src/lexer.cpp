// Implementation for the modernized Lexer
#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

namespace {
    using Map = std::unordered_map<std::string, TokenType>;

    Map make_keyword_map() {
        return Map{
            {"if", TokenType::If},
            {"else", TokenType::Else},
            {"for", TokenType::For},
            {"while", TokenType::While},
            {"return", TokenType::Return},
            {"function", TokenType::Function},
        };
    }

    const Map& keywords() {
        static Map m = make_keyword_map();
        return m;
    }
}

Lexer::Lexer(std::string source) noexcept
    : source_(std::move(source)), pos_(0), length_(source_.size()), line_(1), column_(1) {}

char Lexer::currentChar() const noexcept {
    return pos_ < length_ ? source_[pos_] : '\0';
}

char Lexer::peekChar(std::size_t ahead) const noexcept {
    auto idx = pos_ + ahead;
    return idx < length_ ? source_[idx] : '\0';
}

void Lexer::advance() noexcept {
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

void Lexer::skipWhitespace() noexcept {
    for (;;) {
        char c = currentChar();
        if (c == '\0') return;
        if (std::isspace(static_cast<unsigned char>(c))) {
            advance();
            continue;
        }
        if (c == '/') {
            char p = peekChar();
            if (p == '/' || p == '*') {
                skipComment();
                continue;
            }
        }
        break;
    }
}

void Lexer::skipComment() noexcept {
    if (currentChar() == '/' && peekChar() == '/') {
        // line comment
        while (currentChar() != '\0' && currentChar() != '\n') advance();
        if (currentChar() == '\n') advance();
        return;
    }
    if (currentChar() == '/' && peekChar() == '*') {
        // block comment
        advance(); // /
        advance(); // *
        while (currentChar() != '\0') {
            if (currentChar() == '*' && peekChar() == '/') {
                advance(); // *
                advance(); // /
                break;
            }
            advance();
        }
    }
}

Token Lexer::readNumber() {
    Token tok;
    tok.type = TokenType::Number;
    tok.index = pos_;
    std::size_t start = pos_;

    while (std::isdigit(static_cast<unsigned char>(currentChar()))) advance();
    if (currentChar() == '.' && std::isdigit(static_cast<unsigned char>(peekChar()))) {
        advance(); // dot
        while (std::isdigit(static_cast<unsigned char>(currentChar()))) advance();
    }

    tok.value = source_.substr(start, pos_ - start);
    tok.line = line_;
    tok.column = column_ - static_cast<std::uint32_t>(tok.value.size());
    return tok;
}

Token Lexer::readString() {
    Token tok;
    tok.type = TokenType::String;
    tok.index = pos_;
    char quote = currentChar();
    advance(); // skip opening quote
    std::size_t start = pos_;
    std::string out;

    while (currentChar() != '\0' && currentChar() != quote) {
        if (currentChar() == '\\') {
            advance();
            char esc = currentChar();
            switch (esc) {
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case 'r': out.push_back('\r'); break;
                case '\\': out.push_back('\\'); break;
                case '\'': out.push_back('\''); break;
                case '"': out.push_back('"'); break;
                default: out.push_back(esc); break;
            }
            if (currentChar() != '\0') advance();
            continue;
        }
        out.push_back(currentChar());
        advance();
    }
    if (currentChar() == quote) advance(); // consume closing quote

    tok.value = std::move(out);
    tok.line = line_;
    tok.column = column_ - static_cast<std::uint32_t>(tok.value.size()) - 1;
    return tok;
}

Token Lexer::readIdentifier() {
    Token tok;
    tok.index = pos_;
    std::size_t start = pos_;
    while (std::isalnum(static_cast<unsigned char>(currentChar())) || currentChar() == '_') advance();
    tok.value = source_.substr(start, pos_ - start);
    tok.line = line_;
    tok.column = column_ - static_cast<std::uint32_t>(tok.value.size());

    auto it = keywords().find(tok.value);
    if (it != keywords().end()) tok.type = it->second;
    else tok.type = TokenType::Identifier;
    return tok;
}

Token Lexer::nextToken() {
    skipWhitespace();
    Token tok;
    tok.index = pos_;
    tok.line = line_;
    tok.column = column_;

    char c = currentChar();
    if (c == '\0') {
        tok.type = TokenType::EndOfFile;
        tok.value.clear();
        return tok;
    }

    // Numbers
    if (std::isdigit(static_cast<unsigned char>(c))) return readNumber();

    // Strings
    if (c == '"' || c == '\'') return readString();

    // Identifiers / keywords
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') return readIdentifier();

    // Two-character operators
    switch (c) {
        case '=':
            if (peekChar() == '=') { advance(); advance(); tok.type = TokenType::Equal; tok.value = "=="; return tok; }
            advance(); tok.type = TokenType::Assign; tok.value = "="; return tok;
        case '!':
            if (peekChar() == '=') { advance(); advance(); tok.type = TokenType::NotEqual; tok.value = "!="; return tok; }
            advance(); tok.type = TokenType::Not; tok.value = "!"; return tok;
        case '<':
            if (peekChar() == '=') { advance(); advance(); tok.type = TokenType::LessEqual; tok.value = "<="; return tok; }
            advance(); tok.type = TokenType::Less; tok.value = "<"; return tok;
        case '>':
            if (peekChar() == '=') { advance(); advance(); tok.type = TokenType::GreaterEqual; tok.value = ">="; return tok; }
            advance(); tok.type = TokenType::Greater; tok.value = ">"; return tok;
        case '&':
            if (peekChar() == '&') { advance(); advance(); tok.type = TokenType::And; tok.value = "&&"; return tok; }
            break;
        case '|':
            if (peekChar() == '|') { advance(); advance(); tok.type = TokenType::Or; tok.value = "||"; return tok; }
            break;
    }

    // Single-character tokens
    switch (c) {
        case '+': advance(); tok.type = TokenType::Plus; tok.value = "+"; return tok;
        case '-': advance(); tok.type = TokenType::Minus; tok.value = "-"; return tok;
        case '*': advance(); tok.type = TokenType::Star; tok.value = "*"; return tok;
        case '/': advance(); tok.type = TokenType::Slash; tok.value = "/"; return tok;
        case '%': advance(); tok.type = TokenType::Percent; tok.value = "%"; return tok;
        case '(' : advance(); tok.type = TokenType::LParen; tok.value = "("; return tok;
        case ')' : advance(); tok.type = TokenType::RParen; tok.value = ")"; return tok;
        case '{' : advance(); tok.type = TokenType::LBrace; tok.value = "{"; return tok;
        case '}' : advance(); tok.type = TokenType::RBrace; tok.value = "}"; return tok;
        case '[' : advance(); tok.type = TokenType::LBracket; tok.value = "["; return tok;
        case ']' : advance(); tok.type = TokenType::RBracket; tok.value = "]"; return tok;
        case ';' : advance(); tok.type = TokenType::Semicolon; tok.value = ";"; return tok;
        case ',' : advance(); tok.type = TokenType::Comma; tok.value = ","; return tok;
        case '.' : advance(); tok.type = TokenType::Dot; tok.value = "."; return tok;
        case ':' : advance(); tok.type = TokenType::Colon; tok.value = ":"; return tok;
    }

    // Unknown single char
    advance();
    tok.type = TokenType::Unknown;
    tok.value = std::string(1, c);
    return tok;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> out;
    for (;;) {
        Token t = nextToken();
        out.push_back(std::move(t));
        if (out.back().type == TokenType::EndOfFile) break;
    }
    return out;
}