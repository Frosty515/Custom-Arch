#include "StandardMemoryRegion.hpp"

#include <string.h>

StandardMemoryRegion::StandardMemoryRegion(uint64_t start, uint64_t end) : MemoryRegion(start, end) {
    m_data = new uint8_t[getSize()];
}

StandardMemoryRegion::~StandardMemoryRegion() {
    delete[] m_data;
}

void StandardMemoryRegion::read(uint64_t address, uint8_t* buffer, size_t size) {
    if (isInside(address, size))
        memcpy(buffer, m_data + (address - getStart()), size);
}

void StandardMemoryRegion::write(uint64_t address, const uint8_t* buffer, size_t size) {
    if (isInside(address, size))
        memcpy(m_data + (address - getStart()), buffer, size);
}
