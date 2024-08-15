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
#include "Interrupts.hpp"

#include <Emulator.hpp>
#include <Exceptions.hpp>
#include <Stack.hpp>

#include <IO/IOBus.hpp>

#include <MMU/MMU.hpp>


void ExecuteInstruction(uint64_t IP, MMU& mmu, InstructionState& CurrentState, char const*& last_error) {
    uint8_t Opcode = 0xff;

    Operand operands[2];

    OperandType operand_types[2];

    OperandSize operand_sizes[2];

    size_t operand_count = 0;

    CurrentState = InstructionState::OPCODE;
    Opcode = mmu.read8(IP);

    CurrentState = InstructionState::OPERAND_INFO;
    uint8_t operand_info = mmu.read8(IP + 1);
    if ((operand_info & 3) != 2 && ((operand_info >> 2) & 3) > 0) {
        operand_count = 0;
    }
    else if (((operand_info >> 4) & 3) != 2 && ((operand_info >> 6) & 3) > 0) {
        operand_count = 1;
    }
    else
        operand_count = 2;
    if (operand_count > 0) {
        switch (operand_info & 3) {
        case 0:
            operand_types[0] = OperandType::Register;
            break;
        case 1:
            operand_types[0] = OperandType::Memory;
            break;
        case 2:
            operand_types[0] = OperandType::Immediate;
            break;
        case 3:
            operand_types[0] = OperandType::RegisterOffset;
            break;
        }
        switch ((operand_info >> 2) & 3) {
        case 0:
            operand_sizes[0] = OperandSize::BYTE;
            break;
        case 1:
            operand_sizes[0] = OperandSize::WORD;
            break;
        case 2:
            operand_sizes[0] = OperandSize::DWORD;
            break;
        case 3:
            operand_sizes[0] = OperandSize::QWORD;
            break;
        }
        if (operand_count > 1) {
            switch ((operand_info >> 4) & 3) {
            case 0:
                operand_types[1] = OperandType::Register;
                break;
            case 1:
                operand_types[1] = OperandType::Memory;
                break;
            case 2:
                operand_types[1] = OperandType::Immediate;
                break;
            case 3:
                operand_types[1] = OperandType::RegisterOffset;
                break;
            }
            switch ((operand_info >> 6) & 3) {
            case 0:
                operand_sizes[1] = OperandSize::BYTE;
                break;
            case 1:
                operand_sizes[1] = OperandSize::WORD;
                break;
            case 2:
                operand_sizes[1] = OperandSize::DWORD;
                break;
            case 3:
                operand_sizes[1] = OperandSize::QWORD;
                break;
            }
        }
    }

    uint64_t operand_offset = 0;

    if (operand_count > 0) {
        CurrentState = InstructionState::OPERAND0;
        switch (operand_types[0]) {
        case OperandType::Register:
            operands[0] = Operand(operand_sizes[0], operand_types[0], Emulator::GetRegisterPointer(mmu.read8(IP + 2)));
            operand_offset++;
            break;
        case OperandType::Memory:
            operands[0] = Operand(operand_sizes[0], operand_types[0], mmu.read64(IP + 2), Emulator::HandleMemoryOperation);
            operand_offset += 8;
            break;
        case OperandType::Immediate: {
            uint64_t data;
            switch (operand_sizes[0]) {
            case OperandSize::BYTE:
                data = mmu.read8(IP + 2);
                operand_offset++;
                break;
            case OperandSize::WORD:
                data = mmu.read16(IP + 2);
                operand_offset += 2;
                break;
            case OperandSize::DWORD:
                data = mmu.read32(IP + 2);
                operand_offset += 4;
                break;
            case OperandSize::QWORD:
                data = mmu.read64(IP + 2);
                operand_offset += 8;
                break;
            case OperandSize::Unknown:
                last_error = "Unknown operand0 size";
                return;
            }
            operands[0] = Operand(operand_sizes[0], operand_types[0], data);
            break;
        }
        case OperandType::RegisterOffset:
            operands[0] = Operand(operand_sizes[0], operand_types[0], Emulator::GetRegisterPointer(mmu.read8(IP + 2)), mmu.read64(IP + 3), Emulator::HandleMemoryOperation);
            operand_offset += 9;
            break;
        }
        if (operand_count > 1) {
            CurrentState = InstructionState::OPERAND1;
            switch (operand_types[1]) {
            case OperandType::Register:
                operands[1] = Operand(operand_sizes[1], operand_types[1], Emulator::GetRegisterPointer(mmu.read8(IP + 2 + operand_offset)));
                operand_offset++;
                break;
            case OperandType::Memory:
                operands[1] = Operand(operand_sizes[1], operand_types[1], mmu.read64(IP + 2 + operand_offset), Emulator::HandleMemoryOperation);
                operand_offset += 8;
                break;
            case OperandType::Immediate: {
                uint64_t data;
                switch (operand_sizes[1]) {
                case OperandSize::BYTE:
                    data = mmu.read8(IP + 2 + operand_offset);
                    operand_offset++;
                    break;
                case OperandSize::WORD:
                    data = mmu.read16(IP + 2 + operand_offset);
                    operand_offset += 2;
                    break;
                case OperandSize::DWORD:
                    data = mmu.read32(IP + 2 + operand_offset);
                    operand_offset += 4;
                    break;
                case OperandSize::QWORD:
                    data = mmu.read64(IP + 2 + operand_offset);
                    operand_offset += 8;
                    break;
                case OperandSize::Unknown:
                    last_error = "Unknown operand1 size";
                    return;
                }
                operands[1] = Operand(operand_sizes[1], operand_types[1], data);
                break;
            }
            case OperandType::RegisterOffset:
                operands[1] = Operand(operand_sizes[1], operand_types[1], Emulator::GetRegisterPointer(mmu.read8(IP + 2 + operand_offset)), mmu.read64(IP + 3 + operand_offset), Emulator::HandleMemoryOperation);
                operand_offset += 9;
                break;
            }
            
        }
    }
    // Get the instruction
    uint8_t argument_count = 0;
    void* ins = DecodeOpcode(Opcode, &argument_count);
    if (ins == nullptr)
        g_ExceptionHandler->RaiseException(Exception::INVALID_INSTRUCTION);

    // Increment instruction pointer
    Emulator::SetNextIP(IP + 2 + operand_offset);

    // Execute the instruction
    if (argument_count == 0)
        ((void(*)(void))ins)();
    else if (argument_count == 1)
        ((void(*)(Operand&))ins)(operands[0]);
    else if (argument_count == 2)
        ((void(*)(Operand&, Operand&))ins)(operands[0], operands[1]);

    Emulator::SyncRegisters();

    Emulator::SetCPU_IP(Emulator::GetNextIP());
    return ExecuteInstruction(Emulator::GetCPU_IP(), mmu, CurrentState, last_error);
}

