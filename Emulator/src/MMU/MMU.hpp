#ifndef _MMU_HPP
#define _MMU_HPP

#include "MemoryRegion.hpp"

#include <stddef.h>
#include <stdint.h>

#include <Data-structures/LinkedList.hpp>

class MMU {
public:
    MMU();
    ~MMU();

    void init(size_t RAMSize);

    void ReadBuffer(uint64_t address, uint8_t* data, size_t size);
    void WriteBuffer(uint64_t address, uint8_t* data, size_t size);

    uint8_t read8(uint64_t address);
    uint16_t read16(uint64_t address);
    uint32_t read32(uint64_t address);
    uint64_t read64(uint64_t address);

    void write8(uint64_t address, uint8_t data);
    void write16(uint64_t address, uint16_t data);
    void write32(uint64_t address, uint32_t data);
    void write64(uint64_t address, uint64_t data);

    bool ValidateRead(uint64_t address, size_t size);

private:
    LinkedList::SimpleLinkedList<MemoryRegion> m_regions;
};

#endif /* _MMU_HPP */