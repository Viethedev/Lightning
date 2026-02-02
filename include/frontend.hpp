#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct Token {};

struct ASTNode {};

class Frontend {
    public:
    Frontend() = default;
    ~Frontend() = default;
    void generateShard(const std::string &source);
    uint32_t currentCodepoint();
    void advanceCodepoint();
    uint32_t peekCodepoint(std::size_t ahead = 1);

    private:
    std::string sourceCode;

    // Decoder only

    // Decoder cache - 31 codepoints ahead
    uint32_t codepointCache[32];
    size_t cache__startByteIndices[32];
    size_t cache__startCodepointIndex = 0;
    size_t cache__endCodepointIndex = 0;

    // Decoder state
    size_t decoder__startByteIndex = 0;
    size_t decoder__nextByteIndex = 0;
    size_t decoder__codepointIndex = 0;

    uint32_t inline loadCache(size_t codepointIndex) {
        size_t cacheIndex = codepointIndex & 31;
        decoder__startByteIndex = cache__startByteIndices[cacheIndex];
        decoder__nextByteIndex = cache__startByteIndices[(cacheIndex + 1) & 31];
        return codepointCache[cacheIndex];
    }

    void inline cache(size_t codepointIndex, uint32_t codepoint) {
        size_t cacheIndex = codepointIndex & 31;
        codepointCache[cacheIndex] = codepoint;
        cache__startByteIndices[cacheIndex] = decoder__startByteIndex;
        cache__startByteIndices[(cacheIndex + 1) & 31] = decoder__nextByteIndex;
    }

    void inline resetCache() {
        // The start codepoint is used for circular end handling
        size_t cacheIndex = decoder__codepointIndex & 31;
        cache__startByteIndices[cacheIndex] = decoder__startByteIndex;
        cache__startCodepointIndex = decoder__codepointIndex;
    }

    void inline stopCache() {
        cache__endCodepointIndex = decoder__codepointIndex;
    }

    // Decoder methods
    void stripBOMAndShebang();
    
    // Lexer only
    size_t currentLine = 1;
    size_t currentColumn = 0; 
    std::vector<Token> tokens;
    size_t currentTokenIndex = 0;

    // Lexer methods
    Token currentToken();
    Token peekNextToken();
    void advanceToNextToken();
    void matchToken();
    void skipWhitespaceAndComments();

    // Parser only
    std::vector<ASTNode> astNodes;
    size_t currentNodeIndex = 0;

    // Parser methods
    ASTNode currentNode();
    ASTNode peekNextNode();
    void advanceToNextNode();
};


