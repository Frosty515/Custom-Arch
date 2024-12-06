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

#include <atomic>
#include <Emulator.hpp>
#include <Exceptions.hpp>
#include <Interrupts.hpp>
#include <IO/IOBus.hpp>
#include <libarch/Instruction.hpp>
#include <libarch/Operand.hpp>
#include <MMU/MMU.hpp>
#include <Stack.hpp>

#include "InstructionBuffer.hpp"

std::atomic_uchar g_ExecutionAllowed = 1;
std::atomic_uchar g_ExecutionRunning = 1;
std::atomic_uchar g_TerminateExecution = 0;

bool deleted[2] = {true, true};

InsEncoding::Instruction* g_current_instruction = nullptr;

Operand* g_currentOperands[2] = {nullptr, nullptr};

void CleanupCurrentInstruction() {
    if (g_current_instruction == nullptr)
        return;
    const uint64_t count = g_current_instruction->operands.getCount();
    for (uint8_t i = 0; i < count; i++) {
        if (g_currentOperands[i]->GetType() == OperandType::Complex) {
            ComplexData* complex = g_currentOperands[i]->GetComplexData();
            ComplexItem* items[3] = {&complex->base, &complex->index, &complex->offset};
            for (uint8_t j = 0; j < 3; j++) {
                if (items[j]->present && items[j]->type == ComplexItem::Type::IMMEDIATE) {
                    if (items[j]->data.imm.size == OperandSize::BYTE)
                        delete[] static_cast<uint8_t*>(items[j]->data.imm.data);
                    else if (items[j]->data.imm.size == OperandSize::WORD)
                        delete[] static_cast<uint8_t*>(items[j]->data.imm.data);
                    else if (items[j]->data.imm.size == OperandSize::DWORD)
                        delete[] static_cast<uint8_t*>(items[j]->data.imm.data);
                    else if (items[j]->data.imm.size == OperandSize::QWORD)
                        delete[] static_cast<uint8_t*>(items[j]->data.imm.data);
                }
            }
            delete complex;
        }
        // printf("Deleting operand %d\n", i);
        delete g_currentOperands[i];
        deleted[i] = true;
        InsEncoding::Operand* op = g_current_instruction->operands.get(0);
        g_current_instruction->operands.remove(op);
        delete op;
    }
    delete g_current_instruction;
    g_current_instruction = nullptr;
}

void StopExecution() {
    g_TerminateExecution.store(1);
    while (g_ExecutionRunning.load() == 1) {
    }
}

void PauseExecution() {
    g_ExecutionAllowed.store(0);
    while (g_ExecutionRunning.load() == 1) {
    }
}

void AllowExecution() {
    g_TerminateExecution.store(0);
    g_ExecutionAllowed.store(1);
}

