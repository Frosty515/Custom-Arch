#ifndef _STANDARD_MEMORY_REGION_HPP
#define _STANDARD_MEMORY_REGION_HPP

#include <stdint.h>

#include "MemoryRegion.hpp"

class StandardMemoryRegion : public MemoryRegion {
public:
    StandardMemoryRegion(uint64_t start, uint64_t end);
    ~StandardMemoryRegion();

    virtual void read(uint64_t address, uint8_t* buffer, size_t size) override;
    virtual void write(uint64_t address, const uint8_t* buffer, size_t size) override;

private:
    uint8_t* m_data;
};

#endif /* _STANDARD_MEMORY_REGION_HPP */