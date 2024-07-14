#ifndef _IO_BUS_HPP
#define _IO_BUS_HPP

#include <stdint.h>

class IOBus {
public:
    IOBus();
    ~IOBus();

    uint8_t ReadByte(uint64_t address);
    uint16_t ReadWord(uint64_t address);
    uint32_t ReadDWord(uint64_t address);
    uint64_t ReadQWord(uint64_t address);

    void WriteByte(uint64_t address, uint8_t data);
    void WriteWord(uint64_t address, uint16_t data);
    void WriteDWord(uint64_t address, uint32_t data);
    void WriteQWord(uint64_t address, uint64_t data);
};

extern IOBus* g_IOBus;

#endif /* _IO_BUS_HPP */