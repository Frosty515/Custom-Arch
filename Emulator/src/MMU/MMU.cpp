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

#include "MMU.hpp"
#include "Exceptions.hpp"
#include "MMU/StandardMemoryRegion.hpp"


MMU::MMU() {

}

MMU::~MMU() {

}

void MMU::init(size_t RAMSize) {
    m_regions.insert(new StandardMemoryRegion(0, RAMSize));
}

void MMU::ReadBuffer(uint64_t address, uint8_t* data, size_t size) {
    MemoryRegion* startingRegion = nullptr;
    uint64_t regionIndex = 0;
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
        if (region->isInside(address, 1)) {
            startingRegion = region;
            regionIndex++;
            break;
        }
    }
    if (startingRegion == nullptr) {
        // no region found
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
        return;
    }
    size_t remainingSize = size;
    uint64_t currentAddress = address;
    while (remainingSize > 0) {
        size_t currentSize = startingRegion->getEnd() - currentAddress;
        if (currentSize > remainingSize)
            currentSize = remainingSize;
        startingRegion->read(currentAddress, data, currentSize);
        remainingSize -= currentSize;
        if (remainingSize == 0)
            break;
        currentAddress += currentSize;
        data = (uint8_t*)((uint64_t)data + currentSize * sizeof(uint8_t));
        startingRegion = nullptr;
        for (MemoryRegion* region = m_regions.get(regionIndex); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
            if (region->isInside(currentAddress, 1)) {
                startingRegion = region;
                break;
            }
        }
        if (startingRegion == nullptr) {
            // no region found
            g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
            return;
        }
    }
}

void MMU::WriteBuffer(uint64_t address, const uint8_t* data, size_t size) {
    MemoryRegion* startingRegion = nullptr;
    uint64_t regionIndex = 0;
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
        if (region->isInside(address, 1)) {
            startingRegion = region;
            regionIndex++;
            break;
        }
    }
    if (startingRegion == nullptr) {
        // no region found
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
        return;
    }
    size_t remainingSize = size;
    uint64_t currentAddress = address;
    while (remainingSize > 0) {
        size_t currentSize = startingRegion->getEnd() - currentAddress;
        if (currentSize > remainingSize)
            currentSize = remainingSize;
        startingRegion->write(currentAddress, data, currentSize);
        remainingSize -= currentSize;
        if (remainingSize == 0)
            break;
        currentAddress += currentSize;
        data = (uint8_t*)((uint64_t)data + currentSize * sizeof(uint8_t));
        startingRegion = nullptr;
        for (MemoryRegion* region = m_regions.get(regionIndex); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
            if (region->isInside(currentAddress, 1)) {
                startingRegion = region;
                break;
            }
        }
        if (startingRegion == nullptr) {
            // no region found
            g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
            return;
        }
    }
}

uint8_t MMU::read8(uint64_t address) {
    uint8_t data = 0;
    bool found = false;
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 1)) {
            region->read8(address, &data);
            found = true;
            break;
        }
    }
    if (!found)
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
    return data;
}

uint16_t MMU::read16(uint64_t address) {
    uint16_t data = 0;
    bool found = false;
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 2)) {
            region->read16(address, &data);
            found = true;
            break;
        }
    }
    if (!found)
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
    return data;
}

uint32_t MMU::read32(uint64_t address) {
    uint32_t data = 0;
    bool found = false;
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 4)) {
            region->read32(address, &data);
            found = true;
            break;
        }
    }
    if (!found)
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
    return data;
}

uint64_t MMU::read64(uint64_t address) {
    uint64_t data = 0;
    bool found = false;
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 8)) {
            region->read64(address, &data);
            found = true;
            break;
        }
    }
    if (!found)
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
    return data;
}

void MMU::write8(uint64_t address, uint8_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 1)) {
            region->write8(address, &data);
            return;
        }
    }
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
}

void MMU::write16(uint64_t address, uint16_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 2)) {
            region->write16(address, &data);
            return;
        }
    }
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
}

void MMU::write32(uint64_t address, uint32_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 4)) {
            region->write32(address, &data);
            return;
        }
    }
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
}

void MMU::write64(uint64_t address, uint64_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 8)) {
            region->write64(address, &data);
            return;
        }
    }
    printf("Address: %lx\n", address);
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION);
}

bool MMU::ValidateRead(uint64_t address, size_t size) {
    MemoryRegion* startingRegion = nullptr;
    uint64_t regionIndex = 0;
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
        if (region->isInside(address, 1)) {
            startingRegion = region;
            regionIndex++;
            break;
        }
    }
    if (startingRegion == nullptr) {
        // no region found
        return false;
    }
    size_t remainingSize = size;
    uint64_t currentAddress = address;
    while (remainingSize > 0) {
        size_t currentSize = startingRegion->getEnd() - currentAddress;
        if (currentSize > remainingSize)
            currentSize = remainingSize;
        remainingSize -= currentSize;
        if (remainingSize == 0)
            break;
        currentAddress += currentSize;
        startingRegion = nullptr;
        for (MemoryRegion* region = m_regions.get(regionIndex); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
            if (region->isInside(currentAddress, 1)) {
                startingRegion = region;
                break;
            }
        }
        if (startingRegion == nullptr) {
            // no region found
            return false;
        }
    }
    return true;
}

void MMU::AddMemoryRegion(MemoryRegion* region) {
    m_regions.insert(region);
}

void MMU::DumpMemory() const {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        region->dump();
    }
}
