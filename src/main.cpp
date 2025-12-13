#include <iostream>
#include <string>
#include "parser.hpp"

int main() {
    std::string input = R"(
if x > 0 :
    print ( positive )
    if x > 10 :
        print ( big )
    else :
        print ( small )
else :
    print ( non-positive )
print ( done )
)";

    Parser p;
    p.parse(input);
    return 0;
}