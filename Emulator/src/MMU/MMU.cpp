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
#include "MMU/MemoryRegion.hpp"
#include "MMU/StandardMemoryRegion.hpp"

MMU::MMU() {
}

MMU::~MMU() {
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
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
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
        data = reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(data) + currentSize * sizeof(uint8_t));
        startingRegion = nullptr;
        for (MemoryRegion* region = m_regions.get(regionIndex); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
            if (region->isInside(currentAddress, 1)) {
                startingRegion = region;
                break;
            }
        }
        if (startingRegion == nullptr) {
            // no region found
            g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, currentAddress);
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
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
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
        data = reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(data) + currentSize * sizeof(uint8_t));
        startingRegion = nullptr;
        for (MemoryRegion* region = m_regions.get(regionIndex); region != nullptr; region = m_regions.getNext(region), regionIndex++) {
            if (region->isInside(currentAddress, 1)) {
                startingRegion = region;
                break;
            }
        }
        if (startingRegion == nullptr) {
            // no region found
            g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, currentAddress);
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
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
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
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
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
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
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
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
    return data;
}

void MMU::write8(uint64_t address, uint8_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 1)) {
            region->write8(address, &data);
            return;
        }
    }
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
}

void MMU::write16(uint64_t address, uint16_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 2)) {
            region->write16(address, &data);
            return;
        }
    }
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
}

void MMU::write32(uint64_t address, uint32_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 4)) {
            region->write32(address, &data);
            return;
        }
    }
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
}

void MMU::write64(uint64_t address, uint64_t data) {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        if (region->isInside(address, 8)) {
            region->write64(address, &data);
            return;
        }
    }
    g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, address);
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

bool MMU::ValidateWrite(uint64_t address, size_t size) {
    return ValidateRead(address, size);
}


bool MMU::ValidateExecute(uint64_t address, size_t size) {
    return ValidateRead(address, size);
}

void MMU::AddMemoryRegion(MemoryRegion* region) {
    m_regions.insert(region);
}

void MMU::RemoveMemoryRegion(MemoryRegion* region) {
    m_regions.remove(region);
}

void MMU::DumpMemory() const {
    for (MemoryRegion* region = m_regions.get(0); region != nullptr; region = m_regions.getNext(region)) {
        region->dump();
    }
}

bool MMU::RemoveRegionSegment(uint64_t start, uint64_t end) {
    for (uint64_t i = 0; i < m_regions.getCount(); i++) {
        if (MemoryRegion* region = m_regions.get(i); start >= region->getStart() && end <= region->getEnd()) {
            if (start == region->getStart() && end == region->getEnd()) {
                m_regions.remove(region);
                return true;
            }
            if (!region->canSplit())
                return false;
            // check if the region starts at start
            if (region->getStart() < start) {
                m_regions.remove(region);

                StandardMemoryRegion* newRegion = new StandardMemoryRegion(region->getStart(), start);
                m_regions.insert(newRegion);

                region = new StandardMemoryRegion(end, region->getEnd());
            }

            // check if the region ends at end
            if (region->getEnd() > end) {
                m_regions.remove(region);

                StandardMemoryRegion* newRegion = new StandardMemoryRegion(end, region->getEnd());
                m_regions.insert(newRegion);

                region = newRegion;
            }

            return true;
        }
    }
    return false;
}

bool MMU::ReaddRegionSegment(uint64_t start, uint64_t end) {
    MemoryRegion* region = nullptr;
    MemoryRegion* previousRegion = nullptr;
    for (uint64_t i = 0; i < m_regions.getCount(); i++) {
        previousRegion = region;
        region = m_regions.get(i);
        if (previousRegion == nullptr)
            continue;
        if (start >= previousRegion->getEnd() && end <= region->getStart()) {
            if (start == previousRegion->getEnd() && end == region->getStart()) {
                uint64_t newStart = previousRegion->getStart();
                uint64_t newEnd = region->getEnd();
                m_regions.remove(previousRegion);
                m_regions.remove(region);
                StandardMemoryRegion* newRegion = new StandardMemoryRegion(newStart, newEnd);
                m_regions.insert(newRegion);
            } else if (start == previousRegion->getEnd()) {
                uint64_t newStart = previousRegion->getStart();
                m_regions.remove(previousRegion);
                StandardMemoryRegion* newRegion = new StandardMemoryRegion(newStart, end);
                m_regions.insert(newRegion);
            } else if (end == region->getStart()) {
                uint64_t newEnd = region->getEnd();
                m_regions.remove(region);
                StandardMemoryRegion* newRegion = new StandardMemoryRegion(start, newEnd);
                m_regions.insert(newRegion);
            } else {
                StandardMemoryRegion* newRegion = new StandardMemoryRegion(start, end);
                m_regions.insert(newRegion);
            }
            return true;
        }
    }
    return false;
}
