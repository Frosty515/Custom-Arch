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

#include "StandardMemoryRegion.hpp"

#include <string.h>

#include <OSSpecific/Memory.hpp>

StandardMemoryRegion::StandardMemoryRegion(uint64_t start, uint64_t end)
    : MemoryRegion(start, end) {
    m_data = static_cast<uint8_t*>(OSSpecific::AllocateCOWMemory(MemoryRegion::getSize()));
}

StandardMemoryRegion::~StandardMemoryRegion() {
    OSSpecific::FreeCOWMemory(m_data);
}

void StandardMemoryRegion::read(uint64_t address, uint8_t* buffer, size_t size) {
    if (isInside(address, size))
        memcpy(buffer, m_data + (address - getStart()), size);
}

void StandardMemoryRegion::write(uint64_t address, const uint8_t* buffer, size_t size) {
    if (isInside(address, size))
        memcpy(m_data + (address - getStart()), buffer, size);
}
