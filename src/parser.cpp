#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <functional>
#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"

// Definition table: map from keyword/pattern hash to parser
std::unordered_map<std::string, StatementParser> statementDefinitions;

// Hash function for patterns (simple: use the first word)
std::string computePatternId(const std::string& stmtText) {
    size_t spacePos = stmtText.find(' ');
    return (spacePos != std::string::npos) ? stmtText.substr(0, spacePos) : stmtText;
}

// Register a statement definition
void registerStatement(const std::string& keyword, StatementParser parser) {
    statementDefinitions[keyword] = std::move(parser);
}

// Parser class implementation
Parser::Parser(const std::string& source) : source_(source) {}

std::unique_ptr<Program> Parser::parse() {
    Lexer lexer(source_);
    tokens_ = lexer.tokenize();
    pos_ = 0;
    initDefinitions();

    auto program = std::make_unique<Program>();
    parseBlock(program->root, 0);
    return program;
}

// Parse a block at given indentation level
void Parser::parseBlock(Block& block, int indentLevel) {
    while (current()) {
        const Token* tok = current();
        if (tok->type == TokenType::EndOfFile) break;

        int currentIndent = tok->column - 1;
        if (currentIndent < indentLevel) break;
        if (currentIndent > indentLevel) {
            std::cerr << "Unexpected indentation at line " << tok->line << std::endl;
            advance();
            continue;
        }

        std::unique_ptr<ASTNode> stmt = parseStatement(indentLevel);
        if (stmt) {
            block.add(std::move(stmt));
        } else {
            advance();
        }
    }
}

// Generalized parseStatement: look up in definition table
std::unique_ptr<ASTNode> Parser::parseStatement(int indentLevel) {
    std::string stmtText;
    int startLine = current() ? current()->line : 0;
    while (current() && current()->line == startLine && current()->type != TokenType::EndOfFile) {
        if (!stmtText.empty()) stmtText += " ";
        stmtText += current()->value;
        advance();
    }
    if (stmtText.empty()) return nullptr;

    // Compute pattern ID (first word)
    std::string id = computePatternId(stmtText);

    // Look up in definition table
    auto it = statementDefinitions.find(id);
    if (it != statementDefinitions.end()) {
        return it->second(stmtText, indentLevel, *this);
    }

    // Default: simple statement
    return std::make_unique<Statement>(stmtText);
}

// Helper to parse a sub-block
std::unique_ptr<Block> Parser::parseSubBlock(int indentLevel) {
    auto block = std::make_unique<Block>();
    parseBlock(*block, indentLevel);
    return block;
}

// Public accessors for parsers
const Token* Parser::getCurrent() const {
    return current();
}

void Parser::advanceToken() {
    advance();
}

const Token* Parser::current() const {
    return pos_ < tokens_.size() ? &tokens_[pos_] : nullptr;
}

void Parser::advance() {
    ++pos_;
}

// Specific parsers
std::unique_ptr<ASTNode> parseIf(const std::string& stmtText, int indentLevel, Parser& parser) {
    // Assume format: "if condition :"
    size_t colonPos = stmtText.find(" :");
    if (colonPos == std::string::npos) return std::make_unique<Statement>(stmtText); // fallback

    std::string condition = stmtText.substr(3, colonPos - 3); // after "if "
    auto thenBlock = parser.parseSubBlock(indentLevel + 4);

    // Check for else
    std::unique_ptr<Block> elseBlock = nullptr;
    if (parser.getCurrent() && parser.getCurrent()->value == "else" && static_cast<int>(parser.getCurrent()->column - 1) == indentLevel) {
        parser.advanceToken(); // skip "else"
        if (parser.getCurrent() && parser.getCurrent()->value == ":") parser.advanceToken();
        elseBlock = parser.parseSubBlock(indentLevel + 4);
    }

    return std::make_unique<IfStatement>(condition, std::move(thenBlock), std::move(elseBlock));
}

// Initialize definitions
void initDefinitions() {
    registerStatement("if", parseIf);
    // Add more: registerStatement("for", parseFor);
}
