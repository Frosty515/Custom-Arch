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

#include "../Memory.hpp"

#include <stdlib.h>
#include <sys/mman.h>

#include "Emulator.hpp"

namespace OSSpecific {

    void* GenericAllocateMemory(size_t size) {
        return malloc(size);
    }

    void* GenericAllocateZeroedMemory(size_t size) {
        return calloc(1, size);
    }

    void GenericFreeMemory(void* ptr) {
        free(ptr);
    }

    void GenericFreeSizedMemory(void* ptr, size_t size) {
        (void)size;
        free(ptr);
    }

    void* AllocateCOWMemory(size_t size) {
        return AllocateZeroedCOWMemory(size); // on Linux, COW memory is always zeroed
    }

    void* AllocateZeroedCOWMemory(size_t size) {
        if (void* mem = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); mem == MAP_FAILED)
            Emulator::Crash("Failed to allocate COW memory");
        else
            return mem;
    }

    void FreeCOWMemory(void* ptr) {
        munmap(ptr, 0);
    }

    void FreeSizedCOWMemory(void* ptr, size_t size) {
        munmap(ptr, size);
    }

} // namespace OSSpecific
