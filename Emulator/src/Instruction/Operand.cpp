/*
Copyright (©) 2023-2024  Frosty515

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

#include "Operand.hpp"

#include <stdarg.h>
#include <stdio.h>

Operand::Operand() : m_register(nullptr), m_type(OperandType::Register), m_size(OperandSize::Unknown), m_offset(0), m_address(0), m_memoryOperation(nullptr) {

}

Operand::Operand(OperandSize size, OperandType type, ...) : m_register(nullptr), m_type(type), m_size(size), m_offset(0), m_address(0), m_memoryOperation(nullptr) {
    va_list args;
    va_start(args, type);
    switch (type) {
    case OperandType::Register:
        m_register = va_arg(args, Register*);
        break;
    case OperandType::Immediate:
        m_offset = va_arg(args, uint64_t);
        break;
    case OperandType::Memory:
        m_address = va_arg(args, uint64_t);
        m_memoryOperation = va_arg(args, MemoryOperation_t);
        break;
    case OperandType::RegisterOffset:
        m_register = va_arg(args, Register*);
        m_offset = va_arg(args, uint64_t);
        m_memoryOperation = va_arg(args, MemoryOperation_t);
        break;
    }
    va_end(args);
}

Operand::~Operand() {

}

Register* Operand::GetRegister() {
    return m_register;
}

OperandType Operand::GetType() const {
    return m_type;
}

OperandSize Operand::GetSize() const {
    return m_size;
}

uint64_t Operand::GetOffset() const {
    return m_offset;
}

uint64_t Operand::GetAddress() const {
    return m_address;
}

void Operand::PrintInfo() const {
    switch (m_type) {
    case OperandType::Register:
        printf("Register: %s", m_register->GetName());
        break;
    case OperandType::Immediate: {
        char const* size;
        switch (m_size) {
        case OperandSize::BYTE:
            size = "BYTE";
            break;
        case OperandSize::WORD:
            size = "WORD";
            break;
        case OperandSize::DWORD:
            size = "DWORD";
            break;
        case OperandSize::QWORD:
            size = "QWORD";
            break;
        default:
            size = "Unknown";
            break;
        }
        printf("Immediate: value = %#lx, size = %s", m_offset, size);
        break;
    }
    case OperandType::Memory:
        printf("Memory: %#016lx", m_address);
        break;
    case OperandType::RegisterOffset:
        printf("Register: %s, Offset: %#016lx", m_register->GetName(), m_offset);
        break;
    }
}

uint64_t Operand::GetValue() const {
    switch (m_type) {
    case OperandType::Register:
        return m_register->GetValue();
    case OperandType::Immediate:
        return m_offset;
    case OperandType::Memory: {
        uint64_t value = 0;
        m_memoryOperation(m_address, &value, sizeof(uint64_t), 1, false);
        return value;
    }
    case OperandType::RegisterOffset: {
        uint64_t value = 0;
        m_memoryOperation(m_register->GetValue() + (int64_t)m_offset, &value, sizeof(uint64_t), 1, false);
        return value;
    }
    default:
        return 0;
    }
}

void Operand::SetValue(uint64_t value) {
    switch (m_type) {
    case OperandType::Register:
        if (m_register->GetType() == RegisterType::Flags || m_register->GetType() == RegisterType::Instruction) // writes are not allowed on these registers
            return;
        m_register->SetValue(value);
        break;
    case OperandType::Immediate:
        m_offset = value;
        break;
    case OperandType::Memory:
        m_memoryOperation(m_address, &value, sizeof(uint64_t), 1, true);
        break;
    case OperandType::RegisterOffset:
        m_memoryOperation(m_register->GetValue() + (int64_t)m_offset, &value, sizeof(uint64_t), 1, true);
        break;
    }
}
