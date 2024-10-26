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

#include "Instruction.hpp"

#include "Data-structures/Buffer.hpp"
#include "Operand.hpp"

namespace InsEncoding {

    Instruction::Instruction(Opcode opcode) : m_opcode(opcode) {

    }

    Instruction::~Instruction() {

    }

    Opcode Instruction::GetOpcode() {
        return m_opcode;
    }

    void error(const char* message) {
        printf("Error: %s\n", message);
    }

    RegisterID GetRegisterID(Register reg) {
        RegisterID reg_id;
    #define REG_CASE(name, group, num) case Register::name: reg_id.type = group; reg_id.number = num; break;
        switch (reg) {
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
        REG_CASE(cr4, 2, 4)
        REG_CASE(cr5, 2, 5)
        REG_CASE(cr6, 2, 6)
        REG_CASE(cr7, 2, 7)
        REG_CASE(sts, 2, 8)
        REG_CASE(ip, 2, 9)
        default:
            error("Invalid register type");
            break;
        }
    #undef REG_CASE
        return reg_id;
    }

    Register GetRegisterFromID(RegisterID reg_id) {
        Register reg;
        switch (reg_id.type) {
#define REG_CASE(name, num) case num: reg = Register::name; break;
        case 0:
            switch (reg_id.number) {
                REG_CASE(r0, 0)
                REG_CASE(r1, 1)
                REG_CASE(r2, 2)
                REG_CASE(r3, 3)
                REG_CASE(r4, 4)
                REG_CASE(r5, 5)
                REG_CASE(r6, 6)
                REG_CASE(r7, 7)
                REG_CASE(r8, 8)
                REG_CASE(r9, 9)
                REG_CASE(r10, 10)
                REG_CASE(r11, 11)
                REG_CASE(r12, 12)
                REG_CASE(r13, 13)
                REG_CASE(r14, 14)
                REG_CASE(r15, 15)
            default:
                error("Invalid register number");
                reg = Register::unknown;
                break;
            }
            break;
        case 1:
            switch (reg_id.number) {
                REG_CASE(scp, 0)
                REG_CASE(sbp, 1)
                REG_CASE(stp, 2)
            default:
                error("Invalid register number");
                reg = Register::unknown;
                break;
            }
            break;
        case 2:
            switch (reg_id.number) {
                REG_CASE(cr0, 0)
                REG_CASE(cr1, 1)
                REG_CASE(cr2, 2)
                REG_CASE(cr3, 3)
                REG_CASE(cr4, 4)
                REG_CASE(cr5, 5)
                REG_CASE(cr6, 6)
                REG_CASE(cr7, 7)
                REG_CASE(sts, 8)
                REG_CASE(ip, 9)
            default:
                error("Invalid register number");
                reg = Register::unknown;
                break;
            }
            break;
        default:
            error("Invalid register type");
            reg = Register::unknown;
            break;
        }
#undef REG_CASE
        return reg;
    }

    uint8_t GetArgCountForOpcode(Opcode opcode) {
        switch (opcode) {
        case Opcode::ADD:
        case Opcode::MUL:
        case Opcode::SUB:
        case Opcode::DIV:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::NOR:
        case Opcode::AND:
        case Opcode::NAND:
        case Opcode::CMP:
        case Opcode::SHL:
        case Opcode::SHR:
        case Opcode::MOV:
            return 2;
        case Opcode::INC:
        case Opcode::DEC:
        case Opcode::CALL:
        case Opcode::JMP:
        case Opcode::JC:
        case Opcode::JNC:
        case Opcode::JZ:
        case Opcode::JNZ:
        case Opcode::ENTERUSER:
        case Opcode::PUSH:
        case Opcode::POP:
        case Opcode::INT:
        case Opcode::LIDT:
        case Opcode::NOT:
            return 1;
        case Opcode::HLT:
        case Opcode::NOP:
        case Opcode::SYSRET:
        case Opcode::SYSCALL:
        case Opcode::RET:
        case Opcode::PUSHA:
        case Opcode::POPA:
        case Opcode::IRET:
        default:
            return 0;
        }
    }

    Instruction* DecodeInstruction(const uint8_t* data, size_t data_size) {
        Buffer buffer(data_size);
        buffer.Write(0, data, data_size);
        uint64_t current_offset = 0;
        return DecodeInstruction(buffer, current_offset);
    }

