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

#include "MemoryRegion.hpp"

MemoryRegion::MemoryRegion(uint64_t start, uint64_t end) : m_start(start), m_end(end), m_size(end - start + 1) {

}

uint64_t MemoryRegion::getStart() {
    return m_start;
}

uint64_t MemoryRegion::getEnd() {
    return m_end;
}

size_t MemoryRegion::getSize() {
    return m_size;
}

bool MemoryRegion::isInside(uint64_t address, size_t size) {
    return address >= m_start && (address + size) <= m_end;
}
