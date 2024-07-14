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