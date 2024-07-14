#ifndef _STACK_HPP
#define _STACK_HPP

#include <stdint.h>

#include <MMU/MMU.hpp>

class Stack {
public:
    Stack();
    Stack(MMU* mmu, uint64_t base, uint64_t top, uint64_t pointer);
    ~Stack();

    void push(uint64_t value);
    uint64_t pop();
    uint64_t peek();
    void clear();

    void setStackBase(uint64_t base);
    void setStackTop(uint64_t top);
    void setStackPointer(uint64_t pointer);

    uint64_t getStackBase() const;
    uint64_t getStackTop() const;
    uint64_t getStackPointer() const;

    bool WillOverflowOnPush() const;
    bool WillUnderflowOnPop() const;
    
private:
    MMU* m_MMU;

    uint64_t m_stackBase;
    uint64_t m_stackPointer;
    uint64_t m_stackTop;
};

extern Stack* g_stack;

#endif /* _STACK_HPP */