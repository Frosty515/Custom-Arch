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

#include "Lexer.hpp"

#include <stdio.h>
#include <string.h>

#include <util.h>

Lexer::Lexer() {
}

Lexer::~Lexer() {
}

void Lexer::tokenize(const char* source, size_t source_size) {
    if (source == nullptr || source_size == 0)
        return;
    
    bool start_of_token = true;
    std::string token = "";
    uint64_t current_offset_in_token = 0;

    for (uint64_t i = 0; i < source_size; i++) {
        if (start_of_token) {
            if (source[i] == ' ' || source[i] == '\n' || source[i] == '\t')
                continue;
            else if (source[i] == '[' || source[i] == ']' || source[i] == ',') {
                token += source[i];
                AddToken(token);
                token = "";
                current_offset_in_token = 0;
            }
            else if ((source[i] == '+' || source[i] == '*') || (source[i] == '-' && ((i + 1) >= source_size || !(source[i + 1] >= '0' && source[i + 1] <= '9')))) {
                token += source[i];
                AddToken(token);
                token = "";
                current_offset_in_token = 0;
            }
            else {
                start_of_token = false;
                token += source[i];
                current_offset_in_token++;
            }
        }
        else {
            if (source[i] == ' ' || source[i] == '\n' || source[i] == '\t') {
                start_of_token = true;
                AddToken(token);
                token = "";
                current_offset_in_token = 0;
            }
            else if (source[i] == '[' || source[i] == ']' || source[i] == ',') {
                start_of_token = true;
                AddToken(token);
                token = "";
                current_offset_in_token = 0;
                token += source[i];
                AddToken(token);
                token = "";
            }
            else if (source[i] == '+' || source[i] == '*' || source[i] == '-') {
                if (source[i] == '-' && (i + 1) < source_size) {
                    if (source[i + 1] >= '0' && source[i + 1] <= '9') { // do not read outside of bounds
                        start_of_token = true;
                        AddToken(token);
                        token = "";
                        token += source[i];
                        current_offset_in_token = 1;
                        continue;
                    }
                }
                start_of_token = true;
                AddToken(token);
                token = "";
                current_offset_in_token = 0;
                token += source[i];
                AddToken(token);
                token = "";
            }
            else {
                token += source[i];
                current_offset_in_token++;
            }
        }
    }
    for (uint64_t i = 0; i < current_offset_in_token; i++) {
        if (token[i] == ' ' || token[i] == '\n' || token[i] == '\t')
            continue;
        else {
            AddToken(token);
            token = "";
            break;
        }
    }

}

const LinkedList::RearInsertLinkedList<Token>& Lexer::GetTokens() const {
    return m_tokens;
}

const char* Lexer::TokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::INSTRUCTION:
        return "INSTRUCTION";
    case TokenType::REGISTER:
        return "REGISTER";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::SIZE:
        return "SIZE";
    case TokenType::LBRACKET:
        return "LBRACKET";
    case TokenType::RBRACKET:
        return "RBRACKET";
    case TokenType::DIRECTIVE:
        return "DIRECTIVE";
    case TokenType::BLABEL:
        return "BLABEL";
    case TokenType::BSUBLABEL:
        return "BSUBLABEL";
    case TokenType::LABEL:
        return "LABEL";
    case TokenType::SUBLABEL:
        return "SUBLABEL";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::OPERATOR:
        return "OPERATOR";
    case TokenType::UNKNOWN:
        return "UNKNOWN";
    default:
        return "UNKNOWN";
    }
}

void Lexer::Clear() {
    m_tokens.EnumerateReverse([&](Token* token) -> bool {
        delete[] (char*)token->data;
        delete token;
        return true;
    });
    m_tokens.clear();
}

