#include "parser.hpp"
#include "lexer.hpp"

Parser::Parser(const std::vector<Token>& tokens, const std::string& reference) : toks(tokens), source(reference.data()) {
    begin = toks.data();
    current = begin;
    end = toks.data() + toks.size();
    nodes.reserve(1024);
};

NodeId Parser::parse() {
    NodeId unit = parseDeclaration();
    
    while (current != end && current->type != TT_EOF) {
        NodeId statement = parseDeclaration();
        unit = createNode(AST_BLOCK, unit, statement);
    }

    return unit;
}

NodeId Parser::parseDeclaration() {
    const Token* ahead = current + 1;
    if (ahead == end) return parseStatement();
    else if (ahead->type == TT_LPAREN) return parseFunction();
    else if (ahead->type == TT_ASSIGNOP) return parseAssignment();
    else return parseStatement();
}

NodeId Parser::parseFunction() {
    node.nameOffset = current->offset;
    node.nameLength = current->length;
    ++current;
    ++current;
    const Token* ahead = current + 1;
    Argument arg;
    while (ahead != end && current->type == TT_IDENT && ahead->type == TT_PUNCT) {
        arg.nameOffset = current->offset;
        arg.nameLength = current->length;
        ++current;
        const char punct = source[ahead->offset];
        if (punct == ',') {
            
        }
    }
}