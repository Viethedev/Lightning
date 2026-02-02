/* DECAPRATED CODEBASE
#include "frontend.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <stdexcept>

// Implementation for generating a shard from the source code
void Frontend::generateShard(const std::string &source) {
    sourceCode = source;
    stripBOMAndShebang();
}

// Implementation for stripping BOM and shebang from sourceCode
void Frontend::stripBOMAndShebang() {
    // Strip BOM if present
    if (decoder__startByteIndex == 0 && sourceCode.size() >= 3 &&
        static_cast<uint8_t>(sourceCode[0]) == 0xEF &&
        static_cast<uint8_t>(sourceCode[1]) == 0xBB &&
        static_cast<uint8_t>(sourceCode[2]) == 0xBF) {
        decoder__startByteIndex += 3;
    }
    // Strip shebang at currentByteIndex
    if (decoder__startByteIndex + 1 < sourceCode.size() &&
        sourceCode[decoder__startByteIndex] == '#' &&
        sourceCode[decoder__startByteIndex + 1] == '!') {

        while (decoder__startByteIndex < sourceCode.size() &&
               sourceCode[decoder__startByteIndex] != '\n') {
            decoder__startByteIndex++;
        }
        if (decoder__startByteIndex < sourceCode.size()) {
            decoder__startByteIndex++; // skip '\n'
            decoder__codepointIndex++; // increment codepoint index
            currentLine++;
            currentColumn = 0;
        }
    }
}

// Implementation for advancing to the next Unicode code point
void Frontend::advanceCodepoint() {  
    decoder__codepointIndex++;
    decoder__startByteIndex = decoder__nextByteIndex;
}

uint32_t Frontend::currentCodepoint() {
    // Check cache first
    if (decoder__codepointIndex >= cache__startCodepointIndex &&
        decoder__codepointIndex < cache__endCodepointIndex) {
        return loadCache(decoder__codepointIndex);
    }

    // End of input
    if (decoder__startByteIndex >= sourceCode.size()) {
        return 0;
    }

    const unsigned char c0 =
        static_cast<unsigned char>(sourceCode[decoder__startByteIndex]);

    // 1-byte (ASCII): 0xxxxxxx
    if (c0 <= 0x7F) {
        decoder__nextByteIndex = decoder__startByteIndex + 1;
        return c0;
    }

    // Helper to read continuation bytes safely
    auto read_cont = [&](size_t offset) -> unsigned char {
        if (decoder__startByteIndex + offset >= sourceCode.size())
            throw std::runtime_error("Invalid UTF-8 (truncated)");
        unsigned char cx =
            static_cast<unsigned char>(sourceCode[decoder__startByteIndex + offset]);
        if ((cx & 0xC0) != 0x80)
            throw std::runtime_error("Invalid UTF-8 (bad continuation)");
        return cx;
    };

    // 2-byte: 110xxxxx 10xxxxxx
    // Valid lead: C2–DF
    if (c0 >= 0xC2 && c0 <= 0xDF) {
        unsigned char c1 = read_cont(1);

        char32_t cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);

        // Overlong check (must be >= 0x80)
        if (cp < 0x80)
            throw std::runtime_error("Invalid UTF-8 (overlong)");

        decoder__nextByteIndex = decoder__startByteIndex + 2;
        return cp;
    }

    // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
    // Valid lead: E0–EF
    if (c0 >= 0xE0 && c0 <= 0xEF) {
        unsigned char c1 = read_cont(1);
        unsigned char c2 = read_cont(2);

        // Special restrictions to prevent overlongs & surrogates
        if (c0 == 0xE0 && c1 < 0xA0)
            throw std::runtime_error("Invalid UTF-8 (overlong 3-byte)");
        if (c0 == 0xED && c1 >= 0xA0)
            throw std::runtime_error("Invalid UTF-8 (surrogate)");

        char32_t cp =
            ((c0 & 0x0F) << 12) |
            ((c1 & 0x3F) << 6) |
            (c2 & 0x3F);

        // General overlong check
        if (cp < 0x800)
            throw std::runtime_error("Invalid UTF-8 (overlong)");

        // Surrogate range check
        if (cp >= 0xD800 && cp <= 0xDFFF)
            throw std::runtime_error("Invalid UTF-8 (surrogate)");

        decoder__nextByteIndex = decoder__startByteIndex + 3;
        return cp;
    }

    // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    // Valid lead: F0–F4
    if (c0 >= 0xF0 && c0 <= 0xF4) {
        unsigned char c1 = read_cont(1);
        unsigned char c2 = read_cont(2);
        unsigned char c3 = read_cont(3);

        // Special restrictions
        if (c0 == 0xF0 && c1 < 0x90)
            throw std::runtime_error("Invalid UTF-8 (overlong 4-byte)");
        if (c0 == 0xF4 && c1 > 0x8F)
            throw std::runtime_error("Invalid UTF-8 (> U+10FFFF)");

        char32_t cp =
            ((c0 & 0x07) << 18) |
            ((c1 & 0x3F) << 12) |
            ((c2 & 0x3F) << 6) |
            (c3 & 0x3F);

        // Final range check
        if (cp < 0x10000 || cp > 0x10FFFF)
            throw std::runtime_error("Invalid UTF-8 (range)");

        decoder__nextByteIndex = decoder__startByteIndex + 4;
        return cp;
    }

    // Everything else is invalid (C0, C1, F5–FF, stray continuation, etc.)
    throw std::runtime_error("Invalid UTF-8 leading byte");
}

// Implementation for peeking the next Unicode code point
uint32_t Frontend::peekCodepoint(std::size_t ahead) {
    
    size_t targetCodepointIndex = decoder__codepointIndex + ahead;
    
    // Check cache first
    if (targetCodepointIndex < cache__endCodepointIndex && 
        targetCodepointIndex > cache__startCodepointIndex) {
        return loadCache(targetCodepointIndex);
    }
    
    // Save current decoder state
    size_t savedByteIndex = decoder__startByteIndex;
    size_t savedCodepointIndex = decoder__codepointIndex;
    size_t savedNextByteIndex = decoder__nextByteIndex;

    
    // Cache the nearest codepoints from current position
    uint32_t cp;
    uint8_t cachedTimes = static_cast<uint8_t>(std::min(ahead, static_cast<std::size_t>(31)));
    
    resetCache();
    for (uint8_t i = 0; i < cachedTimes; ++i) {
        advanceCodepoint();
        cp = currentCodepoint();
        cache(decoder__codepointIndex, cp);
    }
    stopCache();
    
    while (decoder__codepointIndex < targetCodepointIndex) {
        advanceCodepoint();
        cp = currentCodepoint();
    }
    decoder__startByteIndex = savedByteIndex;
    decoder__nextByteIndex = savedNextByteIndex;
    decoder__codepointIndex = savedCodepointIndex;
    return cp;
}

Token Frontend::currentToken() {
    // Implementation for getting the current token
}

Token Frontend::peekNextToken() {
    // Implementation for peeking the next token
}

void Frontend::advanceToNextToken() {
    // Implementation for advancing to the next token
}

void Frontend::matchToken() {
    // Implementation for matching a token
}

ASTNode Frontend::currentNode() {
    // Implementation for getting the current AST node
}

ASTNode Frontend::peekNextNode() {
    // Implementation for peeking the next AST node
}

void Frontend::advanceToNextNode() {
    // Implementation for advancing to the next AST node
}

void Frontend::skipWhitespaceAndComments() {
    // Implementation for skipping whitespace and comments
}

#include <iostream>

int main() {
    // Example usage of Frontend
    Frontend frontend;
    std::string sourceCode = "#!/usr/bin/env lightning\nprint('Hello, World!')\na = 42\nb = a @ 2";
    frontend.generateShard(sourceCode);
    uint32_t cp;
    uint32_t peek;
    for (int i = 0; i < 40; ++i) { // Limit iterations to avoid infinite loop
        cp = frontend.currentCodepoint();
        peek = frontend.peekCodepoint(3);
        std::cout << cp << " (peek: " << peek << ")" << std::endl;
        frontend.advanceCodepoint();
    }
    return 0;
};
*/