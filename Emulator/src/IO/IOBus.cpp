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

#include "IODevice.hpp"
#include "IOMemoryRegion.hpp"

#ifdef EMULATOR_DEBUG
#include <stdio.h>
#endif

#include <Emulator.hpp>
#include <Exceptions.hpp>

IOBus* g_IOBus = nullptr;

IOBus::IOBus(MMU* mmu)
    : m_MMU(mmu), m_registers{0, {true, false, 0}, {0, 0, 0, 0}, {0, 0}} {
}

IOBus::~IOBus() {
}

uint64_t IOBus::ReadRegister(uint64_t offset) {
#ifdef EMULATOR_DEBUG
    printf("IOBus::ReadRegister(%lu)\n", offset);
#endif
    Validate();
    switch (static_cast<IOBusRegister>(offset)) {
    case IOBusRegister::COMMAND:
        return 0;
    case IOBusRegister::STATUS: {
        uint64_t* temp = reinterpret_cast<uint64_t*>(&m_registers);
        return temp[1];
    }
    case IOBusRegister::DATA0:
    case IOBusRegister::DATA1:
    case IOBusRegister::DATA2:
    case IOBusRegister::DATA3:
        return m_registers.data[offset - static_cast<uint64_t>(IOBusRegister::DATA0)];
    }
    return 0;
}

void IOBus::WriteRegister(uint64_t offset, uint64_t data) {
#ifdef EMULATOR_DEBUG
    printf("IOBus::WriteRegister(%lu, %lu)\n", offset, data);
#endif
    Validate();
    switch (static_cast<IOBusRegister>(offset)) {
    case IOBusRegister::COMMAND:
        RunCommand(data);
        break;
    case IOBusRegister::STATUS: {
        IOBus_StatusRegister* temp = reinterpret_cast<IOBus_StatusRegister*>(&m_registers);
        m_registers.status = *temp;
        break;
    }
    case IOBusRegister::DATA0:
    case IOBusRegister::DATA1:
    case IOBusRegister::DATA2:
    case IOBusRegister::DATA3:
        m_registers.data[offset - static_cast<uint64_t>(IOBusRegister::DATA0)] = data;
        break;
    }
}

bool IOBus::AddDevice(IODevice* device) {
    if (FindDevice(device->GetID()) != nullptr)
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

void IOBus::RunCommand(uint64_t command) {
    IOBusCommands cmd = static_cast<IOBusCommands>(command);
    m_registers.status.error = false;
    m_registers.status.commandComplete = false;
    switch (cmd) {
    case IOBusCommands::GET_BUS_INFO: {
        IOBus_GetBusInfoResponse response{m_devices.getCount()};
        m_registers.data[0] = *reinterpret_cast<uint64_t*>(&response);
        break;
    }
    case IOBusCommands::GET_DEVICE_INFO: {
        // data[0] = index
        uint64_t index = m_registers.data[0];
        if (index >= m_devices.getCount()) {
            m_registers.status.error = true;
            break;
        }
        IODevice* device = m_devices.get(index);
        if (device == nullptr) {
            m_registers.status.error = true;
            break;
        }
        IOBus_GetDeviceInfoResponse response;
        response.ID = static_cast<uint64_t>(device->GetID());
        response.baseAddress = device->GetBaseAddress();
        response.size = device->GetSize();
        m_registers.data[0] = *reinterpret_cast<uint64_t*>(&response);
        break;
    }
    case IOBusCommands::SET_DEVICE_INFO: {
        // data[0] = ID
        // data[1] = baseAddress
        IODeviceID ID = static_cast<IODeviceID>(m_registers.data[0]);
        uint64_t baseAddress = m_registers.data[1];
        IODevice* device = FindDevice(ID);
        if (device == nullptr) {
            m_registers.status.error = true;
            break;
        }
        if (uint64_t oldBaseAddress = device->GetBaseAddress(); oldBaseAddress != 0) {
            // need to delete the old region
            if (IOMemoryRegion* region = device->GetMemoryRegion(); region != nullptr) {
                uint64_t start = region->getStart();
                uint64_t end = region->getEnd();
                m_MMU->RemoveMemoryRegion(region);
                delete region;
                m_MMU->ReaddRegionSegment(start, end);
            }
        }
        device->SetBaseAddress(baseAddress);
        if (baseAddress != 0) {
            uint64_t start = baseAddress;
            uint64_t end = baseAddress + device->GetSize() * 8;
            if (!m_MMU->RemoveRegionSegment(start, end)) {
                m_registers.status.error = true;
                break;
            }
            IOMemoryRegion* region = new IOMemoryRegion(start, end, device);
            m_MMU->AddMemoryRegion(region);
            device->SetMemoryRegion(region);
        }
        break;
    }
    }
    m_registers.status.commandComplete = true;
}

IODevice* IOBus::FindDevice(IODeviceID ID) {
    for (uint64_t i = 0; i < m_devices.getCount(); i++) {
        if (IODevice* device = m_devices.get(i); device->GetID() == ID)
            return device;
    }
    return nullptr;
}
