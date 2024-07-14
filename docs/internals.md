# Design

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

### Instruction ID

- 8-bit integer that identifies a specific instruction
- upper 3 bits are type (0 for other, 1 for stack, 2 for ALU, 3 for program flow, 4 for IO, 5-7 are reserved)
- last 5 bits are number

### Argument layout

#### Arg 1 & 2 are registers

- 1-byte arg1 Register ID
- 1-byte arg2 Register ID

#### Arg 1 is register and Arg 2 is memory address

- 1-byte arg1 Register ID
- 8-byte arg2 address

#### Arg 1 is register and Arg 2 is literal

- 1-byte arg1 Register ID
- n-byte arg2 literal

#### Arg 1 is register and Arg 2 is register + offset

- 1-byte arg1 Register ID
- 1-byte arg2 Register ID
- 8-byte arg2 offset

#### Arg 1 is memory address and Arg 2 is Register

- 8-byte arg1 address
- 1-byte arg2 Register ID

#### Arg 1 & 2 are memory addresses

- 8-byte arg1 address
- 8-byte arg2 address

#### Arg 1 is memory address and Arg 2 is literal

- 8-byte arg1 address
- n-byte arg2 literal

#### Arg 1 is memory address and Arg 2 is register + offset

- 1-byte arg1 address
- 1-byte arg2 Register ID
- 8-byte arg2 offset

#### Arg 1 is literal and Arg2 is register

- n-byte arg1 literal
- 1-byte arg2 Register ID

#### Arg 1 is literal and Arg2 is address

- n-byte arg1 literal
- 8-byte arg2 address

#### Arg 1 is literal and Arg2 is literal

- n-byte arg1 literal
- n-byte arg2 literal

#### Arg 1 is literal and Arg2 is register + offset

- n-byte arg1 literal
- 1-byte arg2 Register ID
- 8-byte arg2 offset

#### Arg 1 is register + offset and Arg2 is register

- 1-byte arg1 Register ID
- 8-byte arg1 offset
- 1-byte arg2 Register ID

#### Arg 1 is register + offset and Arg2 is memory address

- 1-byte arg1 Register ID
- 8-byte arg1 offset
- 8-byte arg2 address

#### Arg 1 is register + offset and Arg2 is literal

- 1-byte arg1 Register ID
- 8-byte arg1 offset
- n-byte arg2 literal

#### Arg 1 is register + offset and Arg2 is register + offset

- 1-byte arg1 Register ID
- 8-byte arg1 offset
- 1-byte arg2 Register ID
- 8-byte arg1 offset

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
| (invalid) | 7 |
| (invalid) | 8 |
| (invalid) | 9 |
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
