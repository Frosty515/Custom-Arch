#include "Exceptions.hpp"
#include "Emulator.hpp"
#include "Interrupts.hpp"

ExceptionHandler::ExceptionHandler() : m_INTHandler(nullptr) {

}

ExceptionHandler::ExceptionHandler(InterruptHandler* INTHandler) : m_INTHandler(INTHandler) {

}

ExceptionHandler::~ExceptionHandler() {

}

void ExceptionHandler::RaiseException(Exception exception) {
    if (exception == Exception::TWICE_UNHANDLED_INTERRUPT || m_INTHandler == nullptr) {
        Emulator::Crash("ExceptionHandler::RaiseException(): unhandled exception");
        return;
    }

    m_INTHandler->RaiseInterrupt((uint8_t)exception, Emulator::GetCPU_IP());
}

void ExceptionHandler::SetINTHandler(InterruptHandler* INTHandler) {
    m_INTHandler = INTHandler;
}

ExceptionHandler* g_ExceptionHandler = nullptr;
