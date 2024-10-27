#include "BIOSMemoryRegion.hpp"

#include <stdio.h>

BIOSMemoryRegion::BIOSMemoryRegion(uint64_t start, uint64_t end, uint64_t real_size) : StandardMemoryRegion(start, end), m_real_size(real_size) {

}

BIOSMemoryRegion::~BIOSMemoryRegion() {

}

void BIOSMemoryRegion::dump() {
    printf("BIOSMemoryRegion: %lx - %lx, real_size = %lx", getStart(), getEnd(), m_real_size);
    uint8_t buffer[16];
    uint8_t buffer_index = 0;
    for (uint64_t i = getStart(); i < m_real_size; i++, buffer_index++) {
        if ((i - getStart()) % 16 == 0) {
            buffer_index = 0;
            printf("\n%016lx: ", i);
        }
        else if ((i - getStart()) % 8 == 0)
            printf(" ");
        uint8_t data = 0;
        read8(i, &data);
        buffer[buffer_index] = data;
        printf("%02X ", data);
        if ((i - getStart()) % 16 == 15) {
            printf(" |");
            for (uint64_t j = 0; j < 16; j++)
                printf("%c", buffer[j] >= 32 && buffer[j] <= 126 ? buffer[j] : '.');
            printf("|");
        }
    }
    printf("\n");
}