#include "lcc.hpp"
#include <cctype>

Token Frontend::currentToken() {
    if ('A' < currentChar() && currentChar() < 'Z') {
        return EOFToken;
    }
}