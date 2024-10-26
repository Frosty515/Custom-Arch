/*
Copyright (©) 2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
    SIZE,
    LBRACKET,
    RBRACKET,
    DIRECTIVE,
    BLABEL, // Label used to mark the start of a block
    BSUBLABEL, // Label used to mark the start of a sub-block
    LABEL,
    SUBLABEL,
    COMMA,
    OPERATOR,
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

    const LinkedList::RearInsertLinkedList<Token>& GetTokens() const;

    static const char* TokenTypeToString(TokenType type);

    void Clear();

private:

    void AddToken(const std::string& token);

private:
    LinkedList::RearInsertLinkedList<Token> m_tokens;
};

#endif /* _LEXER_HPP */