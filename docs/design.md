# Custom Architecture

## Registers

- 16 64-bit general purpose registers (GPRs) named r0-r15
- 3 64-bit stack related registers. See [Stack](#stack-info)
- 1 64-bit flags register that is read-only. named FLAGS. See [Flags Layout](#flags-layout) for more info
- 4 64-bit control registers named CR0-CR8, see [Control Registers Layout](#control-registers-layout) for more info
- 2 64-bit instruction registers, I0 for current address and I1 for previous frames address (return address). I0 and I1 are read-only.

### Control Registers Layout

#### CR0

| Bit | Name | Description |
| --- | ---- | ----------- |
| 0   | PE   | Protected mode enabled |
| 1   | IO   | IO instructions enabled in user mode |
| 2-63| RESERVED | Reserved |

#### CR1

- Note: CR1 is reserved in 64-bit real mode

| Bit | Name | Description |
| --- | ---- | ----------- |
| 0-63| SFLAGS | Supervisor flags on supervisor mode entry |

#### CR2

- Note: CR2 is reserved in 64-bit real mode

| Bit | Name | Description |
| --- | ---- | ----------- |
| 0-63| SENTRY | Supervisor `I0` on supervisor mode entry |

#### CR3-CR8

| Bit | Name | Description |
| --- | ---- | ----------- |
| 0-63| RESERVED | Reserved |

### Flags Layout

| Bit | Name | Description |
| --- | ---- | ----------- |
| 0   | CF   | Carry flag |
| 1   | ZF   | Zero flag |
| 2   | SF   | Sign flag |
| 3   | IF   | Interrupt flag |
| 4   | IOF  | Is writing to the IO bus |
| 5   | IIF  | Is reading from the IO bus |
| 6-63| RESERVED | Reserved |

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

- Default mode: 64-bit real mode
- Extra mode: 64-bit protected mode

### 64-bit real mode

- All memory is accessible by everything
- All registers are accessible by everything (I0, I1, and FLAGS are read-only)
- All valid instructions always work

### 64-bit protected mode

- 2 levels of access: user, supervisor
- bit 0 of CR0 is set to 1 to enable protected mode
- defaults to supervisor mode
- Interrupts behave differently. See [Protected mode interrupts](#protected-mode-interrupts) for more info
- bit 1 of CR0 controls where [IO instructions](#io) are available in user mode.

#### Switching privilege levels

On supervisor mode entry, the contents of `FLAGS` is swapped with the contents of `CR1`, and `I0` will be set to `CR2`. `I1` will be set to the address of the instruction after the `syscall` instruction. The old value of I1 is not saved. `r15` will be set to the user mode stack. Supervisor mode is responsible for saving `sbp`, `stp`, and `r0`-`r14` if they are of importance. Supervisor mode is also responsible for getting its own stack. After all this, the values of `I1` and `r15` would have been overridden. If they are of importance, they should be saved prior to the `syscall` instruction.

On supervisor mode exit, the contents of `CR1` is swapped with the contents of `FLAGS`, and `I0` will be set to `I1`. `scp` will be set to `r15`. The old value of `I0` is not saved. Supervisor mode is responsible for restoring `sbp`, `stp`, and `r0`-`r14` if they were saved.

On user mode entry (different from supervisor mode exit), `I1` and `FLAGS` are cleared. `I0` will be set to the first argument of the instruction. All other registers are untouched.

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

### Protected mode Instructions

- `syscall` to enter supervisor mode
- `sysret` to return to supervisor mode
- `enteruser (address, register or immediate)` to enter user mode

## Interrupts info

Has a register called IDTR which contains the address of a table called the Interrupt Descriptor Table (IDT) which contains the addresses of interrupt handlers. It is 256 entries long. The format of an entry is as follows:

| Bit | Name | Description |
| --- | ---- | ----------- |
| 0   | Present | Is this entry present |
| 1-7 | Flags | Reserved |
| 8-71 | Address | Address of interrupt handler |

### Interrupt calling

1. I0, flags register and an error code are sent as arguments
2. CPU calls address in relevant IDT entry using `call` instruction
3. relevant value in flags register is set so instructions know the CPU is in interrupt mode.

### Interrupt returning

1. remove arguments from stack
2. call `ret` instruction

### Protected mode interrupts

- Bit 1 of the IDT entry is set to 1 if the interrupt is available in user mode.
- Interrupts are **always** handled in kernel mode.

## Assembly syntax

### Labels

- Labels are defined by a string followed by a colon

### Sub-labels

- Sub-labels are defined by a period followed by a string followed by a colon
- Sub-labels are only accessible within the scope of the label they are defined in

### Comments

- Single line comments are defined by a semicolon followed by a string
- Multi-line comments are defined by a `/*` followed by a string and ending with a `*/`

### Includes

- `%include "path/to/file.asm"` to include another assembly file

### Directives

- `db` to define a byte
- `dw` to define a word (2 bytes)
- `dd` to define a double-word (4 bytes)
- `dq` to define a quad-word (8 bytes)
