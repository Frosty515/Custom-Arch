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

#ifndef _PARSER_HPP
#define _PARSER_HPP

#include <stddef.h>

#include <LinkedList.hpp>

#include "Lexer.hpp"

#include <libarch/Instruction.hpp>
#include <libarch/Operand.hpp>

class Parser {
public:
    Parser();
    ~Parser();

    void parse(const LinkedList::RearInsertLinkedList<Token>& tokens);

    void PrintSections(FILE* fd) const;

    void Clear();

    const LinkedList::RearInsertLinkedList<InsEncoding::Label>& GetLabels() const;

private:
    InsEncoding::Opcode GetOpcode(const char* name, size_t name_size) const;
    InsEncoding::Register GetRegister(const char* name, size_t name_size) const;
    void error(const char* message);

    const char* GetInstructionName(InsEncoding::Opcode opcode) const;
    const char* GetRegisterName(InsEncoding::Register reg) const;

private:
    LinkedList::RearInsertLinkedList<InsEncoding::Label> m_labels;
};

#endif /* _PARSER_HPP */