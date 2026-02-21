#pragma once
#include "lexer.hpp"

typedef uint32_t NodeId;

enum NodeType: uint32_t {
    AST_UNKNOWN,
    AST_FUNCDEF, AST_ARG, AST_FUNCCALL, 
    AST_BLOCK, AST_TYPE,
    AST_NULL, // Filler type
};

typedef struct {
    NodeType type;
    NodeId a;
    NodeId b;
    NodeId c;
    uint64_t payload;
} AstNode;

class Parser {
    public:
    Parser(const std::vector<Token>& tokens, const std::string& reference);
    NodeId parse();

    private:
    std::vector<AstNode> nodes;
    char stringPool[1 << 12] = {};
    const char* source;
    const std::vector<Token>& toks;
    const Token* begin;
    const Token* current;
    const Token* end;

    // Helper
    NodeId createNode(NodeType type, NodeId a = 0, NodeId b = 0, NodeId c = 0, uint64_t payload = 0) {
        nodes.push_back(AstNode {type, a, b, c, payload});
        return static_cast<NodeId>(nodes.size() - 1);
    }
    uint32_t internString(char* root, uint32_t offset, uint32_t length) {
        
    }

    // Parsing
    NodeId parseDeclaration();

    NodeId parseFunction();
    NodeId parseStatement();

    NodeId parseBlock();
    NodeId parseIf();
    NodeId parseWhile();
    NodeId parseReturn();

    NodeId parseExpression();
    NodeId parseAssignment();
    NodeId parseEquality();
    NodeId parseComparison();
    NodeId parseTerm();
    NodeId parseFactor();
    NodeId parseUnary();
    NodeId parsePrimary();
};

