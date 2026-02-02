#pragma once
#include <cstdint>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"

class Frontend {
    public:
    Frontend() = default;
    ~Frontend() = default;
    void generateShard(const std::string &source);

    private:
    std::string sourceCode;
    uint32_t line = 0;
    uint32_t column = 0;
    uint32_t index = 0;
};