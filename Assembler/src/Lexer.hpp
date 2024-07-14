#ifndef _LEXER_HPP
#define _LEXER_HPP

/* A simple nasm-style custom assembly language. Mostly created without c++ things like strings */

#include <stddef.h>

#include <LinkedList.hpp>

#include <string>

enum class TokenType {
    INSTRUCTION,
    REGISTER,
    NUMBER,
    //SIZE,
    LBRACKET,
    RBRACKET,
    DIRECTIVE,
    BLABEL, // Label used to mark the start of a block
    BSUBLABEL, // Label used to mark the start of a sub-block
    LABEL,
    SUBLABEL,
    COMMA,
    UNKNOWN
};

struct Token {
    TokenType type;
    void* data;
    size_t data_size;
};

class Lexer {
public:
    Lexer();
    ~Lexer();

    void tokenize(const char* source, size_t source_size);

    const LinkedList::SimpleLinkedList<Token>& GetTokens() const;

    static const char* TokenTypeToString(TokenType type);

private:

    void AddToken(const std::string& token);

private:
    LinkedList::SimpleLinkedList<Token> m_tokens;
};

#endif /* _LEXER_HPP */