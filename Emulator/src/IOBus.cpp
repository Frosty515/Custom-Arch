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

#include "IOBus.hpp"

#include <stdio.h>

#include <Emulator.hpp>
#include <Exceptions.hpp>

IOBus* g_IOBus = nullptr;

IOBus::IOBus() {
}

IOBus::~IOBus() {
}

uint8_t IOBus::ReadByte(uint64_t address) {
    Validate();
    printf("IOBus::ReadByte(%lu)\n", address);
    return 0;
}

uint16_t IOBus::ReadWord(uint64_t address) {
    Validate();
    printf("IOBus::ReadWord(%lu)\n", address);
    return 0;
}

uint32_t IOBus::ReadDWord(uint64_t address) {
    Validate();
    printf("IOBus::ReadDWord(%lu)\n", address);
    return 0;
}

uint64_t IOBus::ReadQWord(uint64_t address) {
    Validate();
    printf("IOBus::ReadQWord(%lu)\n", address);
    return 0;
}

void IOBus::WriteByte(uint64_t address, uint8_t data) {
    Validate();
    printf("IOBus::WriteByte(%lu, %hhu)\n", address, data);
}

void IOBus::WriteWord(uint64_t address, uint16_t data) {
    Validate();
    printf("IOBus::WriteWord(%lu, %hu)\n", address, data);
}

void IOBus::WriteDWord(uint64_t address, uint32_t data) {
    Validate();
    printf("IOBus::WriteDWord(%lu, %u)\n", address, data);
}

void IOBus::WriteQWord(uint64_t address, uint64_t data) {
    Validate();
    printf("IOBus::WriteQWord(%lu, %lu)\n", address, data);
}

void IOBus::Validate() const {
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode() && !Emulator::isUserIOAllowed())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
}
