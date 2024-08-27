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

#include "IOMemoryRegion.hpp"

IOMemoryRegion::IOMemoryRegion(uint64_t start, uint64_t end, IOBus* bus) : MemoryRegion(start, end), m_bus(bus) {

}

void IOMemoryRegion::read(uint64_t address, uint8_t* buffer, size_t size) {
    for (size_t i = 0; i < size; i++)
        buffer[i] = m_bus->ReadByte((address - getStart() + i) / 8);
}

void IOMemoryRegion::write(uint64_t address, const uint8_t* buffer, size_t size) {
    for (size_t i = 0; i < size; i++)
        m_bus->WriteByte((address - getStart() + i) / 8, buffer[i]);
}

void IOMemoryRegion::read8(uint64_t address, uint8_t* buffer) {
    *buffer = m_bus->ReadByte((address - getStart()) / 8);
}

void IOMemoryRegion::read16(uint64_t address, uint16_t* buffer) {
    *buffer = m_bus->ReadWord((address - getStart()) / 8);
}

void IOMemoryRegion::read32(uint64_t address, uint32_t* buffer) {
    *buffer = m_bus->ReadDWord((address - getStart()) / 8);
}

void IOMemoryRegion::read64(uint64_t address, uint64_t* buffer) {
    *buffer = m_bus->ReadQWord((address - getStart()) / 8);
}

void IOMemoryRegion::write8(uint64_t address, const uint8_t* buffer) {
    m_bus->WriteByte((address - getStart()) / 8, *buffer);
}

void IOMemoryRegion::write16(uint64_t address, const uint16_t* buffer) {
    m_bus->WriteWord((address - getStart()) / 8, *buffer);
}

void IOMemoryRegion::write32(uint64_t address, const uint32_t* buffer) {
    m_bus->WriteDWord((address - getStart()) / 8, *buffer);
}

void IOMemoryRegion::write64(uint64_t address, const uint64_t* buffer) {
    m_bus->WriteQWord((address - getStart()) / 8, *buffer);
}

void IOMemoryRegion::dump() {
    printf("IOMemoryRegion: %lx - %lx\n", getStart(), getEnd());
}
