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
