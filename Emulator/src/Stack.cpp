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

#include "Stack.hpp"
#include "Exceptions.hpp"

#include <Emulator.hpp>

Stack::Stack() {

}

Stack::Stack(MMU* mmu, uint64_t base, uint64_t top, uint64_t pointer) : m_MMU(mmu), m_stackBase(base), m_stackPointer(pointer), m_stackTop(top) {

}

Stack::~Stack() {

}

void Stack::push(uint64_t value) {
    if (m_stackPointer < m_stackBase)
        g_ExceptionHandler->RaiseException(Exception::STACK_VIOLATION); // stack underflow
    if (m_stackPointer >= m_stackTop)
        g_ExceptionHandler->RaiseException(Exception::STACK_VIOLATION); // stack overflow
    m_stackPointer += 8;
    m_MMU->write64(m_stackPointer, value);
}

uint64_t Stack::pop() {
    if (m_stackPointer >= m_stackTop)
        g_ExceptionHandler->RaiseException(Exception::STACK_VIOLATION); // stack overflow
    if (m_stackPointer < m_stackBase)
        g_ExceptionHandler->RaiseException(Exception::STACK_VIOLATION); // stack underflow
    uint64_t value = m_MMU->read64(m_stackPointer);
    m_stackPointer -= 8;
    return value;
}

uint64_t Stack::peek() {
    return m_MMU->read64(m_stackPointer);
}

void Stack::clear() {
    m_stackPointer = m_stackBase;
}

void Stack::setStackBase(uint64_t base) {
    m_stackBase = base;
}

void Stack::setStackTop(uint64_t top) {
    m_stackTop = top;
}

void Stack::setStackPointer(uint64_t pointer) {
    m_stackPointer = pointer;
}

uint64_t Stack::getStackBase() const {
    return m_stackBase;
}

uint64_t Stack::getStackTop() const {
    return m_stackTop;
}

uint64_t Stack::getStackPointer() const {
    return m_stackPointer;
}

bool Stack::WillOverflowOnPush() const {
    return m_stackPointer >= m_stackTop;
}

bool Stack::WillUnderflowOnPop() const {
    return m_stackPointer < m_stackBase;
}

Stack* g_stack = nullptr;
