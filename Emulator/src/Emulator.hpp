/*
Copyright (©) 2023-2024  Frosty515

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

#ifndef _EMULATOR_HPP
#define _EMULATOR_HPP

#include <stdint.h>
#include <stddef.h>

#include <Register.hpp>

namespace Emulator {

    enum StartErrors {
        SE_SUCCESS        = 0,
        SE_MALLOC_FAIL    = 1,
        SE_TOO_LITTLE_RAM = 2
    };

    void HandleMemoryOperation(uint64_t address, void* data, uint64_t size, uint64_t count, bool write);

    int Start(uint8_t* data, size_t size, size_t RAM);
    int RequestEmulatorStop();
    int SendInstruction(uint64_t instruction);

    void SetCPUFlags(uint64_t mask);
    void ClearCPUFlags(uint64_t mask);
    uint64_t GetCPUFlags();

    void SetNextIP(uint64_t value);
    uint64_t GetNextIP();

    void SetCPU_IP(uint64_t value);
    uint64_t GetCPU_IP();

    void JumpToIP(uint64_t value);

    void SyncRegisters();

    Register* GetRegisterPointer(uint8_t ID);

    void Crash(const char* message);
    void HandleHalt();
}

#endif /* _EMULATOR_HPP */