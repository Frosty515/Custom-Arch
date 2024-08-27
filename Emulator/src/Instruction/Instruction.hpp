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

#ifndef _INSTRUCTION_HPP
#define _INSTRUCTION_HPP

#include "Operand.hpp"

#include <MMU/MMU.hpp>

/*
### Stack



### ALU

- `add`, `mul`, `sub`, `div`, `or`, `xor`, `nor`, `and`, `nand` instructions which support 2 arguments; dst and src, dst can be either a register or address. src can be the same types as dst as well as an immediate. The result of the arithmetic between the 2 values is put in the destination.
- `cmp (address or register), (address, register or immediate)` instruction that compares the values of its to arguments. If its result is successful, the error flag is reset. If it fails, the error flag is set.
- `inc (address or register)` instruction that increments the value in the address or register supplied.
- `dec (address or register)` instruction that decrements the value in the address or register supplied.
- `not (address or register)` instruction that flips all bits in the address or register supplied.
- `shl dst(address or register), n(address, register or immediate)` instruction that shifts dst to the left by n and then returns it in dst.
- `shr dst(address or register), n(address, register or immediate)` instruction that shifts dst to the right by n and then returns it in dst.

### Program flow

- `ret` return from a function
- `call (address or register)` call a function by saving I1 to stack, saving return address in I1 and calling the given address or address in the register
- `jmp (address or register)` jump to address provided or address in register
- `jc (address or register)` jump to address provided or address in register if carry flag is set, otherwise `nop`
- `jnc (address or register)` jump to address provided or address in register if carry flag is not set, otherwise `nop`
- `jz (address or register)` jump to address provided or address in register if zero flag is set, otherwise `nop`
- `jnz (address or register)` jump to address provided or address in register if zero flag is not set, otherwise `nop`

### Misc

- `mov dst(address or register), src(address, register or immediate)` instruction to move values between registers and memory addresses
- `nop` does nothing for that instruction cycle
- `hlt` freeze CPU in its current state
- `push (address or register)` instruction to push 64-bit integers from a memory address or from a general purpose register to the stack
- `pop (address or GPR)` instruction to pop 64-bit integers to a memory address or a  register from the stack
- `pusha` instruction to push all general purpose registers to the stack starting with r15, and finishing with r0
- `popa` instruction to pop all general purpose registers to the stack starting with r0, and finishing with r15
- `int (number)` send interrupt (number)
- `lidt (address or register)` loads Interrupt Descriptor Table
- `iret` instruction which cleans up after interrupt and then calls `ret`

*/


enum class InstructionState {
    OPCODE,
    OPERAND_INFO,
    OPERAND0,
    OPERAND1
};

void ExecuteInstruction(uint64_t IP, MMU& mmu, InstructionState& CurrentState, char const*& last_error);
void ExecutionLoop(MMU& mmu, InstructionState& CurrentState, char const*& last_error);

// return function pointer to instruction based on opcode, output argument count into argument_count if non-null.
void* DecodeOpcode(uint8_t opcode, uint8_t* argument_count);

void ins_add(Operand& dst, Operand& src);
void ins_mul(Operand& dst, Operand& src);
void ins_sub(Operand& dst, Operand& src);
void ins_div(Operand& dst, Operand& src);
void ins_or(Operand& dst, Operand& src);
void ins_xor(Operand& dst, Operand& src);
void ins_nor(Operand& dst, Operand& src);
void ins_and(Operand& dst, Operand& src);
void ins_nand(Operand& dst, Operand& src);
void ins_not(Operand& dst);
void ins_cmp(Operand& a, Operand& b);
void ins_inc(Operand& dst);
void ins_dec(Operand& dst);
void ins_shl(Operand& dst, Operand& src);
void ins_shr(Operand& dst, Operand& src);

void ins_ret();
void ins_call(Operand& dst);
void ins_jmp(Operand& dst);
void ins_jc(Operand& dst);
void ins_jnc(Operand& dst);
void ins_jz(Operand& dst);
void ins_jnz(Operand& dst);

void ins_mov(Operand& dst, Operand& src);
void ins_nop();
void ins_hlt();
void ins_push(Operand& src);
void ins_pop(Operand& dst);
void ins_pusha();
void ins_popa();
void ins_int(Operand& number);
void ins_lidt(Operand& src);
void ins_iret();

void ins_syscall();
void ins_sysret();
void ins_enteruser(Operand& dst);

#endif /* _INSTRUCTION_HPP */