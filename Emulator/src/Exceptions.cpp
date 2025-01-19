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

#include "Exceptions.hpp"

#include <stdarg.h>

#include <Instruction/Instruction.hpp>
#include <Stack.hpp>

#include "Emulator.hpp"
#include "Interrupts.hpp"

ExceptionHandler::ExceptionHandler()
    : m_INTHandler(nullptr) {
}

ExceptionHandler::ExceptionHandler(InterruptHandler* INTHandler)
    : m_INTHandler(INTHandler) {
}

ExceptionHandler::~ExceptionHandler() {
}

[[noreturn]] void ExceptionHandler::RaiseException(Exception exception, ...) {
    if (exception == Exception::TWICE_UNHANDLED_INTERRUPT || m_INTHandler == nullptr) {
        Emulator::Crash("ExceptionHandler::RaiseException(): unhandled exception");
    }
    if (exception == Exception::PHYS_MEM_VIOLATION || exception == Exception::UNHANDLED_INTERRUPT || exception == Exception::STACK_VIOLATION || exception == Exception::PAGING_VIOLATION) {
        // check that the stack can be accessed
        if (g_stack->WillOverflowOnPush() || g_stack->WillUnderflowOnPop() /* might already be under */ || (g_stack->getStackPointer() % 8) > 0 || !m_INTHandler->GetMMU()->ValidateRead(g_stack->getStackPointer() + 8, 8)) {
            if (exception == Exception::UNHANDLED_INTERRUPT)
                RaiseException(Exception::TWICE_UNHANDLED_INTERRUPT);
            else
                RaiseException(Exception::UNHANDLED_INTERRUPT, static_cast<uint8_t>(exception));
        }

        va_list args;
        va_start(args, exception);

        switch (exception) {
        case Exception::PHYS_MEM_VIOLATION:
            g_stack->push(va_arg(args, uint64_t)); // address
            break;
        case Exception::UNHANDLED_INTERRUPT:
            g_stack->push(va_arg(args, int)); // interrupt, uint8_t is promoted to int
            break;
        case Exception::STACK_VIOLATION: {
            StackViolationErrorCode code = va_arg(args, StackViolationErrorCode);
            uint64_t* temp = reinterpret_cast<uint64_t*>(&code);
            g_stack->push(*temp); // error code
            break;
        }
        case Exception::PAGING_VIOLATION: {
            g_stack->push(va_arg(args, uint64_t)); // address
            PagingViolationErrorCode code = va_arg(args, PagingViolationErrorCode);
            uint64_t* temp = reinterpret_cast<uint64_t*>(&code);
            g_stack->push(*temp); // error code
            break;
        }
        default:
            break;
        }

        va_end(args);
    }
    m_INTHandler->RaiseInterrupt(static_cast<uint8_t>(exception), Emulator::GetCPU_IP());
}

void ExceptionHandler::SetINTHandler(InterruptHandler* INTHandler) {
    m_INTHandler = INTHandler;
}

ExceptionHandler* g_ExceptionHandler = nullptr;
