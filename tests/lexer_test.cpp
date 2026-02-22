#include <cstddef>
#include <cstdio>
#include "lexer.hpp"

int main() {
    std::string src = "if lang = Spanish\n\n  print Hola, 1234.0\r\n else\n\r  print Hello, 1234";
    Lexer lexer = Lexer(src);
    std::vector<Token> tokens = lexer.tokenize();
    Token tok;
    printf("Size of token vector: %lli\n", tokens.size());
    for (size_t i = 0; i < tokens.size(); ++i) {
        tok = tokens[i];
        printf("Type: %d, lexeme: %.*s\n", tok.type, tok.length, src.c_str() + tok.offset);
    }
    return 0;
}