/*
### Opcode

#### Numbering

- Bits 0-3 are the offset
- Bits 4-6 are the group (0 for ALU, 1 for control flow, 2 for IO, 3 for other, 4-7 are reserved)
- Bit 7 is reserved and should always be 0

#### ALU

| Name | offset |
| ---- | --- |
| add  | 0 |
| mul  | 1 |
| sub  | 2 |
| div  | 3 |
| or   | 4 |
| xor  | 5 |
| nor  | 6 |
| and  | 7 |
| nand | 8 |
| not  | 9 |
| cmp  | a |
| inc  | b |
| dec  | c |
| shl  | d |
| shr  | e |
| (invalid)  | f |

#### Control flow

| Name | offset |
| ---- | --- |
| ret  | 0 |
| call | 1 |
| jmp  | 2 |
| jc   | 3 |
| jnc  | 4 |
| jz   | 5 |
| jnz  | 6 |
| syscall | 7 |
| sysret | 8 |
| enteruse | 9 |
| (invalid) | a |
| (invalid) | b |
| (invalid) | c |
| (invalid) | d |
| (invalid) | e |
| (invalid) | f |

#### IO

| Name | offset |
| ---- | --- |
| inb  | 0 |
| outb | 1 |
| inw  | 2 |
| outw | 3 |
| ind  | 4 |
| outd | 5 |
| inq  | 6 |
| outq | 7 |
| (invalid) | 8 |
| (invalid) | 9 |
| (invalid) | a |
| (invalid) | b |
| (invalid) | c |
| (invalid) | d |
| (invalid) | e |
| (invalid) | f |

#### Other

| Name | offset |
| ---- | --- |
| mov  | 0 |
| nop  | 1 |
| hlt  | 2 |
| push  | 3 |
| pop | 4 |
| pusha | 5 |
| popa | 6 |
| int | 7 |
| lidt | 8 |
| iret | 9 |
| (invalid) | a |
| (invalid) | b |
| (invalid) | c |
| (invalid) | d |
| (invalid) | e |
| (invalid) | f |
*/

