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

#ifndef _REGISTER_HPP
#define _REGISTER_HPP

#include <stdint.h>

enum class OperandSize;

enum class RegisterType {
    GeneralPurpose,
    Instruction,
    Stack,
    Status,
    Control,
    Unknown // Should never be used, only here for error checking
};

enum RegisterID {
    RegisterID_R0 = 0,
    RegisterID_R1,
    RegisterID_R2,
    RegisterID_R3,
    RegisterID_R4,
    RegisterID_R5,
    RegisterID_R6,
    RegisterID_R7,
    RegisterID_R8,
    RegisterID_R9,
    RegisterID_R10,
    RegisterID_R11,
    RegisterID_R12,
    RegisterID_R13,
    RegisterID_R14,
    RegisterID_R15,
    RegisterID_SCP,
    RegisterID_SBP,
    RegisterID_STP,
    RegisterID_CR0 = 0x20,
    RegisterID_CR1,
    RegisterID_CR2,
    RegisterID_CR3,
    RegisterID_CR4,
    RegisterID_CR5,
    RegisterID_CR6,
    RegisterID_CR7,
    RegisterID_STS,
    RegisterID_IP,
    RegisterID_UNKNOWN = 0xFF
};

class Register {
public:
    Register();
    Register(uint8_t ID, bool writable, uint64_t value = 0);
    Register(RegisterType type, uint8_t index, bool writable, uint64_t value = 0);
    ~Register();

    RegisterType GetType() const;
    uint8_t GetIndex() const;
    uint8_t GetID() const;

    uint64_t GetValue() const;
    uint64_t GetValue(OperandSize size) const;
    bool SetValue(uint64_t value, bool force = false);
    bool SetValue(uint64_t value, OperandSize size);

    Register& operator=(const uint64_t value);
    Register* operator=(const uint64_t* value);

    uint64_t operator=(const Register& reg) const;

    const char* GetName() const;

    void SetDirty(bool dirty);
    bool IsDirty() const;

private:
    void DecodeID(uint8_t ID);

private:
    uint8_t m_index;
    uint8_t m_ID;
    uint64_t m_value;
    RegisterType m_type;
    bool m_dirty;
    bool m_writable;
};

#endif /* _REGISTER_HPP */