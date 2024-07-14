#ifndef _MEMORY_REGION_HPP
#define _MEMORY_REGION_HPP

#include <stdint.h>
#include <stddef.h>

class MemoryRegion {
public:
    MemoryRegion(uint64_t start, uint64_t end);

    virtual void read(uint64_t address, uint8_t* buffer, size_t size) = 0;
    virtual void write(uint64_t address, const uint8_t* buffer, size_t size) = 0;

    virtual uint64_t getStart();
    virtual uint64_t getEnd();
    virtual size_t getSize();

    virtual bool isInside(uint64_t address, size_t size);

private:
    uint64_t m_start;
    uint64_t m_end;
    size_t m_size;
};

#endif /* _MEMORY_REGION_HPP */