bool ExecuteInstruction(uint64_t IP, MMU* mmu, InstructionState& CurrentState, char const*& last_error) {
    if (g_TerminateExecution.load() == 1) {
        g_ExecutionRunning.store(0);
        return false; // completely stop execution
    }
    if (g_ExecutionAllowed.load() == 0 && g_ExecutionRunning.load() != 0) {
        g_ExecutionRunning.store(0);
        return true; // still looping through instructions, just not doing anything
    }
    else if (g_ExecutionRunning.load() == 0)
        g_ExecutionRunning.store(1);
    assert(deleted[0] && deleted[1]);
    (void)CurrentState;
    (void)last_error;
    InstructionBuffer buffer(mmu, IP);
    uint64_t current_offset = 0;
    InsEncoding::Instruction* instruction = InsEncoding::DecodeInstruction(buffer, current_offset);
    if (instruction == nullptr)
        g_ExceptionHandler->RaiseException(Exception::INVALID_INSTRUCTION);
    g_current_instruction = instruction;
    uint8_t Opcode = static_cast<uint8_t>(instruction->GetOpcode());
    Operand* operands[2];
    for (uint8_t i = 0; i < instruction->operands.getCount(); i++) {
        switch (InsEncoding::Operand* op = instruction->operands.get(i); op->type) {
        case InsEncoding::OperandType::REGISTER: {
            InsEncoding::Register* temp_reg = static_cast<InsEncoding::Register*>(op->data);
            Register* reg = Emulator::GetRegisterPointer(static_cast<uint8_t>(*temp_reg));
            delete temp_reg;
            operands[i] = new Operand(static_cast<OperandSize>(op->size), OperandType::Register, reg);
            deleted[i] = false;
            break;
        }
        case InsEncoding::OperandType::IMMEDIATE: {
            uint64_t data;
            switch (op->size) {
            case InsEncoding::OperandSize::BYTE:
                data = *static_cast<uint8_t*>(op->data);
                delete[] static_cast<uint8_t*>(op->data);
                break;
            case InsEncoding::OperandSize::WORD:
                data = *static_cast<uint16_t*>(op->data);
                delete[] static_cast<uint8_t*>(op->data);
                break;
            case InsEncoding::OperandSize::DWORD:
                data = *static_cast<uint32_t*>(op->data);
                delete[] static_cast<uint8_t*>(op->data);
                break;
            case InsEncoding::OperandSize::QWORD:
                data = *static_cast<uint64_t*>(op->data);
                delete[] static_cast<uint8_t*>(op->data);
                break;
            default:
                g_ExceptionHandler->RaiseException(Exception::INVALID_INSTRUCTION);
                break;
            }
            operands[i] = new Operand(static_cast<OperandSize>(op->size), OperandType::Immediate, data);
            deleted[i] = false;
            break;
        }
        case InsEncoding::OperandType::MEMORY: {
            uint64_t* temp = static_cast<uint64_t*>(op->data);
            operands[i] = new Operand(static_cast<OperandSize>(op->size), OperandType::Memory, *temp, Emulator::HandleMemoryOperation);
            delete[] reinterpret_cast<uint8_t*>(temp);
            deleted[i] = false;
            break;
        }
        case InsEncoding::OperandType::COMPLEX: {
            ComplexData* complex = new ComplexData();
            InsEncoding::ComplexData* temp = static_cast<InsEncoding::ComplexData*>(op->data);
            complex->base.present = temp->base.present;
            complex->index.present = temp->index.present;
            complex->offset.present = temp->offset.present;
            if (complex->base.present) {
                if (temp->base.type == InsEncoding::ComplexItem::Type::REGISTER) {
                    InsEncoding::Register* temp_reg = temp->base.data.reg;
                    Register* reg = Emulator::GetRegisterPointer(static_cast<uint8_t>(*temp_reg));
                    delete temp_reg;
                    complex->base.data.reg = reg;
                    complex->base.type = ComplexItem::Type::REGISTER;
                } else {
                    complex->base.data.imm.size = static_cast<OperandSize>(temp->base.data.imm.size);
                    complex->base.data.imm.data = temp->base.data.imm.data;
                    complex->base.type = ComplexItem::Type::IMMEDIATE;
                }
            } else
                complex->base.present = false;
            if (complex->index.present) {
                if (temp->index.type == InsEncoding::ComplexItem::Type::REGISTER) {
                    InsEncoding::Register* temp_reg = temp->index.data.reg;
                    Register* reg = Emulator::GetRegisterPointer(static_cast<uint8_t>(*temp_reg));
                    delete temp_reg;
                    complex->index.data.reg = reg;
                    complex->index.type = ComplexItem::Type::REGISTER;
                } else {
                    complex->index.data.imm.size = static_cast<OperandSize>(temp->index.data.imm.size);
                    complex->index.data.imm.data = temp->index.data.imm.data;
                    complex->index.type = ComplexItem::Type::IMMEDIATE;
                }
            } else
                complex->index.present = false;
            if (complex->offset.present) {
                if (temp->offset.type == InsEncoding::ComplexItem::Type::REGISTER) {
                    InsEncoding::Register* temp_reg = temp->offset.data.reg;
                    Register* reg = Emulator::GetRegisterPointer(static_cast<uint8_t>(*temp_reg));
                    delete temp_reg;
                    complex->offset.data.reg = reg;
                    complex->offset.type = ComplexItem::Type::REGISTER;
                    complex->offset.sign = temp->offset.sign;
                } else {
                    complex->offset.data.imm.size = static_cast<OperandSize>(temp->offset.data.imm.size);
                    complex->offset.data.imm.data = temp->offset.data.imm.data;
                    complex->offset.type = ComplexItem::Type::IMMEDIATE;
                }
            } else
                complex->offset.present = false;
            delete temp;
            operands[i] = new Operand(static_cast<OperandSize>(op->size), OperandType::Complex, complex, Emulator::HandleMemoryOperation);
            deleted[i] = false;
            break;
        }
        default:
            g_ExceptionHandler->RaiseException(Exception::INVALID_INSTRUCTION);
            break;
        }
    }

    g_currentOperands[0] = operands[0];
    g_currentOperands[1] = operands[1];

    // Get the instruction
    uint8_t argument_count = 0;
    void* ins = DecodeOpcode(Opcode, &argument_count);
    if (ins == nullptr)
        g_ExceptionHandler->RaiseException(Exception::INVALID_INSTRUCTION);

    // Increment instruction pointer
    Emulator::SetNextIP(IP + current_offset);

    // Execute the instruction
    if (argument_count == 0)
        reinterpret_cast<void (*)()>(ins)();
    else if (argument_count == 1)
        reinterpret_cast<void (*)(Operand*)>(ins)(operands[0]);
    else if (argument_count == 2)
        reinterpret_cast<void (*)(Operand*, Operand*)>(ins)(operands[0], operands[1]);

    // perform cleanup
    // const uint64_t count = instruction->operands.getCount();
    // for (uint8_t i = 0; i < count; i++) {
    //     if (operands[i]->GetType() == OperandType::Complex) {
    //         ComplexData* complex = (ComplexData*)operands[i]->GetComplexData();
    //         ComplexItem* items[3] = { &complex->base, &complex->index, &complex->offset };
    //         for (uint8_t j = 0; j < 3; j++) {
    //             if (items[j]->present && items[j]->type == ComplexItem::Type::IMMEDIATE) {
    //                 if (items[j]->data.imm.size == OperandSize::BYTE)
    //                     delete[] (uint8_t*)items[j]->data.imm.data;
    //                 else if (items[j]->data.imm.size == OperandSize::WORD)
    //                     delete[] (uint8_t*)items[j]->data.imm.data;
    //                 else if (items[j]->data.imm.size == OperandSize::DWORD)
    //                     delete[] (uint8_t*)items[j]->data.imm.data;
    //                 else if (items[j]->data.imm.size == OperandSize::QWORD)
    //                     delete[] (uint8_t*)items[j]->data.imm.data;
    //             }
    //         }
    //         delete complex;
    //     }
    //     // printf("Deleting operand %d\n", i);
    //     delete operands[i];
    //     deleted[i] = true;
    //     InsEncoding::Operand* op = instruction->operands.get(0);
    //     instruction->operands.remove(op);
    //     delete op;
    // }
    // delete instruction;
    CleanupCurrentInstruction();

    Emulator::SyncRegisters();

    Emulator::SetCPU_IP(Emulator::GetNextIP());
    return true;
}

