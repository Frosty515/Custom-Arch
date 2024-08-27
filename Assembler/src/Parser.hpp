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

enum class Register {
    r0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
    scp,
    sbp,
    stp,
    cr0,
    cr1,
    cr2,
    cr3,
    cr4,
    cr5,
    cr6,
    cr7,
    sts,
    ip,
    unknown
};

struct RegisterOffsetData {
    Register reg;
    uint64_t offset;
};

enum class OperandType {
    REGISTER,
    IMMEDIATE,
    MEMORY,
    REGISTER_OFFSET,
    POTENTIAL_MEMORY, // not sure if it is memory or register+offset
    LABEL,
    SUBLABEL
};

enum class OperandSize {
    BYTE,
    WORD,
    DWORD,
    QWORD
};

struct Operand {
    OperandType type;
    OperandSize size;
    void* data;
    bool complete;
};



enum class Opcode {
    PUSH,
    POP,
    PUSHA,
    POPA,
    ADD,
    MUL,
    SUB,
    DIV,
    OR,
    XOR,
    NOR,
    AND,
    NAND,
    NOT,
    CMP,
    INC,
    DEC,
    SHL,
    SHR,
    RET,
    CALL,
    JMP,
    JC,
    JNC,
    JZ,
    JNZ,
    SYSCALL,
    SYSRET,
    ENTERUSER,
    INT,
    LIDT,
    IRET,
    MOV,
    NOP,
    HLT,
    UNKNOWN
};

struct Instruction {
    Opcode opcode;
    LinkedList::SimpleLinkedList<Operand> operands;
};

enum class RawDataType {
    RAW,
    LABEL,
    SUBLABEL
};

struct RawData {
    void* data;
    size_t data_size;
    RawDataType type;
};

struct Data {
    Data() {}
    ~Data() {}
    bool type; // true for instruction, false for raw data
    void* data;
};

struct Block {
    char* name;
    size_t name_size;
    LinkedList::SimpleLinkedList<Data> data_blocks;
    LinkedList::SimpleLinkedList<uint64_t> jumps_to_here;
};


struct Label {
    char* name;
    size_t name_size;
    LinkedList::SimpleLinkedList<Block> blocks;
};

class Parser {
public:
    Parser();
    ~Parser();

    void parse(const LinkedList::SimpleLinkedList<Token>& tokens);

    void PrintSections(FILE* fd) const;

    const LinkedList::SimpleLinkedList<Label>& GetLabels() const;

private:
    Opcode GetOpcode(const char* name, size_t name_size) const;
    Register GetRegister(const char* name, size_t name_size) const;
    void error(const char* message);

    const char* GetInstructionName(Opcode opcode) const;
    const char* GetRegisterName(Register reg) const;

private:
    LinkedList::SimpleLinkedList<Label> m_labels;
};

#endif /* _PARSER_HPP */