#include <cstdio>
#include "lexer.hpp"

int main() {
    std::string src = "if lang = Spanish\n\n  print Hola, 1234.0\r\n else\n\r  print Hello, 1234";
    Lexer lexer = Lexer(src);
    std::vector<Token> tokens = lexer.tokenize();
    for (Token tok : tokens) {
        printf("Type: %d, lexeme: \"%s\"\n", tok.type, src.substr(tok.offset, tok.length).c_str());
    }
    // Will raise: 
    // terminate called after throwing an instance of 'std::out_of_range'
    //   what():  basic_string::substr: __pos (which is 69) > this->size() (which is 67)
    // because try to get EOF substring in source
    return 0;
}