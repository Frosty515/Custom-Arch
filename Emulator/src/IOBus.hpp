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