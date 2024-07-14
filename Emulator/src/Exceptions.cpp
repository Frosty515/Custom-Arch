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