    Instruction* DecodeInstruction(Buffer& buffer, uint64_t& current_offset) {
        uint8_t raw_opcode;
        buffer.Read(current_offset, &raw_opcode, 1);
        current_offset++;

        Instruction* instruction = new Instruction((Opcode)raw_opcode);

        uint8_t arg_count = GetArgCountForOpcode(instruction->GetOpcode());
        if (arg_count == 0)
            return instruction;

        OperandType operand_types[2];
        OperandSize operand_sizes[2];

        ComplexOperandInfo complex_infos[2];


        if (arg_count == 1) {
            // read 1 byte to get the operand type
            uint8_t raw_operand_info;
            buffer.Read(current_offset, &raw_operand_info, 1);
            current_offset++;
            StandardOperandInfo* temp_operand_info = (StandardOperandInfo*)&raw_operand_info;

            operand_types[0] = (OperandType)temp_operand_info->type;
            operand_sizes[0] = (OperandSize)temp_operand_info->size;

            if (operand_types[0] == OperandType::COMPLEX) {
                ComplexOperandInfo complex_info;
                buffer.Read(current_offset - 1, (uint8_t*)&complex_info, sizeof(ComplexOperandInfo));
                current_offset += sizeof(ComplexOperandInfo) - 1;
                complex_infos[0] = complex_info;
            }
        }
        else if (arg_count == 2) {
            // read 1 byte to get the operand types
            uint8_t raw_operand_info;
            buffer.Read(current_offset, &raw_operand_info, 1);
            current_offset++;
            StandardOperandInfo* temp_operand_info = (StandardOperandInfo*)&raw_operand_info;

            if (temp_operand_info->type == (uint8_t)OperandType::COMPLEX) {
                ComplexStandardOperandInfo temp_complex_info;
                buffer.Read(current_offset - 1, (uint8_t*)&temp_complex_info, sizeof(ComplexStandardOperandInfo));
                current_offset += sizeof(ComplexStandardOperandInfo) - 1;
                operand_types[0] = (OperandType)temp_complex_info.complex.type;
                operand_sizes[0] = (OperandSize)temp_complex_info.complex.size;
                operand_types[1] = (OperandType)temp_complex_info.standard.type;
                operand_sizes[1] = (OperandSize)temp_complex_info.standard.size;

                complex_infos[0] = temp_complex_info.complex;

                if (operand_types[1] == OperandType::COMPLEX) {
                    ComplexComplexOperandInfo temp_complex_info;
                    buffer.Read(current_offset - sizeof(ComplexStandardOperandInfo), (uint8_t*)&temp_complex_info, sizeof(ComplexComplexOperandInfo));
                    current_offset += sizeof(ComplexComplexOperandInfo) - sizeof(ComplexStandardOperandInfo);
                    complex_infos[1] = temp_complex_info.second;
                }
            }
            else {
                StandardStandardOperandInfo* temp_standard_info = (StandardStandardOperandInfo*)&raw_operand_info;
                operand_types[0] = (OperandType)temp_standard_info->first_type;
                operand_sizes[0] = (OperandSize)temp_standard_info->first_size;
                operand_types[1] = (OperandType)temp_standard_info->second_type;
                operand_sizes[1] = (OperandSize)temp_standard_info->second_size;

                if (operand_types[1] == OperandType::COMPLEX) {
                    StandardComplexOperandInfo temp_complex_info;
                    buffer.Read(current_offset - 1, (uint8_t*)&temp_complex_info, sizeof(StandardComplexOperandInfo));
                    current_offset += sizeof(StandardComplexOperandInfo) - 1;
                    complex_infos[1] = temp_complex_info.complex;
                    operand_sizes[1] = (OperandSize)temp_complex_info.complex.size;
                }
            }
        }
        else {
            error("Invalid argument count");
            return nullptr; // TODO: free memory
        }

        for (uint8_t i = 0; i < arg_count; i++) {
            OperandType operand_type = operand_types[i];
            OperandSize operand_size = operand_sizes[i];

            Operand* operand = new Operand(operand_type, operand_size, nullptr);

            switch (operand_type) {
            case OperandType::COMPLEX: {
                ComplexOperandInfo complex_info = complex_infos[i];
                ComplexData* complex = new ComplexData;
                complex->base.present = complex_info.base_present;
                if (complex->base.present) {
                    complex->base.type = complex_info.base_type == 0 ? ComplexItem::Type::REGISTER : ComplexItem::Type::IMMEDIATE;
                    if (complex->base.type == ComplexItem::Type::IMMEDIATE) {
                        complex->base.data.imm.size = OperandSize(complex_info.base_size);
                        complex->base.data.imm.data = new uint8_t[1 << complex_info.base_size];
                        buffer.Read(current_offset, (uint8_t*)(complex->base.data.imm.data), 1 << complex_info.base_size);
                        current_offset += 1 << complex_info.base_size;
                    }
                    else if (complex->base.type == ComplexItem::Type::REGISTER) {
                        RegisterID reg_id;
                        buffer.Read(current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                        current_offset += sizeof(RegisterID);
                        complex->base.data.reg = new Register(GetRegisterFromID(reg_id));
                    }
                }
                complex->index.present = complex_info.index_present;
                if (complex->index.present) {
                    complex->index.type = complex_info.index_type == 0 ? ComplexItem::Type::REGISTER : ComplexItem::Type::IMMEDIATE;
                    if (complex->index.type == ComplexItem::Type::IMMEDIATE) {
                        complex->index.data.imm.size = OperandSize(complex_info.index_size);
                        complex->index.data.imm.data = new uint8_t[1 << complex_info.index_size];
                        buffer.Read(current_offset, (uint8_t*)(complex->index.data.imm.data), 1 << complex_info.index_size);
                        current_offset += 1 << complex_info.index_size;
                    }
                    else if (complex->index.type == ComplexItem::Type::REGISTER) {
                        RegisterID reg_id;
                        buffer.Read(current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                        current_offset += sizeof(RegisterID);
                        complex->index.data.reg = new Register(GetRegisterFromID(reg_id));
                    }
                }
                complex->offset.present = complex_info.offset_present;
                if (complex->offset.present) {
                    complex->offset.type = complex_info.offset_type == 0 ? ComplexItem::Type::REGISTER : ComplexItem::Type::IMMEDIATE;
                    if (complex->offset.type == ComplexItem::Type::IMMEDIATE) {
                        complex->offset.data.imm.size = OperandSize(complex_info.offset_size);
                        complex->offset.data.imm.data = new uint8_t[1 << complex_info.offset_size];
                        buffer.Read(current_offset, (uint8_t*)(complex->offset.data.imm.data), 1 << complex_info.offset_size);
                        current_offset += 1 << complex_info.offset_size;
                    }
                    else if (complex->offset.type == ComplexItem::Type::REGISTER) {
                        RegisterID reg_id;
                        buffer.Read(current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                        current_offset += sizeof(RegisterID);
                        complex->offset.data.reg = new Register(GetRegisterFromID(reg_id));
                        complex->offset.sign = complex_info.offset_size;
                    }
                }
                operand->data = complex;
                break;
            }
            case OperandType::REGISTER: {
                RegisterID reg_id;
                buffer.Read(current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                current_offset += sizeof(RegisterID);
                operand->data = new Register(GetRegisterFromID(reg_id));
                break;
            }
            case OperandType::MEMORY: {
                uint8_t* mem_data = new uint8_t[8];
                buffer.Read(current_offset, mem_data, 8);
                current_offset += 8;
                operand->data = mem_data;
                break;
            }
            case OperandType::IMMEDIATE: {
                uint8_t* imm_data = new uint8_t[1 << (uint8_t)operand_size];
                buffer.Read(current_offset, imm_data, 1 << (uint8_t)operand_size);
                current_offset += 1 << (uint8_t)operand_size;
                operand->data = imm_data;
                break;
            }
            default:
                error("Invalid operand type");
                return nullptr; // TODO: free memory
            }
            
            instruction->operands.insert(operand);
        }

        return instruction;
    }

    size_t EncodeInstruction(Instruction* instruction, uint8_t* data, size_t data_size, uint64_t global_offset) {
        Buffer buffer;
        uint64_t current_offset = 0;

        if (instruction->operands.getCount() > 2)
            error("Instruction has more than 2 operands");

        uint8_t arg_count = GetArgCountForOpcode(instruction->GetOpcode());
        if (instruction->operands.getCount() != arg_count)
            error("Invalid number of arguments for instruction");

        uint8_t opcode = (uint8_t)instruction->GetOpcode();

        buffer.Write(current_offset, &opcode, 1);
        current_offset++;

        Operand* operands[2] = { nullptr, nullptr };
        for (uint64_t l = 0; l < instruction->operands.getCount(); l++)
            operands[l] = instruction->operands.get(l);

        if (arg_count == 1) {
            if (operands[0]->type == OperandType::COMPLEX) {
                ComplexData* complex = (ComplexData*)operands[0]->data;
                ComplexOperandInfo info;
                info.type = (uint8_t)OperandType::COMPLEX;
                info.size = (uint8_t)operands[0]->size;
                info.base_type = complex->base.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.base_size = complex->base.type == ComplexItem::Type::REGISTER ? 0 : ((complex->base.type == ComplexItem::Type::LABEL || complex->base.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex->base.data.imm.size);
                info.base_present = complex->base.present;
                info.index_type = complex->index.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.index_size = complex->index.type == ComplexItem::Type::REGISTER ? 0 : ((complex->index.type == ComplexItem::Type::LABEL || complex->index.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex->index.data.imm.size);
                info.index_present = complex->index.present;
                info.offset_type = complex->offset.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.offset_size = complex->offset.type == ComplexItem::Type::REGISTER ? complex->offset.sign : ((complex->offset.type == ComplexItem::Type::LABEL || complex->offset.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex->offset.data.imm.size);
                info.offset_present = complex->offset.present;
                buffer.Write(current_offset, (uint8_t*)&info, sizeof(ComplexOperandInfo));
                current_offset += sizeof(ComplexOperandInfo);
            }
            else {
                StandardOperandInfo info;
                if (operands[0]->type == OperandType::LABEL || operands[0]->type == OperandType::SUBLABEL) {
                    info.type = (uint8_t)OperandType::IMMEDIATE;
                    info.size = 3;
                }
                else {
                    info.type = (uint8_t)operands[0]->type;
                    info.size = (uint8_t)operands[0]->size;
                }
                info._padding = 0;
                buffer.Write(current_offset, (uint8_t*)&info, sizeof(StandardOperandInfo));
                current_offset += sizeof(StandardOperandInfo);
            }
        }
        else if (arg_count == 2) {
            if ((operands[0]->type == OperandType::COMPLEX && operands[1]->type != OperandType::COMPLEX) || (operands[1]->type == OperandType::COMPLEX && operands[0]->type != OperandType::COMPLEX)) {
                // start with a StandardComplexOperandInfo, and swap at the end if needed
                StandardComplexOperandInfo info;
                {
                    Operand* temp = operands[operands[0]->type != OperandType::COMPLEX ? 0 : 1];
                    if (temp->type == OperandType::LABEL || temp->type == OperandType::SUBLABEL) {
                        info.standard.type = (uint8_t)OperandType::IMMEDIATE;
                        info.standard.size = 3;
                    }
                    else {
                        info.standard.type = (uint8_t)temp->type;
                        info.standard.size = (uint8_t)temp->size;
                    }
                }
                info.standard._padding = 0;
                ComplexData* complex = (ComplexData*)operands[operands[0]->type == OperandType::COMPLEX ? 0 : 1]->data;
                info.complex.type = (uint8_t)OperandType::COMPLEX;
                info.complex.size = (uint8_t)operands[operands[0]->type == OperandType::COMPLEX ? 0 : 1]->size;
                info.complex.base_type = complex->base.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.complex.base_size = complex->base.type == ComplexItem::Type::REGISTER ? 0 : ((complex->base.type == ComplexItem::Type::LABEL || complex->base.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex->base.data.imm.size);
                info.complex.base_present = complex->base.present;
                info.complex.index_type = complex->index.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.complex.index_size = complex->index.type == ComplexItem::Type::REGISTER ? 0 : ((complex->index.type == ComplexItem::Type::LABEL || complex->index.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex->index.data.imm.size);
                info.complex.index_present = complex->index.present;
                info.complex.offset_type = complex->offset.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.complex.offset_size = complex->offset.type == ComplexItem::Type::REGISTER ? complex->offset.sign : ((complex->offset.type == ComplexItem::Type::LABEL || complex->offset.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex->offset.data.imm.size);
                info.complex.offset_present = complex->offset.present;
                if (operands[0]->type == OperandType::COMPLEX) {
                    ComplexStandardOperandInfo temp;
                    temp.complex = info.complex;
                    temp.standard = info.standard;
                    buffer.Write(current_offset, (uint8_t*)&temp, sizeof(ComplexStandardOperandInfo));
                    current_offset += sizeof(ComplexStandardOperandInfo);
                }
                else {
                    info.standard._padding = info.complex.type;
                    buffer.Write(current_offset, (uint8_t*)&info, sizeof(StandardComplexOperandInfo));
                    current_offset += sizeof(StandardComplexOperandInfo);
                }
            }
            else if (operands[0]->type == OperandType::COMPLEX && operands[1]->type == OperandType::COMPLEX) {
                ComplexComplexOperandInfo info;
                ComplexData* complex0 = (ComplexData*)operands[0]->data;
                ComplexData* complex1 = (ComplexData*)operands[1]->data;
                info.first.type = (uint8_t)OperandType::COMPLEX;
                info.first.size = (uint8_t)operands[0]->size;
                info.first.base_type = complex0->base.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.first.base_size = complex0->base.type == ComplexItem::Type::REGISTER ? 0 : ((complex0->base.type == ComplexItem::Type::LABEL || complex0->base.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex0->base.data.imm.size);
                info.first.base_present = complex0->base.present;
                info.first.index_type = complex0->index.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.first.index_size = complex0->index.type == ComplexItem::Type::REGISTER ? 0 : ((complex0->index.type == ComplexItem::Type::LABEL || complex0->index.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex0->index.data.imm.size);
                info.first.index_present = complex0->index.present;
                info.first.offset_type = complex0->offset.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.first.offset_size = complex0->offset.type == ComplexItem::Type::REGISTER ? complex0->offset.sign : ((complex0->offset.type == ComplexItem::Type::LABEL || complex0->offset.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex0->offset.data.imm.size);
                info.first.offset_present = complex0->offset.present;
                info.second.type = (uint8_t)OperandType::COMPLEX;
                info.second.size = (uint8_t)operands[1]->size;
                info.second.base_type = complex1->base.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.second.base_size = complex1->base.type == ComplexItem::Type::REGISTER ? 0 : ((complex1->base.type == ComplexItem::Type::LABEL || complex1->base.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex1->base.data.imm.size);
                info.second.base_present = complex1->base.present;
                info.second.index_type = complex1->index.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.second.index_size = complex1->index.type == ComplexItem::Type::REGISTER ? 0 : ((complex1->index.type == ComplexItem::Type::LABEL || complex1->index.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex1->index.data.imm.size);
                info.second.index_present = complex1->index.present;
                info.second.offset_type = complex1->offset.type == ComplexItem::Type::REGISTER ? 0 : 1;
                info.second.offset_size = complex1->offset.type == ComplexItem::Type::REGISTER ? complex1->offset.sign : ((complex1->offset.type == ComplexItem::Type::LABEL || complex1->offset.type == ComplexItem::Type::SUBLABEL) ? 3 : (uint8_t)complex1->offset.data.imm.size);
                info.second.offset_present = complex1->offset.present;
                buffer.Write(current_offset, (uint8_t*)&info, sizeof(ComplexComplexOperandInfo));
                current_offset += sizeof(ComplexComplexOperandInfo);
            }
            else if (operands[0]->type != OperandType::COMPLEX && operands[1]->type != OperandType::COMPLEX) {
                StandardStandardOperandInfo info;
                if (operands[0]->type == OperandType::LABEL || operands[0]->type == OperandType::SUBLABEL) {
                    info.first_type = (uint8_t)OperandType::IMMEDIATE;
                    info.first_size = 3;
                }
                else {
                    info.first_type = (uint8_t)operands[0]->type;
                    info.first_size = (uint8_t)operands[0]->size;
                }
                if (operands[1]->type == OperandType::LABEL || operands[1]->type == OperandType::SUBLABEL) {
                    info.second_type = (uint8_t)OperandType::IMMEDIATE;
                    info.second_size = 3;
                }
                else {
                    info.second_type = (uint8_t)operands[1]->type;
                    info.second_size = (uint8_t)operands[1]->size;
                }
                buffer.Write(current_offset, (uint8_t*)&info, sizeof(StandardStandardOperandInfo));
                current_offset += sizeof(StandardStandardOperandInfo);
            }
            else
                error("Invalid operand combination");
        }
        
        for (uint64_t l = 0; l < instruction->operands.getCount(); l++) {
            Operand* operand = instruction->operands.get(l);
            if (operand->type == OperandType::REGISTER) {
                Register* reg = (Register*)operand->data;
                RegisterID reg_id = GetRegisterID(*reg);
                buffer.Write(current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                current_offset += sizeof(RegisterID);
            }
            else if (operand->type == OperandType::MEMORY) {
                buffer.Write(current_offset, (uint8_t*)operand->data, 8);
                current_offset += 8;
            }
            else if (operand->type == OperandType::IMMEDIATE) {
                switch (operand->size) {
                    case OperandSize::BYTE:
                        buffer.Write(current_offset, (uint8_t*)operand->data, 1);
                        current_offset += 1;
                        break;
                    case OperandSize::WORD:
                        buffer.Write(current_offset, (uint8_t*)operand->data, 2);
                        current_offset += 2;
                        break;
                    case OperandSize::DWORD:
                        buffer.Write(current_offset, (uint8_t*)operand->data, 4);
                        current_offset += 4;
                        break;
                    case OperandSize::QWORD:
                        buffer.Write(current_offset, (uint8_t*)operand->data, 8);
                        current_offset += 8;
                        break;
                    default:
                        error("Invalid operand size");
                        break;
                }
            }
            else if (operand->type == OperandType::COMPLEX) {
                ComplexData* complex = (ComplexData*)operand->data;
                ComplexItem* items[3] = {&complex->base, &complex->index, &complex->offset};
                for (uint8_t i = 0; i < 3; i++) {
                    ComplexItem* item = items[i];
                    if (item->present) {
                        switch (item->type) {
                            case ComplexItem::Type::REGISTER: {
                                Register* reg = (Register*)item->data.reg;
                                RegisterID reg_id = GetRegisterID(*reg);
                                buffer.Write(current_offset, (uint8_t*)&reg_id, sizeof(RegisterID));
                                current_offset += sizeof(RegisterID);
                                break;
                            }
                            case ComplexItem::Type::IMMEDIATE: {
                                switch (item->data.imm.size) {
                                    case OperandSize::BYTE:
                                        buffer.Write(current_offset, (uint8_t*)item->data.imm.data, 1);
                                        current_offset += 1;
                                        break;
                                    case OperandSize::WORD:
                                        buffer.Write(current_offset, (uint8_t*)item->data.imm.data, 2);
                                        current_offset += 2;
                                        break;
                                    case OperandSize::DWORD:
                                        buffer.Write(current_offset, (uint8_t*)item->data.imm.data, 4);
                                        current_offset += 4;
                                        break;
                                    case OperandSize::QWORD:
                                        buffer.Write(current_offset, (uint8_t*)item->data.imm.data, 8);
                                        current_offset += 8;
                                        break;
                                    default:
                                        error("Invalid immediate size");
                                        break;
                                }
                                break;
                            }
                            case ComplexItem::Type::LABEL: {
                                Label* label = (Label*)item->data.label;
                                uint64_t* offset = new uint64_t;
                                *offset = current_offset + global_offset;
                                label->blocks.get(0)->jumps_to_here.insert(offset);
                                uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                                buffer.Write(current_offset, (uint8_t*)&temp_offset, 8);
                                current_offset += 8;
                                break;
                            }
                            case ComplexItem::Type::SUBLABEL: {
                                Block* block = (Block*)item->data.sublabel;
                                uint64_t* offset = new uint64_t;
                                *offset = current_offset + global_offset;
                                block->jumps_to_here.insert(offset);
                                uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                                buffer.Write(current_offset, (uint8_t*)&temp_offset, 8);
                                current_offset += 8;
                                break;
                            }
                            default:
                                error("Invalid complex item type");
                                break;
                        }
                    }
                }
            }
            else if (operand->type == OperandType::LABEL) {
                Label* i_label = (Label*)operand->data;
                Block* i_block = i_label->blocks.get(0);
                uint64_t* offset = new uint64_t;
                *offset = current_offset + global_offset;
                i_block->jumps_to_here.insert(offset);
                uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                buffer.Write(current_offset, (uint8_t*)&temp_offset, 8);
                current_offset += 8;
            }
            else if (operand->type == OperandType::SUBLABEL) {
                Block* i_block = (Block*)operand->data;
                uint64_t* offset = new uint64_t;
                *offset = current_offset + global_offset;
                i_block->jumps_to_here.insert(offset);
                uint64_t temp_offset = 0xDEADBEEFDEADBEEF;
                buffer.Write(current_offset, (uint8_t*)&temp_offset, 8);
                current_offset += 8;
            }
            else {
                error("Invalid operand type");
            }
        }

        buffer.Read(0, data, data_size);
        if (current_offset > data_size)
            error("Data buffer overflow");
        return current_offset;
    }

} // namespace InsEncoding