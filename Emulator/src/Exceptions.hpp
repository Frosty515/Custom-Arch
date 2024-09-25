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

#ifndef _EXCEPTIONS_HPP
#define _EXCEPTIONS_HPP

#include <stdint.h>

class InterruptHandler;

enum class Exception {
    DIV_BY_ZERO = 0,
    PHYS_MEM_VIOLATION = 1,
    UNHANDLED_INTERRUPT = 2,
    INVALID_INSTRUCTION = 3,
    STACK_VIOLATION = 4,
    USER_MODE_VIOLATION = 5,
    SUPERVISOR_MODE_VIOLATION = 6,
    TWICE_UNHANDLED_INTERRUPT = -1 // not valid number
};

class ExceptionHandler {
public:
    ExceptionHandler();
    ExceptionHandler(InterruptHandler* INTHandler);
    ~ExceptionHandler();

    [[noreturn]] void RaiseException(Exception exception);

    void SetINTHandler(InterruptHandler* INTHandler);

private:
    InterruptHandler* m_INTHandler;
};

extern ExceptionHandler* g_ExceptionHandler;

#endif /* _EXCEPTIONS_HPP */