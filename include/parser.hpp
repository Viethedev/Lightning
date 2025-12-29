#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "lexer.hpp"
#include "ast.hpp"

// Forward declarations
struct Program;

// StatementParser function type: takes statement text, indent level, and returns ASTNode
using StatementParser = std::function<std::unique_ptr<ASTNode>(const std::string& stmtText, int indentLevel, class Parser& parser)>;

// Definition table: map from keyword/pattern hash to parser
extern std::unordered_map<std::string, StatementParser> statementDefinitions;

// Hash function for patterns (simple: use the first word)
std::string computePatternId(const std::string& stmtText);

// Register a statement definition
void registerStatement(const std::string& keyword, StatementParser parser);

// Parser class
class Parser {
public:
    explicit Parser(const std::string& source);

    std::unique_ptr<Program> parse();

    // Parse a block at given indentation level
    void parseBlock(Block& block, int indentLevel);

    // Generalized parseStatement: look up in definition table
    std::unique_ptr<ASTNode> parseStatement(int indentLevel);

    // Helper to parse a sub-block
    std::unique_ptr<Block> parseSubBlock(int indentLevel);

    // Public accessors for parsers
    const Token* getCurrent() const;
    void advanceToken();

private:
    std::string source_;
    std::vector<Token> tokens_;
    size_t pos_;

    const Token* current() const;
    void advance();
};

// Specific parsers
std::unique_ptr<ASTNode> parseIf(const std::string& stmtText, int indentLevel, Parser& parser);

// Initialize definitions
void initDefinitions();
