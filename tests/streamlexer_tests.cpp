/* DECAPRATED
#include <cstdio>
#include "streamlexer.hpp"

int main() {
    std::string src = "if lang = Spanish\n\n  print Hola#, 1234.0\r\n else\n\r  print Hello, 1234";
    Lexer lexer = Lexer(src);
    for (Token tok = lexer.nextToken(); tok.type != TT_EOF; tok = lexer.nextToken()) {
        printf("Type: %d, lexeme: \"%s\"\n", tok.type, src.substr(tok.index, tok.length).c_str());
    }
    return 0;
}

*/