void* DecodeOpcode(uint8_t opcode, uint8_t* argument_count) {
    uint8_t offset = opcode & 0x0f;
    uint8_t group = (opcode >> 4) & 0x07;
    switch (group) {
    case 0: // ALU
        switch (offset) {
        case 0: // add
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_add;
        case 1: // mul
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_mul;
        case 2: // sub
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_sub;
        case 3: // div
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_div;
        case 4: // or
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_or;
        case 5: // xor
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_xor;
        case 6: // nor
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_nor;
        case 7: // and
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_and;
        case 8: // nand
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_nand;
        case 9: // not
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_not;
        case 0xa: // cmp
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_cmp;
        case 0xb: // inc
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_inc;
        case 0xc: // dec
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_dec;
        case 0xd: // shl
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_shl;
        case 0xe: // shr
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_shr;
        default:
            return nullptr;
        }
    case 1: // control flow
        switch (offset) {
        case 0: // ret
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_ret;
        case 1: // call
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_call;
        case 2: // jmp
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_jmp;
        case 3: // jc
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_jc;
        case 4: // jnc
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_jnc;
        case 5: // jz
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_jz;
        case 6: // jnz
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_jnz;
        case 7: // syscall
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_syscall;
        case 8: // sysret
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_sysret;
        case 9: // enteruser
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_enteruser;
        default:
            return nullptr;
        }
    case 2: // IO
        switch (offset) {
        case 0: // inb
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_inb;
        case 1: // outb
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_outb;
        case 2: // inw
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_inw;
        case 3: // outw
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_outw;
        case 4: // ind
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_ind;
        case 5: // outd
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_outd;
        case 6: // inq
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_inq;
        case 7: // outq
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_outq;
        default:
            return nullptr;
        }
    case 3: // other
        switch (offset) {
        case 0: // mov
            if (argument_count != nullptr)
                *argument_count = 2;
            return (void*)ins_mov;
        case 1: // nop
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_nop;
        case 2: // hlt
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_hlt;
        case 3: // push
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_push;
        case 4: // pop
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_pop;
        case 5: // pusha
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_pusha;
        case 6: // popa
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_popa;
        case 7: // int
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_int;
        case 8: // lidt
            if (argument_count != nullptr)
                *argument_count = 1;
            return (void*)ins_lidt;
        case 9: // iret
            if (argument_count != nullptr)
                *argument_count = 0;
            return (void*)ins_iret;
        default:
            return nullptr;
        }
    }
    return nullptr;
}

extern "C" int printf(const char* format, ...);

#ifdef EMULATOR_DEBUG
#define PRINT_INS_INFO2(dst, src) printf("%s: dst = \"", __extension__ __PRETTY_FUNCTION__); dst.PrintInfo(); printf("\", src = \""); src.PrintInfo(); printf("\"\n")
#define PRINT_INS_INFO1(dst) printf("%s: dst = \"", __extension__ __PRETTY_FUNCTION__); dst.PrintInfo(); printf("\"\n")
#define PRINT_INS_INFO0() printf("%s\n", __extension__ __PRETTY_FUNCTION__)
#else
#define PRINT_INS_INFO2(dst, src)
#define PRINT_INS_INFO1(dst)
#define PRINT_INS_INFO0()
#endif

