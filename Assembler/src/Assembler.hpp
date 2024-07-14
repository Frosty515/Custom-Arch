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