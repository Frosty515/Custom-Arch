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
    TWICE_UNHANDLED_INTERRUPT = -1 // not valid number
};

class ExceptionHandler {
public:
    ExceptionHandler();
    ExceptionHandler(InterruptHandler* INTHandler);
    ~ExceptionHandler();

    void RaiseException(Exception exception);

    void SetINTHandler(InterruptHandler* INTHandler);

private:
    InterruptHandler* m_INTHandler;
};

extern ExceptionHandler* g_ExceptionHandler;

#endif /* _EXCEPTIONS_HPP */