#ifdef __x86_64__

#include <arch/x86_64/ALUInstruction.h>

#define ALU_INSTRUCTION2(name) void ins_##name(Operand& dst, Operand& src) { \
    PRINT_INS_INFO2(dst, src); \
    uint64_t flags = 0; \
    dst.SetValue(x86_64_##name(dst.GetValue(), src.GetValue(), &flags)); \
    Emulator::ClearCPUFlags(7); \
    Emulator::SetCPUFlags(flags & 7); \
}

#define ALU_INSTRUCTION2_NO_RET_VAL(name) void ins_##name(Operand& dst, Operand& src) { \
    PRINT_INS_INFO2(dst, src); \
    uint64_t flags = 0; \
    x86_64_##name(dst.GetValue(), src.GetValue(), &flags); \
    Emulator::ClearCPUFlags(7); \
    Emulator::SetCPUFlags(flags & 7); \
}

#define ALU_INSTRUCTION1(name) void ins_##name(Operand& dst) { \
    PRINT_INS_INFO1(dst); \
    uint64_t flags = 0; \
    dst.SetValue(x86_64_##name(dst.GetValue(), &flags)); \
    Emulator::ClearCPUFlags(7); \
    Emulator::SetCPUFlags(flags & 7); \
}

ALU_INSTRUCTION2(add)
ALU_INSTRUCTION2(mul)
ALU_INSTRUCTION2(sub)

void ins_div(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    uint64_t src_val = src.GetValue();
    if (src_val == 0)
        g_ExceptionHandler->RaiseException(Exception::DIV_BY_ZERO);
    uint64_t flags = 0;
    dst.SetValue(x86_64_div(dst.GetValue(), src_val, &flags));
    Emulator::ClearCPUFlags(7);
    Emulator::SetCPUFlags(flags & 7);
}

ALU_INSTRUCTION2(or)
ALU_INSTRUCTION2(xor)
ALU_INSTRUCTION2(nor)
ALU_INSTRUCTION2(and)
ALU_INSTRUCTION2(nand)
ALU_INSTRUCTION1(not)
ALU_INSTRUCTION2(shl)
ALU_INSTRUCTION2(shr)
ALU_INSTRUCTION2_NO_RET_VAL(cmp)
ALU_INSTRUCTION1(inc)
ALU_INSTRUCTION1(dec)

#else /* __x86_64__ */

void ins_add(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() + src.GetValue());
}

void ins_mul(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() * src.GetValue());
}

void ins_sub(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() - src.GetValue());
}

void ins_div(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() / src.GetValue());
}

void ins_or(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() | src.GetValue());
}

void ins_xor(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() ^ src.GetValue());
}

void ins_nor(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(~(dst.GetValue() | src.GetValue()));
}

void ins_and(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() & src.GetValue());
}

void ins_nand(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(~(dst.GetValue() & src.GetValue()));
}

void ins_not(Operand& dst) {
    PRINT_INS_INFO1(dst);
    dst.SetValue(~dst.GetValue());
}

void ins_cmp(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);

}

void ins_inc(Operand& dst) {
    PRINT_INS_INFO1(dst);
    dst.SetValue(dst.GetValue() + 1);
}

void ins_dec(Operand& dst) {
    PRINT_INS_INFO1(dst);
    dst.SetValue(dst.GetValue() - 1);
}

void ins_shl(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() << src.GetValue());
}

void ins_shr(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(dst.GetValue() >> src.GetValue());
}

#endif /* __x86_64__ */

void ins_ret() {
    PRINT_INS_INFO0();
    Emulator::SetNextIP(g_stack->pop());
}

void ins_call(Operand& dst) {
    PRINT_INS_INFO1(dst);
    g_stack->push(Emulator::GetNextIP());
    Emulator::SetNextIP(dst.GetValue());
}

void ins_jmp(Operand& dst) {
    PRINT_INS_INFO1(dst);
    Emulator::SetNextIP(dst.GetValue());
}

