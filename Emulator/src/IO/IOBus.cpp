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

#include "IOBus.hpp"
#include "IO/IODevice.hpp"

#ifdef EMULATOR_DEBUG
#include <stdio.h>
#endif

#include <Emulator.hpp>
#include <Exceptions.hpp>

IOBus* g_IOBus = nullptr;

IOBus::IOBus() {
}

IOBus::~IOBus() {
}

uint8_t IOBus::ReadByte(uint64_t address) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::ReadByte(%lu)\n", address);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return 0;
    return device->ReadByte(address - device->GetBaseAddress());
}

uint16_t IOBus::ReadWord(uint64_t address) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::ReadWord(%lu)\n", address);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return 0;
    return device->ReadWord(address - device->GetBaseAddress());
}

uint32_t IOBus::ReadDWord(uint64_t address) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::ReadDWord(%lu)\n", address);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return 0;
    return device->ReadDWord(address - device->GetBaseAddress());
}

uint64_t IOBus::ReadQWord(uint64_t address) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::ReadQWord(%lu)\n", address);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return 0;
    return device->ReadQWord(address - device->GetBaseAddress());
}

void IOBus::WriteByte(uint64_t address, uint8_t data) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::WriteByte(%lu, %hhu)\n", address, data);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return;
    device->WriteByte(address - device->GetBaseAddress(), data);
}

void IOBus::WriteWord(uint64_t address, uint16_t data) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::WriteWord(%lu, %hu)\n", address, data);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return;
    device->WriteWord(address - device->GetBaseAddress(), data);
}

void IOBus::WriteDWord(uint64_t address, uint32_t data) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::WriteDWord(%lu, %u)\n", address, data);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return;
    device->WriteDWord(address - device->GetBaseAddress(), data);
}

void IOBus::WriteQWord(uint64_t address, uint64_t data) {
    Validate();
#ifdef EMULATOR_DEBUG
    printf("IOBus::WriteQWord(%lu, %lu)\n", address, data);
#endif
    IODevice* device = FindDevice(address);
    if (device == nullptr)
        return;
    device->WriteQWord(address - device->GetBaseAddress(), data);
}

bool IOBus::AddDevice(IODevice* device) {
    if (FindDevice(device->GetBaseAddress(), device->GetSize()) != nullptr)
        return false;
    m_devices.insert(device);
    return true;
}

void IOBus::RemoveDevice(IODevice* device) {
    m_devices.remove(device);
}

void IOBus::Validate() const {
    if (Emulator::isInProtectedMode() && Emulator::isInUserMode())
        g_ExceptionHandler->RaiseException(Exception::USER_MODE_VIOLATION);
}

IODevice* IOBus::FindDevice(uint64_t address) {
    for (uint64_t i = 0; i < m_devices.getCount(); i++) {
        IODevice* device = m_devices.get(i);
        if (device->GetBaseAddress() <= address && (device->GetBaseAddress() + device->GetSize()) > address)
            return device;
    }
    return nullptr;
}

IODevice* IOBus::FindDevice(uint64_t address, uint64_t size) {
    for (uint64_t i = 0; i < m_devices.getCount(); i++) {
        IODevice* device = m_devices.get(i);
        if (device->GetBaseAddress() <= address && (device->GetBaseAddress() + device->GetSize()) >= (address + size))
            return device;
    }
    return nullptr;
}
