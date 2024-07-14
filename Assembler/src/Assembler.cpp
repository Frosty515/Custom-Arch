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

#include "Assembler.hpp"

#include <stdio.h>
#include <string.h>

Section::Section(char* name, uint64_t name_size, uint64_t offset) : m_name(name), m_name_size(name_size), m_offset(offset) {

}

Section::~Section() {

}

char const* Section::GetName() const {
    return m_name;
}

uint64_t Section::GetNameSize() const {
    return m_name_size;
}

uint64_t Section::GetOffset() const {
    return m_offset;
}


Assembler::Assembler() : m_current_offset(0), m_buffer() {
}

Assembler::~Assembler() {
}

/*
## Instructions

First 2 bytes are for the instruction stuff, and data follows it

### Layout

1. 1-byte opcode
2. 1-byte argument info

### Flags Layout

- bits 0-3: argument 1 info
- bits 4-7: argument 2 info

### Argument info layout

- bits 0 & 1: data type (0 for register, 1 for memory address, 2 for literal, 3 for register and offset)
- bits 2 & 3: data size (literals: 0 for 8-bit, 1 for 16-bit, 2 for 32-bit, 3 for 64-bit. other: 1-3 for unused argument) [NOTE: only used if literal type is used (because others have set sizes), otherwise ignored]
- If argument is unused: data type is 0, 1 or 3, data size is 1-3

### Register ID

- 8-bit integer that identifies a specific register
- first 4 bits are type (0 is general purpose, 1 is stack, 2 is control & flags & instruction pointers)
- last 4 bits are the number

### Register numbers

- General purpose registers: it is simply the GPR number
- Stack: scp is 0, sbp is 1, stp is 2, rest are reserved
- Control, Flags & IPs: CR0-CR3 are numbers 0-3, FLAGS is 4, I0 is 5, I1 is 6, rest are reserved

### Opcode

#### Numbering

- Bits 0-3 are the offset
- Bits 4-6 are the group (0 for ALU, 1 for control flow, 2 for IO, 3 for other, 4-7 are reserved)
- Bit 7 is reserved and should always be 0



*/

struct FlagsData {
    uint8_t arg1_type : 2;
    uint8_t arg1_size : 2;
    uint8_t arg2_type : 2;
    uint8_t arg2_size : 2;
} __attribute__((packed));

struct RegisterID {
    uint8_t type : 4;
    uint8_t number : 4;
} __attribute__((packed));

struct InstructionHeader {
    uint8_t opcode;
    FlagsData flags;
} __attribute__((packed));

