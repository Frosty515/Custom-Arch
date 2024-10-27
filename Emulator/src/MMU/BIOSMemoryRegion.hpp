#ifndef _BIOS_MEMORY_REGION_HPP
#define _BIOS_MEMORY_REGION_HPP

#include "StandardMemoryRegion.hpp"

class BIOSMemoryRegion : public StandardMemoryRegion {
public:
    BIOSMemoryRegion(uint64_t start, uint64_t end, uint64_t real_size);
    ~BIOSMemoryRegion();

    virtual void dump() override;

private:
    uint64_t m_real_size;
};

#endif /* _BIOS_MEMORY_REGION_HPP */