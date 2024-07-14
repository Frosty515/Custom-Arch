#include "IOBus.hpp"

#include <stdio.h>

IOBus* g_IOBus = nullptr;

IOBus::IOBus() {
}

IOBus::~IOBus() {
}

uint8_t IOBus::ReadByte(uint64_t address) {
    printf("IOBus::ReadByte(%lu)\n", address);
    return 0;
}

uint16_t IOBus::ReadWord(uint64_t address) {
    printf("IOBus::ReadWord(%lu)\n", address);
    return 0;
}

uint32_t IOBus::ReadDWord(uint64_t address) {
    printf("IOBus::ReadDWord(%lu)\n", address);
    return 0;
}

uint64_t IOBus::ReadQWord(uint64_t address) {
    printf("IOBus::ReadQWord(%lu)\n", address);
    return 0;
}

void IOBus::WriteByte(uint64_t address, uint8_t data) {
    printf("IOBus::WriteByte(%lu, %hhu)\n", address, data);
}

void IOBus::WriteWord(uint64_t address, uint16_t data) {
    printf("IOBus::WriteWord(%lu, %hu)\n", address, data);
}

void IOBus::WriteDWord(uint64_t address, uint32_t data) {
    printf("IOBus::WriteDWord(%lu, %u)\n", address, data);
}

void IOBus::WriteQWord(uint64_t address, uint64_t data) {
    printf("IOBus::WriteQWord(%lu, %lu)\n", address, data);
}
