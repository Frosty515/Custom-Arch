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

#ifndef _IO_DEVICE_HPP
#define _IO_DEVICE_HPP

#include <stdint.h>

#include "IOBus.hpp"

class IOMemoryRegion;

class IODevice {
   public:
    explicit IODevice(IODeviceID ID, uint64_t size)
        : m_base_address(0), m_size(size), m_ID(ID), m_memoryRegion(nullptr) {}
    virtual ~IODevice() = default;

    virtual uint8_t ReadByte(uint64_t address) = 0;
    virtual uint16_t ReadWord(uint64_t address) = 0;
    virtual uint32_t ReadDWord(uint64_t address) = 0;
    virtual uint64_t ReadQWord(uint64_t address) = 0;

    virtual void WriteByte(uint64_t address, uint8_t data) = 0;
    virtual void WriteWord(uint64_t address, uint16_t data) = 0;
    virtual void WriteDWord(uint64_t address, uint32_t data) = 0;
    virtual void WriteQWord(uint64_t address, uint64_t data) = 0;

    uint64_t GetBaseAddress() const { return m_base_address; }
    uint64_t GetSize() const { return m_size; }
    IODeviceID GetID() const { return m_ID; }
    IOMemoryRegion* GetMemoryRegion() const { return m_memoryRegion; }

    void SetBaseAddress(uint64_t base_address) { m_base_address = base_address; }
    void SetMemoryRegion(IOMemoryRegion* memoryRegion) { m_memoryRegion = memoryRegion; }

   private:
    uint64_t m_base_address;
    uint64_t m_size;
    IODeviceID m_ID;
    IOMemoryRegion* m_memoryRegion;
};

#endif /* _IO_DEVICE_HPP */