void ins_jc(Operand& dst) {
    PRINT_INS_INFO1(dst);
    uint64_t flags = Emulator::GetCPUFlags();
    if (flags & 1)
        Emulator::SetNextIP(dst.GetValue());
}

void ins_jnc(Operand& dst) {
    PRINT_INS_INFO1(dst);
    uint64_t flags = Emulator::GetCPUFlags();
    if (!(flags & 1))
        Emulator::SetNextIP(dst.GetValue());
}

void ins_jz(Operand& dst) {
    PRINT_INS_INFO1(dst);
    uint64_t flags = Emulator::GetCPUFlags();
    if (flags & 2)
        Emulator::SetNextIP(dst.GetValue());
}

void ins_jnz(Operand& dst) {
    PRINT_INS_INFO1(dst);
    uint64_t flags = Emulator::GetCPUFlags();
    if (!(flags & 2))
        Emulator::SetNextIP(dst.GetValue());
}

void ins_inb(Operand& port, Operand& out) {
    PRINT_INS_INFO2(port, out);
    out.SetValue(g_IOBus->ReadByte(port.GetValue()));
}

void ins_outb(Operand& port, Operand& byte) {
    PRINT_INS_INFO2(port, byte);
    g_IOBus->WriteByte(port.GetValue(), byte.GetValue());
}

void ins_inw(Operand& port, Operand& out) {
    PRINT_INS_INFO2(port, out);
    out.SetValue(g_IOBus->ReadWord(port.GetValue()));
}

void ins_outw(Operand& port, Operand& word) {
    PRINT_INS_INFO2(port, word);
    g_IOBus->WriteWord(port.GetValue(), word.GetValue());
}

void ins_ind(Operand& port, Operand& out) {
    PRINT_INS_INFO2(port, out);
    out.SetValue(g_IOBus->ReadDWord(port.GetValue()));
}

void ins_outd(Operand& port, Operand& dword) {
    PRINT_INS_INFO2(port, dword);
    g_IOBus->WriteDWord(port.GetValue(), dword.GetValue());
}

void ins_inq(Operand& port, Operand& out) {
    PRINT_INS_INFO2(port, out);
    out.SetValue(g_IOBus->ReadQWord(port.GetValue()));
}

void ins_outq(Operand& port, Operand& qword) {
    PRINT_INS_INFO2(port, qword);
    g_IOBus->WriteQWord(port.GetValue(), qword.GetValue());
}

void ins_mov(Operand& dst, Operand& src) {
    PRINT_INS_INFO2(dst, src);
    dst.SetValue(src.GetValue());
}

void ins_nop() {
    PRINT_INS_INFO0();
}

void ins_hlt() {
    PRINT_INS_INFO0();
    Emulator::HandleHalt();
}

void ins_push(Operand& src) {
    PRINT_INS_INFO1(src);
    g_stack->push(src.GetValue());
}

void ins_pop(Operand& dst) {
    PRINT_INS_INFO1(dst);
    dst.SetValue(g_stack->pop());
}

void ins_pusha() {
    PRINT_INS_INFO0();
    Register* r0 = Emulator::GetRegisterPointer(RegisterID_R0);
    Register* r1 = Emulator::GetRegisterPointer(RegisterID_R1);
    Register* r2 = Emulator::GetRegisterPointer(RegisterID_R2);
    Register* r3 = Emulator::GetRegisterPointer(RegisterID_R3);
    Register* r4 = Emulator::GetRegisterPointer(RegisterID_R4);
    Register* r5 = Emulator::GetRegisterPointer(RegisterID_R5);
    Register* r6 = Emulator::GetRegisterPointer(RegisterID_R6);
    Register* r7 = Emulator::GetRegisterPointer(RegisterID_R7);
    Register* r8 = Emulator::GetRegisterPointer(RegisterID_R8);
    Register* r9 = Emulator::GetRegisterPointer(RegisterID_R9);
    Register* r10 = Emulator::GetRegisterPointer(RegisterID_R10);
    Register* r11 = Emulator::GetRegisterPointer(RegisterID_R11);
    Register* r12 = Emulator::GetRegisterPointer(RegisterID_R12);
    Register* r13 = Emulator::GetRegisterPointer(RegisterID_R13);
    Register* r14 = Emulator::GetRegisterPointer(RegisterID_R14);
    Register* r15 = Emulator::GetRegisterPointer(RegisterID_R15);
    g_stack->push(r0->GetValue());
    g_stack->push(r1->GetValue());
    g_stack->push(r2->GetValue());
    g_stack->push(r3->GetValue());
    g_stack->push(r4->GetValue());
    g_stack->push(r5->GetValue());
    g_stack->push(r6->GetValue());
    g_stack->push(r7->GetValue());
    g_stack->push(r8->GetValue());
    g_stack->push(r9->GetValue());
    g_stack->push(r10->GetValue());
    g_stack->push(r11->GetValue());
    g_stack->push(r12->GetValue());
    g_stack->push(r13->GetValue());
    g_stack->push(r14->GetValue());
    g_stack->push(r15->GetValue());
}