void Assembler::assemble(const LinkedList::SimpleLinkedList<Label>& labels) {
    for (uint64_t i = 0; i < labels.getCount(); i++) {
        Label* label = labels.get(i);
        for (uint64_t j = 0; j < label->blocks.getCount(); j++) {
            Block* block = label->blocks.get(j);
            size_t name_size = label->name_size + block->name_size;
            char* name = new char[name_size + 1];
            memcpy(name, label->name, label->name_size);
            memcpy((void*)((uint64_t)name + label->name_size), block->name, block->name_size);
            name[name_size] = '\0';
            Section* section = new Section(name, name_size, m_current_offset);
            m_sections.insert(section);

            for (uint64_t k = 0; k < block->data_blocks.getCount(); k++) {
                Data* data = block->data_blocks.get(k);
                if (data->type) { // instruction
                    Instruction* instruction = (Instruction*)data->data;
                    InstructionHeader header;
                    if (instruction->operands.getCount() > 2)
                        error("Instruction has more than 2 operands");
                    if (instruction->operands.getCount() == 0) {
                        if (instruction->opcode != Opcode::NOP && instruction->opcode != Opcode::HLT
                        && instruction->opcode != Opcode::RET
                        && instruction->opcode != Opcode::IRET
                        && instruction->opcode != Opcode::PUSHA && instruction->opcode != Opcode::POPA)
                            error("Instruction has no operands");
                        header.flags.arg1_size = 1;
                        header.flags.arg1_type = 0;
                        header.flags.arg2_size = 1;
                        header.flags.arg2_type = 0;
                    }
                    if (instruction->operands.getCount() == 1) {
                        if (instruction->opcode != Opcode::NOP
                        && instruction->opcode != Opcode::PUSH && instruction->opcode != Opcode::POP
                        && instruction->opcode != Opcode::INC && instruction->opcode != Opcode::DEC && instruction->opcode != Opcode::NOT
                        && instruction->opcode != Opcode::CALL && instruction->opcode != Opcode::JMP && instruction->opcode != Opcode::JC && instruction->opcode != Opcode::JNC && instruction->opcode != Opcode::JZ && instruction->opcode != Opcode::JNZ
                        && instruction->opcode != Opcode::INT && instruction->opcode != Opcode::LIDT)
                            error("Instruction has 1 operand");
                        header.flags.arg2_size = 1;
                        header.flags.arg2_type = 0;
                        Operand* operand = instruction->operands.get(0);
                        if (operand->type == OperandType::REGISTER) {
                            header.flags.arg1_type = 0;
                            header.flags.arg1_size = 0;
                        }
                        else if (operand->type == OperandType::MEMORY) {
                            header.flags.arg1_type = 1;
                            header.flags.arg1_size = 0;
                        }
                        else if (operand->type == OperandType::IMMEDIATE || operand->type == OperandType::LABEL || operand->type == OperandType::SUBLABEL) {
                            header.flags.arg1_type = 2;
                            switch (operand->size) {
                                case OperandSize::BYTE:
                                    header.flags.arg1_size = 0;
                                    break;
                                case OperandSize::WORD:
                                    header.flags.arg1_size = 1;
                                    break;
                                case OperandSize::DWORD:
                                    header.flags.arg1_size = 2;
                                    break;
                                case OperandSize::QWORD:
                                    header.flags.arg1_size = 3;
                                    break;
                                default:
                                    error("Invalid operand size");
                                    break;
                            }
                        }
                        else if (operand->type == OperandType::REGISTER_OFFSET) {
                            header.flags.arg1_type = 3;
                            header.flags.arg1_size = 0;
                        }
                        else
                            error("Invalid operand type");
                    }
                    if (instruction->operands.getCount() == 2) {
                        if (instruction->opcode != Opcode::ADD && instruction->opcode != Opcode::MUL && instruction->opcode != Opcode::SUB && instruction->opcode != Opcode::DIV && instruction->opcode != Opcode::OR && instruction->opcode != Opcode::XOR && instruction->opcode != Opcode::NOR && instruction->opcode != Opcode::AND && instruction->opcode != Opcode::NAND && instruction->opcode != Opcode::CMP && instruction->opcode != Opcode::SHL && instruction->opcode != Opcode::SHR
                        && instruction->opcode != Opcode::MOV && instruction->opcode != Opcode::NOP
                        && instruction->opcode != Opcode::INB && instruction->opcode != Opcode::OUTB && instruction->opcode != Opcode::INW && instruction->opcode != Opcode::OUTW && instruction->opcode != Opcode::IND && instruction->opcode != Opcode::OUTD && instruction->opcode != Opcode::INQ && instruction->opcode != Opcode::OUTQ) {
                            printf("Opcode = %d\n", (int)instruction->opcode);
                            error("Instruction has 2 operands");
                        }
                        Operand* operand1 = instruction->operands.get(0);
                        if (operand1->type == OperandType::REGISTER) {
                            header.flags.arg1_type = 0;
                            header.flags.arg1_size = 0;
                        }
                        else if (operand1->type == OperandType::MEMORY) {
                            header.flags.arg1_type = 1;
                            header.flags.arg1_size = 0;
                        }
                        else if (operand1->type == OperandType::IMMEDIATE || operand1->type == OperandType::LABEL || operand1->type == OperandType::SUBLABEL) {
                            header.flags.arg1_type = 2;
                            switch (operand1->size) {
                                case OperandSize::BYTE:
                                    header.flags.arg1_size = 0;
                                    break;
                                case OperandSize::WORD:
                                    header.flags.arg1_size = 1;
                                    break;
                                case OperandSize::DWORD:
                                    header.flags.arg1_size = 2;
                                    break;
                                case OperandSize::QWORD:
                                    header.flags.arg1_size = 3;
                                    break;
                                default:
                                    error("Invalid operand size");
                                    break;
                            }
                        }
                        else if (operand1->type == OperandType::REGISTER_OFFSET) {
                            header.flags.arg1_type = 3;
                            header.flags.arg1_size = 0;
                        }
                        else
                            error("Invalid operand type");
                        Operand* operand2 = instruction->operands.get(1);
                        if (operand2->type == OperandType::REGISTER) {
                            header.flags.arg2_type = 0;
                            header.flags.arg2_size = 0;
                        }
                        else if (operand2->type == OperandType::MEMORY) {
                            header.flags.arg2_type = 1;
                            header.flags.arg2_size = 0;
                        }
                        else if (operand2->type == OperandType::IMMEDIATE || operand2->type == OperandType::LABEL || operand2->type == OperandType::SUBLABEL) {
                            header.flags.arg2_type = 2;
                            switch (operand2->size) {
                                case OperandSize::BYTE:
                                    header.flags.arg2_size = 0;
                                    break;
                                case OperandSize::WORD:
                                    header.flags.arg2_size = 1;
                                    break;
                                case OperandSize::DWORD:
                                    header.flags.arg2_size = 2;
                                    break;
                                case OperandSize::QWORD:
                                    header.flags.arg2_size = 3;
                                    break;
                                default:
                                    error("Invalid operand size");
                                    break;
                            }
                        }
                        else if (operand2->type == OperandType::REGISTER_OFFSET) {
                            header.flags.arg2_type = 3;
                            header.flags.arg2_size = 0;
                        }
                        else
                            error("Invalid operand type");
                    }

                    header.opcode = GetRealOpcode(instruction->opcode);

                    m_buffer.Write(m_current_offset, (uint8_t*)&header, sizeof(InstructionHeader));
                    m_current_offset += sizeof(InstructionHeader);
                    
                    for (uint64_t l = 0; l < instruction->operands.getCount(); l++) {
                        Operand* operand = instruction->operands.get(l);
                        if (operand->type == OperandType::REGISTER) {
                            RegisterID reg_id;
                            Register* reg = (Register*)operand->data;
#define REG_CASE(name, group, num) case Register::name: reg_id.type = group; reg_id.number = num; break;
                            switch (*reg) {
                                REG_CASE(r0, 0, 0)
                                REG_CASE(r1, 0, 1)
                                REG_CASE(r2, 0, 2)
                                REG_CASE(r3, 0, 3)
                                REG_CASE(r4, 0, 4)
                                REG_CASE(r5, 0, 5)
                                REG_CASE(r6, 0, 6)
                                REG_CASE(r7, 0, 7)
                                REG_CASE(r8, 0, 8)
                                REG_CASE(r9, 0, 9)
                                REG_CASE(r10, 0, 10)
                                REG_CASE(r11, 0, 11)
                                REG_CASE(r12, 0, 12)
                                REG_CASE(r13, 0, 13)
                                REG_CASE(r14, 0, 14)
                                REG_CASE(r15, 0, 15)
                                REG_CASE(scp, 1, 0)
                                REG_CASE(sbp, 1, 1)
                                REG_CASE(stp, 1, 2)
                                REG_CASE(cr0, 2, 0)
                                REG_CASE(cr1, 2, 1)
                                REG_CASE(cr2, 2, 2)
                                REG_CASE(cr3, 2, 3)
                                REG_CASE(flags, 2, 4)
                                REG_CASE(i0, 2, 5)
                                REG_CASE(i1, 2, 6)
                                default:
                                    error("Invalid register type");
                                    break;
                            }
#undef REG_CASE
                            m_buffer.Write(m_current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                            m_current_offset += sizeof(RegisterID);
                        }
                        else if (operand->type == OperandType::MEMORY) {
                            m_buffer.Write(m_current_offset, (uint8_t*)operand->data, 8);
                            m_current_offset += 8;
                        }
                        else if (operand->type == OperandType::IMMEDIATE) {
                            switch (operand->size) {
                                case OperandSize::BYTE:
                                    m_buffer.Write(m_current_offset, (uint8_t*)operand->data, 1);
                                    m_current_offset += 1;
                                    break;
                                case OperandSize::WORD:
                                    m_buffer.Write(m_current_offset, (uint8_t*)operand->data, 2);
                                    m_current_offset += 2;
                                    break;
                                case OperandSize::DWORD:
                                    m_buffer.Write(m_current_offset, (uint8_t*)operand->data, 4);
                                    m_current_offset += 4;
                                    break;
                                case OperandSize::QWORD:
                                    m_buffer.Write(m_current_offset, (uint8_t*)operand->data, 8);
                                    m_current_offset += 8;
                                    break;
                                default:
                                    error("Invalid operand size");
                                    break;
                            }
                        }
                        else if (operand->type == OperandType::REGISTER_OFFSET) {
                            RegisterOffsetData* reg_offset = (RegisterOffsetData*)operand->data;
                            RegisterID reg_id;
#define REG_CASE(name, group, num) case Register::name: reg_id.type = group; reg_id.number = num; break;
                            switch (reg_offset->reg) {
                            REG_CASE(r0, 0, 0)
                            REG_CASE(r1, 0, 1)
                            REG_CASE(r2, 0, 2)
                            REG_CASE(r3, 0, 3)
                            REG_CASE(r4, 0, 4)
                            REG_CASE(r5, 0, 5)
                            REG_CASE(r6, 0, 6)
                            REG_CASE(r7, 0, 7)
                            REG_CASE(r8, 0, 8)
                            REG_CASE(r9, 0, 9)
                            REG_CASE(r10, 0, 10)
                            REG_CASE(r11, 0, 11)
                            REG_CASE(r12, 0, 12)
                            REG_CASE(r13, 0, 13)
                            REG_CASE(r14, 0, 14)
                            REG_CASE(r15, 0, 15)
                            REG_CASE(scp, 1, 0)
                            REG_CASE(sbp, 1, 1)
                            REG_CASE(stp, 1, 2)
                            REG_CASE(cr0, 2, 0)
                            REG_CASE(cr1, 2, 1)
                            REG_CASE(cr2, 2, 2)
                            REG_CASE(cr3, 2, 3)
                            REG_CASE(flags, 2, 4)
                            REG_CASE(i0, 2, 5)
                            REG_CASE(i1, 2, 6)
                            default:
                                error("Invalid register type");
                                break;
                            }
#undef REG_CASE
                            m_buffer.Write(m_current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                            m_current_offset += sizeof(RegisterID);
                            m_buffer.Write(m_current_offset, (uint8_t*)&(reg_offset->offset), 8);
                            m_current_offset += 8;
                        }
                        else if (operand->type == OperandType::LABEL) {
                            Label* i_label = (Label*)operand->data;
                            Block* i_block = i_label->blocks.get(0);
                            uint64_t* offset = new uint64_t;
                            *offset = m_current_offset;
                            i_block->jumps_to_here.insert(offset);
                            uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                            m_buffer.Write(m_current_offset, (uint8_t*)&temp_offset, 8);
                            m_current_offset += 8;
                        }
                        else if (operand->type == OperandType::SUBLABEL) {
                            Block* i_block = (Block*)operand->data;
                            uint64_t* offset = new uint64_t;
                            *offset = m_current_offset;
                            i_block->jumps_to_here.insert(offset);
                            uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                            m_buffer.Write(m_current_offset, (uint8_t*)&temp_offset, 8);
                            m_current_offset += 8;
                        }
                        else {
                            printf("Operand type = %d\n", (int)operand->type);
                            error("Invalid operand type");
                        }
                    }
                }
                else { // raw data
                    RawData* raw_data = (RawData*)data->data;
                    switch (raw_data->type) {
                    case RawDataType::RAW:
                        m_buffer.Write(m_current_offset, (uint8_t*)raw_data->data, raw_data->data_size);
                        m_current_offset += raw_data->data_size;
                        break;
                    case RawDataType::LABEL: {
                        Label* i_label = (Label*)raw_data->data;
                        Block* i_block = i_label->blocks.get(0);
                        uint64_t* offset = new uint64_t;
                        *offset = m_current_offset;
                        i_block->jumps_to_here.insert(offset);
                        uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                        m_buffer.Write(m_current_offset, (uint8_t*)&temp_offset, 8);
                        m_current_offset += 8;
                        break;
                    }
                    case RawDataType::SUBLABEL: {
                        Block* i_block = (Block*)raw_data->data;
                        uint64_t* offset = new uint64_t;
                        *offset = m_current_offset;
                        i_block->jumps_to_here.insert(offset);
                        uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                        m_buffer.Write(m_current_offset, (uint8_t*)&temp_offset, 8);
                        m_current_offset += 8;
                        break;
                    }
                    }
                }
            }
        }
    }
    // Enumerate through the labels, and the blocks within them again and fill in the jumps
    uint64_t section_index = 0;
    for (uint64_t i = 0; i < labels.getCount(); i++) {
        Label* label = labels.get(i);
        for (uint64_t j = 0; j < label->blocks.getCount(); j++, section_index++) {
            Block* block = label->blocks.get(j);
            Section* section = m_sections.get(section_index);
            uint64_t real_offset = section->GetOffset();
            for (uint64_t k = 0; k < block->jumps_to_here.getCount(); k++) {
                uint64_t* offset = block->jumps_to_here.get(k);
                m_buffer.Write(*offset, (uint8_t*)&real_offset, 8);
            }
        }
    }

}

const Buffer& Assembler::GetBuffer() const {
    return m_buffer;
}


uint8_t Assembler::GetRealOpcode(Opcode opcode) const {
    switch (opcode) {
#define OPCODE_CASE(name, raw) case Opcode::name: return raw;
    OPCODE_CASE(ADD, 0x00)
    OPCODE_CASE(MUL, 0x01)
    OPCODE_CASE(SUB, 0x02)
    OPCODE_CASE(DIV, 0x03)
    OPCODE_CASE(OR, 0x04)
    OPCODE_CASE(XOR, 0x05)
    OPCODE_CASE(NOR, 0x06)
    OPCODE_CASE(AND, 0x07)
    OPCODE_CASE(NAND, 0x08)
    OPCODE_CASE(NOT, 0x09)
    OPCODE_CASE(CMP, 0x0a)
    OPCODE_CASE(INC, 0x0b)
    OPCODE_CASE(DEC, 0x0c)
    OPCODE_CASE(SHL, 0x0d)
    OPCODE_CASE(SHR, 0x0e)
    OPCODE_CASE(RET, 0x10)
    OPCODE_CASE(CALL, 0x11)
    OPCODE_CASE(JMP, 0x12)
    OPCODE_CASE(JC, 0x13)
    OPCODE_CASE(JNC, 0x14)
    OPCODE_CASE(JZ, 0x15)
    OPCODE_CASE(JNZ, 0x16)
    OPCODE_CASE(INB, 0x20)
    OPCODE_CASE(OUTB, 0x21)
    OPCODE_CASE(INW, 0x22)
    OPCODE_CASE(OUTW, 0x23)
    OPCODE_CASE(IND, 0x24)
    OPCODE_CASE(OUTD, 0x25)
    OPCODE_CASE(INQ, 0x26)
    OPCODE_CASE(OUTQ, 0x27)
    OPCODE_CASE(MOV, 0x30)
    OPCODE_CASE(NOP, 0x31)
    OPCODE_CASE(HLT, 0x32)
    OPCODE_CASE(PUSH, 0x33)
    OPCODE_CASE(POP, 0x34)
    OPCODE_CASE(PUSHA, 0x35)
    OPCODE_CASE(POPA, 0x36)
    OPCODE_CASE(INT, 0x37)
    OPCODE_CASE(LIDT, 0x38)
    OPCODE_CASE(IRET, 0x39)
    default:
        return 0xff;
#undef OPCODE_CASE
    }
}

void Assembler::error(const char* message) const {
    fprintf(stderr, "Assembler error: %s\n", message);
    exit(EXIT_FAILURE);
}