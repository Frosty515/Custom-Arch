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

#include <Data-structures/LinkedList.hpp>

class MMU;
class IODevice;

enum class IOBusRegister {
    COMMAND = 0,
    STATUS = 1,
    DATA0 = 2,
    DATA1 = 3,
    DATA2 = 4,
    DATA3 = 5
};

enum class IOBusCommands {
    GET_BUS_INFO = 0,
    GET_DEVICE_INFO = 1,
    SET_DEVICE_INFO = 2
};

enum class IODeviceID {
    CONSOLE = 0,
    VIDEO = 1
};

struct [[gnu::packed]] IOBus_GetBusInfoResponse {
    uint64_t deviceCount;
};

struct [[gnu::packed]] IOBus_GetDeviceInfoResponse {
    uint64_t ID;
    uint64_t baseAddress; // 0 if not set
    uint64_t size;
};

struct [[gnu::packed]] IOBus_SetDeviceInfoRequest { // ID and size cannot be changed
    uint64_t ID; // device to set the data to
    uint64_t baseAddress;
};

struct [[gnu::packed]] IOBus_StatusRegister {
    bool commandComplete : 1;
    bool error           : 1;
    uint64_t reserved    : 62;
};

struct [[gnu::packed]] IOBusRegisters {
    uint64_t command;
    IOBus_StatusRegister status;
    uint64_t data[4];
    uint64_t reserved[2];
};

class IOBus {
   public:
    IOBus(MMU* mmu);
    ~IOBus();

    uint64_t ReadRegister(uint64_t offset);
    void WriteRegister(uint64_t offset, uint64_t data);

    bool AddDevice(IODevice* device);
    void RemoveDevice(IODevice* device);

   private:
    void Validate() const;

    void RunCommand(uint64_t command);

    IODevice* FindDevice(IODeviceID ID);

   private:
    MMU* m_MMU;
    IOBusRegisters m_registers;
    LinkedList::SimpleLinkedList<IODevice> m_devices;
};

extern IOBus* g_IOBus;

#endif /* _IO_BUS_HPP */