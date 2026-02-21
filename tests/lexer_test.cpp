#include <cstddef>
#include <cstdio>
#include "lexer.hpp"

int main() {
    std::string src = "if lang = Spanish\n\n  print Hola, 1234.0\r\n else\n\r  print Hello, 1234";
    Lexer lexer = Lexer(src);
    std::vector<Token> tokens = lexer.tokenize();
    Token tok;
    printf("Size: %d\n", static_cast<int>(tokens.size()));
    for (size_t i = 0; i < tokens.size() - 3; ++i) { // 3 EOF?
        tok = tokens[i];
        printf("Type: %d, lexeme: \"%s\"\n", tok.type, src.substr(tok.offset, tok.length).c_str());
    }
    printf("Type: EOF, lexeme:\n");
    return 0;
}