void ExecutionLoop(MMU* mmu, InstructionState& CurrentState, char const*& last_error) {
    bool status = true;
    while (status)
        status = ExecuteInstruction(Emulator::GetCPU_IP(), mmu, CurrentState, last_error);
}

/*
### Opcode

#### Numbering

- Bits 0-3 are the offset
- Bits 4-6 are the group (0 for ALU, 1 for control flow, 2 for other, 3-7 are reserved)
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
| enteruser | 9 |
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
    switch ((opcode >> 4) & 0x07) {
    case 0: // ALU
        switch (offset) {
        case 0: // add
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_add);
        case 1: // mul
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_mul);
        case 2: // sub
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_sub);
        case 3: // div
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_div);
        case 4: // or
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_or);
        case 5: // xor
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_xor);
        case 6: // nor
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_nor);
        case 7: // and
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_and);
        case 8: // nand
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_nand);
        case 9: // not
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_not);
        case 0xa: // cmp
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_cmp);
        case 0xb: // inc
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_inc);
        case 0xc: // dec
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_dec);
        case 0xd: // shl
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_shl);
        case 0xe: // shr
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_shr);
        default:
            return nullptr;
        }
    case 1: // control flow
        switch (offset) {
        case 0: // ret
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_ret);
        case 1: // call
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_call);
        case 2: // jmp
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_jmp);
        case 3: // jc
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_jc);
        case 4: // jnc
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_jnc);
        case 5: // jz
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_jz);
        case 6: // jnz
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_jnz);
        case 7: // syscall
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_syscall);
        case 8: // sysret
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_sysret);
        case 9: // enteruser
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_enteruser);
        default:
            return nullptr;
        }
    case 2: // other
        switch (offset) {
        case 0: // mov
            if (argument_count != nullptr)
                *argument_count = 2;
            return reinterpret_cast<void*>(ins_mov);
        case 1: // nop
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_nop);
        case 2: // hlt
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_hlt);
        case 3: // push
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_push);
        case 4: // pop
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_pop);
        case 5: // pusha
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_pusha);
        case 6: // popa
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_popa);
        case 7: // int
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_int);
        case 8: // lidt
            if (argument_count != nullptr)
                *argument_count = 1;
            return reinterpret_cast<void*>(ins_lidt);
        case 9: // iret
            if (argument_count != nullptr)
                *argument_count = 0;
            return reinterpret_cast<void*>(ins_iret);
        default:
            return nullptr;
        }
    default: // reserved
        return nullptr;
    }
    return nullptr;
}

extern "C" int printf(const char* __format, ...);

#ifdef EMULATOR_DEBUG
#define PRINT_INS_INFO2(dst, src)                              \
    printf("%s: dst = \"", __extension__ __PRETTY_FUNCTION__); \
    dst->PrintInfo();                                          \
    printf("\", src = \"");                                    \
    src->PrintInfo();                                          \
    printf("\"\n")
#define PRINT_INS_INFO1(dst)                                   \
    printf("%s: dst = \"", __extension__ __PRETTY_FUNCTION__); \
    dst->PrintInfo();                                          \
    printf("\"\n")
#define PRINT_INS_INFO0() printf("%s\n", __extension__ __PRETTY_FUNCTION__)
#else
#define PRINT_INS_INFO2(dst, src)
#define PRINT_INS_INFO1(dst)
#define PRINT_INS_INFO0()
#endif

#ifdef __x86_64__

#include <arch/x86_64/ALUInstruction.h>

#define ALU_INSTRUCTION2(name)                                                  \
    void ins_##name(Operand* dst, Operand* src) {                               \
        PRINT_INS_INFO2(dst, src);                                              \
        uint64_t flags = 0;                                                     \
        dst->SetValue(x86_64_##name(dst->GetValue(), src->GetValue(), &flags)); \
        Emulator::ClearCPUStatus(7);                                            \
        Emulator::SetCPUStatus(flags & 7);                                      \
    }

#define ALU_INSTRUCTION2_NO_RET_VAL(name)                        \
    void ins_##name(Operand* dst, Operand* src) {                \
        PRINT_INS_INFO2(dst, src);                               \
        uint64_t flags = 0;                                      \
        x86_64_##name(dst->GetValue(), src->GetValue(), &flags); \
        Emulator::ClearCPUStatus(7);                             \
        Emulator::SetCPUStatus(flags & 7);                       \
    }

#define ALU_INSTRUCTION1(name)                                 \
    void ins_##name(Operand* dst) {                            \
        PRINT_INS_INFO1(dst);                                  \
        uint64_t flags = 0;                                    \
        dst->SetValue(x86_64_##name(dst->GetValue(), &flags)); \
        Emulator::ClearCPUStatus(7);                           \
        Emulator::SetCPUStatus(flags & 7);                     \
    }

ALU_INSTRUCTION2(add)
ALU_INSTRUCTION2(mul)
ALU_INSTRUCTION2(sub)

void ins_div(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    uint64_t src_val = src->GetValue();
    if (src_val == 0)
        g_ExceptionHandler->RaiseException(Exception::DIV_BY_ZERO);
    uint64_t flags = 0;
    dst->SetValue(x86_64_div(dst->GetValue(), src_val, &flags));
    Emulator::ClearCPUStatus(7);
    Emulator::SetCPUStatus(flags & 7);
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

void ins_add(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() + src->GetValue());
}

void ins_mul(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() * src->GetValue());
}

void ins_sub(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() - src->GetValue());
}

void ins_div(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() / src->GetValue());
}

void ins_or(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() | src->GetValue());
}

void ins_xor(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() ^ src->GetValue());
}

void ins_nor(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(~(dst->GetValue() | src->GetValue()));
}

void ins_and(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() & src->GetValue());
}

void ins_nand(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(~(dst->GetValue() & src->GetValue()));
}

void ins_not(Operand* dst) {
    PRINT_INS_INFO1(dst);
    dst->SetValue(~dst->GetValue());
}

void ins_cmp(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
}

void ins_inc(Operand* dst) {
    PRINT_INS_INFO1(dst);
    dst->SetValue(dst->GetValue() + 1);
}

void ins_dec(Operand* dst) {
    PRINT_INS_INFO1(dst);
    dst->SetValue(dst->GetValue() - 1);
}

void ins_shl(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() << src->GetValue());
}

void ins_shr(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(dst->GetValue() >> src->GetValue());
}

#endif /* __x86_64__ */

