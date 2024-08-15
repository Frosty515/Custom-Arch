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

#ifndef _OS_SPECIFIC_MEMORY_HPP
#define _OS_SPECIFIC_MEMORY_HPP

#include <stdint.h>
#include <stddef.h>

namespace OSSpecific {

    void* GenericAllocateMemory(size_t size);
    void* GenericAllocateZeroedMemory(size_t size);
    void GenericFreeMemory(void* ptr);
    void GenericFreeSizedMemory(void* ptr, size_t size);

    void* AllocateCOWMemory(size_t size);
    void* AllocateZeroedCOWMemory(size_t size);
    void FreeCOWMemory(void* ptr);
    void FreeSizedCOWMemory(void* ptr, size_t size);

}

#endif /* _OS_SPECIFIC_MEMORY_HPP */