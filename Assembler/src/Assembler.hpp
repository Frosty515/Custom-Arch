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

#ifndef _ASSEMBLER_HPP
#define _ASSEMBLER_HPP

#include "Buffer.hpp"
#include "Parser.hpp"
#include "LinkedList.hpp"

#include <stdint.h>

class Section {
public:
    

    Section(char* name, uint64_t name_size, uint64_t offset);
    ~Section();

    char const* GetName() const;
    uint64_t GetNameSize() const;
    uint64_t GetOffset() const;

private:
    char* m_name;
    uint64_t m_name_size;
    uint64_t m_offset;
};

class Assembler {
public:
    Assembler();
    ~Assembler();

    void assemble(const LinkedList::SimpleLinkedList<Label>& data);

    const Buffer& GetBuffer() const;

private:
    uint8_t GetRealOpcode(Opcode opcode) const;

    void error(const char* message) const;

    uint64_t m_current_offset;
    Buffer m_buffer;
    LinkedList::SimpleLinkedList<Section> m_sections;
};

#endif /* _ASSEMBLER_HPP */