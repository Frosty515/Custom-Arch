# Design

## Instructions

First 2 bytes are for the instruction stuff, and data follows it

### Layout

1. 1-byte opcode
2. minimum 1-byte argument info for each argument

### Argument info layout

- bits 0-1 are data type (0 for register, 1 for memory address, 2 for literal, 3 for complex memory address)
- bits 2-3 are the operand size (0 for 8-bit, 1 for 16-bit, 2 for 32-bit, 3 for 64-bit), also the size for literals

#### complex memory address

- bit 4 is the type of the base (0 for register, 1 for literal)
- bit 5-6 is the size of the base (0 for 8-bit, 1 for 16-bit, 2 for 32-bit, 3 for 64-bit) if it is a literal
- bit 7 is set if the base is present
- bits 8 is the type of the index (0 for register, 1 for literal)
- bits 9-10 is the size of the index (0 for 8-bit, 1 for 16-bit, 2 for 32-bit, 3 for 64-bit) if it is a literal
- bit 11 is set if the index is present
- bit 12 is the type of the offset (0 for register, 1 for literal)
- bits 13-14 is the size of the offset (0 for 8-bit, 1 for 16-bit, 2 for 32-bit, 3 for 64-bit) if it is a literal or the sign of the offset if it is a register
- bit 15 is set if the offset is present

### Register ID

- 8-bit integer that identifies a specific register
- first 4 bits are type (0 is general purpose, 1 is stack, 2 is control & flags & instruction pointers)
- last 4 bits are the number

### Register numbers

- General purpose registers: it is simply the GPR number
- Stack: scp is 0, sbp is 1, stp is 2, rest are reserved
- Control, Flags & IPs: CR0-CR7 are numbers 0-7, STS is 8, IP is 9, rest are reserved

### Argument layout

- When there are 2 arguments, one being standard and the other being complex, there are an extra 4-bits of padding after the standard argument.
- On the case where arg 1 is standard, and arg 2 is complex, the padding for arg 1 has a copy of the first 4 bits of the complex argument. Only the arg type matters, the size is ignored.

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
