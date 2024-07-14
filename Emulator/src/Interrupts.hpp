#ifndef _INTERRUPTS_HPP
#define _INTERRUPTS_HPP

#include <stdint.h>

#include <MMU/MMU.hpp>

struct InterruptDescriptor {
    bool loaded;
    uint8_t flags;
    uint64_t handler;
};

struct RawInterruptDescriptor {
    uint8_t Present : 1;
    uint8_t reserved : 7;
    uint64_t addr;
} __attribute__((packed));

struct IDTR {
    uint16_t limit;
    uint64_t base;
};

class ExceptionHandler;

class InterruptHandler {
public:
    InterruptHandler(MMU* mmu, ExceptionHandler* exceptionHandler);
    ~InterruptHandler();

    void SetIDTR(uint64_t base);

    void RaiseInterrupt(uint8_t interrupt, uint64_t IP);
    void ReturnFromInterrupt();

private:
    InterruptDescriptor ReadDescriptor(uint8_t interrupt);
    void HandleFailure(uint8_t interrupt);

private:
    MMU* m_MMU;
    ExceptionHandler* m_ExceptionHandler;
    InterruptDescriptor m_IDT[256];
    uint64_t m_IDTR;
};

extern InterruptHandler* g_InterruptHandler;

#endif /* _INTERRUPTS_HPP */