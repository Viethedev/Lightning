#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// AST Node definitions
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

struct Statement : ASTNode {
    std::string content;
    explicit Statement(std::string c) : content(std::move(c)) {}
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
        std::cout << "Stmt: " << content << std::endl;
    }
};

struct Block : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    void add(std::unique_ptr<ASTNode> stmt) { statements.push_back(std::move(stmt)); }
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
        std::cout << "Block:" << std::endl;
        for (const auto& stmt : statements) {
            stmt->print(indent + 1);
        }
    }
};

struct IfStatement : ASTNode {
    std::string condition;
    std::unique_ptr<Block> thenBlock;
    std::unique_ptr<Block> elseBlock;
    IfStatement(std::string cond, std::unique_ptr<Block> tb, std::unique_ptr<Block> eb = nullptr)
        : condition(std::move(cond)), thenBlock(std::move(tb)), elseBlock(std::move(eb)) {}
    void print(int indent = 0) const override {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
        std::cout << "If: " << condition << std::endl;
        thenBlock->print(indent + 1);
        if (elseBlock) {
            for (int i = 0; i < indent; ++i) std::cout << "  ";
            std::cout << "Else:" << std::endl;
            elseBlock->print(indent + 1);
        }
    }
};

struct Program : ASTNode {
    Block root;
    void print(int indent = 0) const override {
        root.print(indent);
    }
};