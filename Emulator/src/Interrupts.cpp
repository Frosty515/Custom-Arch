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

#include "Interrupts.hpp"

#include <Instruction/Instruction.hpp>

#include "Emulator.hpp"
#include "Exceptions.hpp"
#include "Stack.hpp"

InterruptHandler::InterruptHandler(MMU* mmu, ExceptionHandler* exceptionHandler)
    : m_MMU(mmu), m_ExceptionHandler(exceptionHandler), m_IDTR(0) {
    for (int i = 0; i < 256; i++) {
        m_IDT[i].loaded = true;
        m_IDT[i].flags = 0;
        m_IDT[i].handler = 0;
    }
}

InterruptHandler::~InterruptHandler() {
}

void InterruptHandler::SetIDTR(uint64_t base) {
    m_IDTR = base;
    for (int i = 0; i < 256; i++)
        m_IDT[i].loaded = false;
}

[[noreturn]] void InterruptHandler::RaiseInterrupt(uint8_t interrupt, uint64_t IP) {
    RaiseInterruptCommon(interrupt, IP);
    Emulator::JumpToIP(m_IDT[interrupt].handler);
}

void InterruptHandler::RaiseInterruptExternal(uint8_t interrupt) {
    Emulator::KillCurrentInstruction();
    RaiseInterruptCommon(interrupt, Emulator::GetCPU_IP());
    Emulator::JumpToIPExternal(m_IDT[interrupt].handler);
}


void InterruptHandler::ReturnFromInterrupt() {
    StackViolationErrorCode code = {1, 0, 0, 0};
    if (g_stack->WillUnderflowOnPop())
        m_ExceptionHandler->RaiseException(Exception::STACK_VIOLATION, code);
    Emulator::SetCPUStatus(g_stack->pop());
    if (g_stack->WillUnderflowOnPop())
        m_ExceptionHandler->RaiseException(Exception::STACK_VIOLATION, code);
    Emulator::JumpToIP(g_stack->pop());
}

void InterruptHandler::ChangeMMU(MMU* mmu) {
    m_MMU = mmu;
    for (int i = 0; i < 256; i++) // invalidate all descriptors
        m_IDT[i].loaded = false;
}

InterruptDescriptor InterruptHandler::ReadDescriptor(uint8_t interrupt) {
    if (!m_MMU->ValidateRead(m_IDTR + sizeof(RawInterruptDescriptor) * interrupt, sizeof(RawInterruptDescriptor)))
        HandleFailure(interrupt);
    RawInterruptDescriptor rawDescriptor;
    m_MMU->ReadBuffer(m_IDTR + sizeof(RawInterruptDescriptor) * interrupt, reinterpret_cast<uint8_t*>(&rawDescriptor), sizeof(RawInterruptDescriptor));
    InterruptDescriptor descriptor;
    descriptor.loaded = true;
    descriptor.flags = rawDescriptor.Present;
    descriptor.handler = rawDescriptor.addr;
    return descriptor;
}

void InterruptHandler::HandleFailure(uint8_t interrupt) {
    if (static_cast<Exception>(interrupt) == Exception::UNHANDLED_INTERRUPT)
        m_ExceptionHandler->RaiseException(Exception::TWICE_UNHANDLED_INTERRUPT);
    else
        m_ExceptionHandler->RaiseException(Exception::UNHANDLED_INTERRUPT, interrupt);
}

void InterruptHandler::RaiseInterruptCommon(uint8_t interrupt, uint64_t IP) {
    if (!m_IDT[interrupt].loaded)
        m_IDT[interrupt] = ReadDescriptor(interrupt);
    if ((m_IDT[interrupt].flags & 1) == 0)
        HandleFailure(interrupt);
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        Emulator::ExitUserMode();
    if (g_stack->WillOverflowOnPush())
        HandleFailure(interrupt);
    g_stack->push(IP);
    if (g_stack->WillOverflowOnPush())
        HandleFailure(interrupt);
    g_stack->push(Emulator::GetCPUStatus());
}

InterruptHandler* g_InterruptHandler = nullptr;