void Lexer::AddToken(const std::string& g_token) {
    std::string token = "";
    for (char c : g_token) { // convert the token to lowercase
        if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 'a';
        token += c;
    }
    Token* new_token = new Token;
    new_token->data = new char[token.size() + 1];
    memcpy(new_token->data, token.c_str(), token.size());
    ((char*)new_token->data)[token.size()] = 0;
    new_token->data_size = token.size();
    
    /* now we identify the token type */
#define IS_REGISTER(token) (token == "r0" || token == "r1" || token == "r2" || token == "r3" || token == "r4" || token == "r5" || token == "r6" || token == "r7" || token == "r8" || token == "r9" || token == "r10" || token == "r11" || token == "r12" || token == "r13" || token == "r14" || token == "r15" || token == "scp" || token == "sbp" || token == "stp" || token == "cr0" || token == "cr1" || token == "cr2" || token == "cr3" || token == "cr4" || token == "cr5" || token == "cr6" || token == "cr7" || token == "sts" || token == "ip")
#define IS_INSTRUCTION(token) (token == "push" || token == "pop" || token == "pusha" || token == "popa" || token == "add" || token == "mul" || token == "sub" || token == "div" || token == "or" || token == "xor" || token == "nor" || token == "and" || token == "nand" || token == "not" || token == "cmp" || token == "inc" || token == "dec" || token == "shl" || token == "shr" || token == "ret" || token == "call" || token == "jmp" || token == "jc" || token == "jnc" || token == "jz" || token == "jnz" || token == "int" || token == "lidt" || token == "iret" || token == "mov" || token == "nop" || token == "hlt" || token == "syscall" || token == "sysret" || token == "enteruser")
    if IS_REGISTER(token)
        new_token->type = TokenType::REGISTER;
    /*else if (token == "byte" || token == "word" || token == "dword" || token == "qword")
        new_token->type = TokenType::SIZE;*/
    else if IS_INSTRUCTION(token)
        new_token->type = TokenType::INSTRUCTION;
    else if (token == "[")
        new_token->type = TokenType::LBRACKET;
    else if (token == "]")
        new_token->type = TokenType::RBRACKET;
    else if (token == ",")
        new_token->type = TokenType::COMMA;
    else if (token == "db" || token == "dw" || token == "dd" || token == "dq")
        new_token->type = TokenType::DIRECTIVE;
    else if (token == "byte" || token == "word" || token == "dword" || token == "qword")
        new_token->type = TokenType::SIZE;
    else if (token == "+" || token == "-" || token == "*")
        new_token->type = TokenType::OPERATOR;
    else {
        uint64_t offset = 0;
        uint64_t size = token.size();
        if (size == 0)
            new_token->type = TokenType::UNKNOWN;
        else {
            if (token[0] == '.')
                offset++;
            if (token[size - 1] == ':')
                size--;

            bool is_label = true;
            
            for (uint64_t i = 0; (i + offset) < size; i++) {
                if (!((token[i + offset] >= 'a' && token[i + offset] <= 'z') || (i > 0 && token[i + offset] >= '0' && token[i + offset] <= '9') || ((i + offset + 1) < size && token[i + offset] == '_'))) {
                    is_label = false;
                    break;
                }
            }

            if (is_label) {
                if (offset == 0 && size == token.size())
                    new_token->type = TokenType::LABEL;
                else if (offset == 1 && size == token.size())
                    new_token->type = TokenType::SUBLABEL;
                else if (offset == 0 && size < token.size())
                    new_token->type = TokenType::BLABEL;
                else if (offset == 1 && size < token.size())
                    new_token->type = TokenType::BSUBLABEL;
            }
            else {
                bool is_number = true;
                uint64_t i = 0;
                if (token[0] == '+')
                    i++;
                for (; i < token.size(); i++) {
                    if (token[i] < '0' || token[i] > '9') {
                        if (i == 0 && token[i] == '-')
                            continue;
                        is_number = false;
                        break;
                    }
                }
                if (is_number)
                    new_token->type = TokenType::NUMBER;
                else
                    new_token->type = TokenType::UNKNOWN;
            }
        }
    }
    m_tokens.insert(new_token);
#ifdef ASSEMBLER_DEBUG
    printf("Token: \"%s\", type = %s\n", token.c_str(), TokenTypeToString(new_token->type));
#endif
}