void ins_popa() {
    PRINT_INS_INFO0();
    Register* r0 = Emulator::GetRegisterPointer(RegisterID_R0);
    Register* r1 = Emulator::GetRegisterPointer(RegisterID_R1);
    Register* r2 = Emulator::GetRegisterPointer(RegisterID_R2);
    Register* r3 = Emulator::GetRegisterPointer(RegisterID_R3);
    Register* r4 = Emulator::GetRegisterPointer(RegisterID_R4);
    Register* r5 = Emulator::GetRegisterPointer(RegisterID_R5);
    Register* r6 = Emulator::GetRegisterPointer(RegisterID_R6);
    Register* r7 = Emulator::GetRegisterPointer(RegisterID_R7);
    Register* r8 = Emulator::GetRegisterPointer(RegisterID_R8);
    Register* r9 = Emulator::GetRegisterPointer(RegisterID_R9);
    Register* r10 = Emulator::GetRegisterPointer(RegisterID_R10);
    Register* r11 = Emulator::GetRegisterPointer(RegisterID_R11);
    Register* r12 = Emulator::GetRegisterPointer(RegisterID_R12);
    Register* r13 = Emulator::GetRegisterPointer(RegisterID_R13);
    Register* r14 = Emulator::GetRegisterPointer(RegisterID_R14);
    Register* r15 = Emulator::GetRegisterPointer(RegisterID_R15);
    r15->SetValue(g_stack->pop());
    r14->SetValue(g_stack->pop());
    r13->SetValue(g_stack->pop());
    r12->SetValue(g_stack->pop());
    r11->SetValue(g_stack->pop());
    r10->SetValue(g_stack->pop());
    r9->SetValue(g_stack->pop());
    r8->SetValue(g_stack->pop());
    r7->SetValue(g_stack->pop());
    r6->SetValue(g_stack->pop());
    r5->SetValue(g_stack->pop());
    r4->SetValue(g_stack->pop());
    r3->SetValue(g_stack->pop());
    r2->SetValue(g_stack->pop());
    r1->SetValue(g_stack->pop());
    r0->SetValue(g_stack->pop());
}

void ins_int(Operand& number) {
    PRINT_INS_INFO1(number);
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    g_InterruptHandler->RaiseInterrupt(number.GetValue(), Emulator::GetNextIP());
}

void ins_lidt(Operand& src) {
    PRINT_INS_INFO1(src);
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    g_InterruptHandler->SetIDTR(src.GetValue());
}

void ins_iret() {
    PRINT_INS_INFO0();
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    g_InterruptHandler->ReturnFromInterrupt();
}

void ins_syscall() {
    PRINT_INS_INFO0();
    if (Emulator::isInProtectedMode() && !Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::SUPERVISOR_MODE_VIOLATION);
    Emulator::ExitUserMode();
}

void ins_sysret() {
    PRINT_INS_INFO0();
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    Emulator::EnterUserMode();
}

void ins_enteruser(Operand &dst) {
    PRINT_INS_INFO1(dst);
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    Emulator::EnterUserMode(dst.GetValue());
}