void ins_ret() {
    PRINT_INS_INFO0();
    Emulator::SetNextIP(g_stack->pop());
}

void ins_call(Operand* dst) {
    PRINT_INS_INFO1(dst);
    g_stack->push(Emulator::GetNextIP());
    Emulator::SetNextIP(dst->GetValue());
}

void ins_jmp(Operand* dst) {
    PRINT_INS_INFO1(dst);
    Emulator::SetNextIP(dst->GetValue());
}

void ins_jc(Operand* dst) {
    PRINT_INS_INFO1(dst);
    if (uint64_t flags = Emulator::GetCPUStatus(); flags & 1)
        Emulator::SetNextIP(dst->GetValue());
}

void ins_jnc(Operand* dst) {
    PRINT_INS_INFO1(dst);
    if (uint64_t flags = Emulator::GetCPUStatus(); !(flags & 1))
        Emulator::SetNextIP(dst->GetValue());
}

void ins_jz(Operand* dst) {
    PRINT_INS_INFO1(dst);
    if (uint64_t flags = Emulator::GetCPUStatus(); flags & 2)
        Emulator::SetNextIP(dst->GetValue());
}

void ins_jnz(Operand* dst) {
    PRINT_INS_INFO1(dst);
    if (uint64_t flags = Emulator::GetCPUStatus(); !(flags & 2))
        Emulator::SetNextIP(dst->GetValue());
}

void ins_mov(Operand* dst, Operand* src) {
    PRINT_INS_INFO2(dst, src);
    dst->SetValue(src->GetValue());
}

void ins_nop() {
    PRINT_INS_INFO0();
}

void ins_hlt() {
    PRINT_INS_INFO0();
    Emulator::HandleHalt();
}

void ins_push(Operand* src) {
    PRINT_INS_INFO1(src);
    g_stack->push(src->GetValue());
}

void ins_pop(Operand* dst) {
    PRINT_INS_INFO1(dst);
    dst->SetValue(g_stack->pop());
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

void ins_int(Operand* number) {
    PRINT_INS_INFO1(number);
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    uint64_t interrupt = number->GetValue();
    CleanupCurrentInstruction();
    g_InterruptHandler->RaiseInterrupt(interrupt, Emulator::GetNextIP());
}

void ins_lidt(Operand* src) {
    PRINT_INS_INFO1(src);
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    g_InterruptHandler->SetIDTR(src->GetValue());
}

void ins_iret() {
    PRINT_INS_INFO0();
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    CleanupCurrentInstruction();
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

void ins_enteruser(Operand* dst) {
    PRINT_INS_INFO1(dst);
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
    Emulator::EnterUserMode(dst->GetValue());
}