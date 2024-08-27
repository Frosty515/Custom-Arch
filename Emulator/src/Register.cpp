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

#include "Register.hpp"
#include "Emulator.hpp"
#include "Exceptions.hpp"

Register::Register() : m_index(0), m_ID(0xFF), m_value(0), m_type(RegisterType::Unknown), m_dirty(false) {

}

Register::Register(uint8_t ID, bool writable, uint64_t value) : m_index(0), m_ID(ID), m_value(value), m_type(RegisterType::Unknown), m_dirty(false), m_writable(writable) {
    DecodeID(ID);
}

Register::Register(RegisterType type, uint8_t index, bool writable, uint64_t value) : m_index(index), m_ID(0xFF), m_value(value), m_type(type), m_dirty(false), m_writable(writable) {
    switch (type) {
    case RegisterType::GeneralPurpose:
        m_ID = index;
        break;
    case RegisterType::Stack:
        m_ID = 0x10 | index;
        break;
    case RegisterType::Control:
        m_ID = 0x20 | index;
        break;
    case RegisterType::Status:
        m_ID = 0x24;
        break;
    case RegisterType::Instruction:
        m_ID = 0x25;
        break;
    default:
        m_ID = 0xFF;
        break;
    }
}

Register::~Register() {

}

RegisterType Register::GetType() const {
    return m_type;
}

uint8_t Register::GetIndex() const {
    return m_index;
}

uint8_t Register::GetID() const {
    return m_ID;
}

uint64_t Register::GetValue() const {
    if (m_type == RegisterType::Control) {
        if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
            g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    }
    return m_value;
}

bool Register::SetValue(uint64_t value, bool force) {
    if (!force && !m_writable)
        return false;
    if (m_type == RegisterType::Control) {
        if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
            g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    }
    m_value = value;
    m_dirty = true;
    return true;
}

Register& Register::operator=(const uint64_t value) {
    m_value = value;
    m_dirty = true;
    return *this;
}

Register* Register::operator=(const uint64_t* value) {
    m_value = *value;
    m_dirty = true;
    return this;
}

uint64_t Register::operator=(const Register& reg) const {
    return reg.m_value;
}

const char* Register::GetName() const {
    switch (m_type) {
    case RegisterType::GeneralPurpose:
        switch (m_index) {
        case 0:
            return "R0";
        case 1:
            return "R1";
        case 2:
            return "R2";
        case 3:
            return "R3";
        case 4:
            return "R4";
        case 5:
            return "R5";
        case 6:
            return "R6";
        case 7:
            return "R7";
        case 8:
            return "R8";
        case 9:
            return "R9";
        case 10:
            return "R10";
        case 11:
            return "R11";
        case 12:
            return "R12";
        case 13:
            return "R13";
        case 14:
            return "R14";
        case 15:
            return "R15";
        default:
            return "Unknown";
        }
    case RegisterType::Stack:
        switch (m_index) {
        case 0:
            return "SCP";
        case 1:
            return "SBP";
        case 2:
            return "STP";
        default:
            return "Unknown";
        }
    case RegisterType::Control:
        switch (m_index) {
        case 0:
            return "CR0";
        case 1:
            return "CR1";
        case 2:
            return "CR2";
        case 3:
            return "CR3";
        case 4:
            return "CR4";
        case 5:
            return "CR5";
        case 6:
            return "CR6";
        case 7:
            return "CR7";
        default:
            return "Unknown";
        }
    case RegisterType::Status:
        return "STS";
    case RegisterType::Instruction:
        return "IP";
    default:
        return "Unknown";
    }
}

void Register::SetDirty(bool dirty) {
    m_dirty = dirty;
}

bool Register::IsDirty() const {
    return m_dirty;
}

void Register::DecodeID(uint8_t ID) {
    uint8_t type = ID >> 4;
    uint8_t index = ID & 0x0F;
    m_type = RegisterType::Unknown;
    switch (type) {
    case 0:
        m_type = RegisterType::GeneralPurpose;
        m_index = index;
        break;
    case 1:
        m_type = RegisterType::Stack;
        m_index = index;
        break;
    case 2:
        if (index < 8) {
            m_type = RegisterType::Control;
            m_index = index;
        }
        else if (index == 8) {
            m_type = RegisterType::Status;
            m_index = 0;
        }
        else if (index == 9) {
            m_type = RegisterType::Instruction;
            m_index = 0;
        }
        break;
    }
}