# Custom Architecture

## Registers

- 16 64-bit general purpose registers (GPRs) named r0-r15
- 3 64-bit stack related registers. See [Stack](#stack-info)
- 1 64-bit flags register that is read-only. named FLAGS
- 4 64-bit control registers named CR0-CR3, currently all reserved
- 2 64-bit instruction registers, I0 for current address and I1 for previous frames address (return address). I0 and I1 are read-only.

## Stack Info

- grows upwards.
- register `sbp` for base of the stack address (index 0)
- register `scp` for current stack address (index 1)
- register `stp` for top of the stack address (index 2)

## Calling convention info

### On call

#### Prior to call instruction

1. registers r0-r7 are saved to the stack (starting with r0, finishing with r7) if their values are of importance
2. arguments are placed on the stack right to left

#### Performed during the call instruction

1. I1 is placed on the stack
2. return address is placed in I1
3. function is jumped to

#### At start of new function

1. sbp is saved to the stack
2. scp is saved in sbp

### On return

#### Prior to ret instruction

1. any return value is in r0 and optionally r1 as well. (low 64-bits are in r0, high 64-bits are in r1)
2. scp is restored from sbp
3. sbp is popped off the stack

#### Performed during ret instruction

1. Value in I1 is moved to I0, but not executed yet
2. I1 is restored from the stack
3. CPU continues executing

#### After ret instruction

1. arguments are removed from stack
2. register r0-r7 are restored from the stack (starting with r7, finishing with r0) if their values were saved

## Operation modes

- 1 mode: 64-bit real mode

### 64-bit real mode

- All memory is accessible by everything
- All registers are accessible by everything (I0 is always read-only, I1 is always intended to be read-only, FLAGS is read-only)
- All valid instructions always work

## Instructions

### Stack

- `push (address or register)` instruction to push 64-bit integers from a memory address or from a general purpose register to the stack
- `pop (address or GPR)` instruction to pop 64-bit integers to a memory address or a  register from the stack
- `pusha` instruction to push all general purpose registers to the stack starting with r15, and finishing with r0
- `popa` instruction to pop all general purpose registers to the stack starting with r0, and finishing with r15

### ALU

- `add`, `mul`, `sub`, `div`, `or`, `xor`, `nor`, `and`, `nand`, `not` instructions which support 2 arguments; dst and src, dst can be either a register or address. src can be the same types as dst as well as an immediate. The result of the arithmetic between the 2 values is put in the destination.
- `cmp (address or register), (address, register or immediate)` instruction that compares the values of its to arguments. If its result is successful, the error flag is reset. If it fails, the error flag is set.
- `inc (address or register)` instruction that increments the value in the address or register supplied.
- `dec (address or register)` instruction that decrements the value in the address or register supplied.
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

### IO

- `inb port(address, register or immediate), out(address or register)` receive byte from port and put it in out
- `outb port(address, register or immediate), byte(address, register or immediate)` send byte to port
- `inw port(address, register or immediate), out(address or register)` receive word from port and put it in out
- `outw port(address, register or immediate), word(address, register or immediate)` send word to port
- `ind port(address, register or immediate), out(address or register)` receive double-word from port and put it in out
- `outd port(address, register or immediate), double-word(address, register or immediate)` send double-word to port
- `inq port(address, register or immediate), out(address or register)` receive quad-word from port and put it in out
- `outq port(address, register or immediate), quad-word(address, register or immediate)` send quad-word to port

### Interrupts

- `int (number)` send interrupt (number)
- `lidt (address or register)` loads Interrupt Descriptor Table
- `iret` instruction which cleans up after interrupt and then calls `ret`

### Other

- `mov dst(address or register), src(address, register or immediate)` instruction to move values between registers and memory addresses
- `nop` does nothing for that instruction cycle
- `hlt` freeze CPU in its current state

## Interrupts info

- has a register called IDTR which contains the address of a table called an IDT.
- IDT might look like this below in C:

```c
struct IDT {
    struct IDTEntry Entries[256]; // array of IDTEntries
};
```

- An IDT Entry might look like this in C:

```c
struct IDTEntry {
    unsigned char Present : 1;
    unsigned char Flags : 7; // currently reserved
    unsigned long int Address;
};
```

### Interrupt calling

1. I0, flags register and an error code are sent as arguments
2. CPU calls address in relevant IDT entry using `call` instruction
3. relevant value in flags register is set so instructions know the CPU is in interrupt mode.

### Interrupt returning

1. remove arguments from stack
2. call `ret` instruction

## Flags info

- Has a 64-bit internal register for storing flags

### Flags Layout

- Bit 0: Carry flag. ALU instruction resulted in Carry
- Bit 1: Zero flag. ALU instruction resulted in Zero
- Bit 2: Sign flag. ALU instruction resulted in a negative number
- Bit 3: Interrupt mode
- Bit 4: Is writing to the IO bus
- Bit 5: Is reading from the IO bus
- Bits 6-63: reserved

Bits 3-5 are not yet implemented.
