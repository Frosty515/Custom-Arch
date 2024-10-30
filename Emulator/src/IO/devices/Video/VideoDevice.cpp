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

#include "VideoDevice.hpp"

#include <stdint.h>

#include "backends/SDL/SDLVideoBackend.hpp"

void HandleVideoMemoryOperation(bool write, uint64_t address, uint8_t* buffer, size_t size, void* data) {
    VideoDevice* device = (VideoDevice*)data;
    device->HandleMemoryOperation(write, address, buffer, size);
}

VideoDevice::VideoDevice(uint64_t base_address, uint64_t size, VideoBackendType backendType, MMU& mmu) : IODevice(base_address, size), m_memoryRegion(nullptr), m_backendType(backendType), m_backend(nullptr), m_mmu(mmu), m_command(0), m_data(0), m_status(0), m_initialised(false), m_currentMode({ 0, 0, 0, 0, 0 }), m_currentModeIndex(0), m_modes({}) {
    
}

VideoDevice::~VideoDevice() {

}

void VideoDevice::Init() {

}

uint8_t VideoDevice::ReadByte(uint64_t address) {
    if (address == (uint64_t)VideoDevicePorts::DATA)
        return m_data & 0xFF;
    else if (address == (uint64_t)VideoDevicePorts::STATUS)
        return m_status & 0xFF;
    else
        return 0;
}

uint16_t VideoDevice::ReadWord(uint64_t address) {
    if (address == (uint64_t)VideoDevicePorts::DATA)
        return m_data & 0xFFFF;
    else if (address == (uint64_t)VideoDevicePorts::STATUS)
        return m_status & 0xFFFF;
    else
        return 0;
}

uint32_t VideoDevice::ReadDWord(uint64_t address) {
    if (address == (uint64_t)VideoDevicePorts::DATA)
        return m_data & 0xFFFFFFFF;
    else if (address == (uint64_t)VideoDevicePorts::STATUS)
        return m_status & 0xFFFFFFFF;
    else
        return 0;
}

uint64_t VideoDevice::ReadQWord(uint64_t address) {
    if (address == (uint64_t)VideoDevicePorts::DATA)
        return m_data;
    else if (address == (uint64_t)VideoDevicePorts::STATUS)
        return m_status;
    else
        return 0;
}

void VideoDevice::WriteByte(uint64_t address, uint8_t data) {
    if (address == (uint64_t)VideoDevicePorts::COMMAND) {
        m_command = data;
        HandleCommand();
    }
    else if (address == (uint64_t)VideoDevicePorts::DATA)
        m_data = data;
}

void VideoDevice::WriteWord(uint64_t address, uint16_t data) {
    if (address == (uint64_t)VideoDevicePorts::COMMAND) {
        m_command = data;
        HandleCommand();
    }
    else if (address == (uint64_t)VideoDevicePorts::DATA)
        m_data = data;
}

void VideoDevice::WriteDWord(uint64_t address, uint32_t data) {
    if (address == (uint64_t)VideoDevicePorts::COMMAND) {
        m_command = data;
        HandleCommand();
    }
    else if (address == (uint64_t)VideoDevicePorts::DATA)
        m_data = data;
}

void VideoDevice::WriteQWord(uint64_t address, uint64_t data) {
    if (address == (uint64_t)VideoDevicePorts::COMMAND) {
        m_command = data;
        HandleCommand();
    }
    else if (address == (uint64_t)VideoDevicePorts::DATA)
        m_data = data;
}

void VideoDevice::HandleMemoryOperation(bool write, uint64_t address, uint8_t* buffer, uint64_t size) {
    if (!m_initialised)
        return;

    if (write)
        m_backend->Write(address, buffer, size);
    else
        m_backend->Read(address, buffer, size);
}

void VideoDevice::HandleCommand() {
    switch ((VideoDeviceCommands)m_command) {
    case VideoDeviceCommands::INITIALISE: {
        if (m_initialised)
            return;
        switch (m_backendType) {
        case VideoBackendType::SDL: {
            // bring the backend online
            SDLVideoBackend* backend = new SDLVideoBackend(NATIVE_VIDEO_MODE);
            backend->Init();
            m_backend = backend;

            // fill the modes list
            m_modes.push_back(NATIVE_VIDEO_MODE);
            m_modes.push_back({ 640, 480, 60, 32, 640*4 });
            m_modes.push_back({ 800, 600, 60, 32, 800*4 });
            m_modes.push_back({1280, 720, 60, 32, 1280*4 });
            m_modes.push_back({1920, 1080, 60, 32, 1920*4 });

            // set the current mode. no need to set the backend mode as it's already set
            m_currentMode = NATIVE_VIDEO_MODE;
            m_currentModeIndex = 0;

            m_initialised = true;

            m_status = 0;
            break;
        }
        default:
            m_status = 1;
            break;
        }
        break;
    }
    case VideoDeviceCommands::GET_SCREEN_INFO: {
        if (!m_initialised) {
            m_status = 1;
            return;
        }

        VideoMode mode = NATIVE_VIDEO_MODE;

        VideoCommand::GetScreenInfoResponse response = { (uint32_t)mode.width, (uint32_t)mode.height, (uint16_t)mode.refreshRate, (uint16_t)mode.bpp, (uint16_t)m_modes.size(), (uint16_t)m_currentModeIndex};

        uint64_t* ptr = (uint64_t*)&response;

        m_mmu.write64(m_data, ptr[0]);
        m_mmu.write64(m_data + 8, ptr[1]);

        m_status = 0;
        break;
    }
    case VideoDeviceCommands::GET_MODE: {
        if (!m_initialised) {
            m_status = 1;
            return;
        }

        VideoCommand::GetModeRequest request;

        uint64_t* ptr = (uint64_t*)&request;

        ptr[0] = m_mmu.read64(m_data);
        ptr[1] = m_mmu.read64(m_data + 8);

        VideoMode mode = m_modes[request.index];

        VideoCommand::GetModeResponse response = { (uint32_t)mode.width, (uint32_t)mode.height, (uint16_t)mode.bpp, (uint32_t)mode.pitch, (uint16_t)mode.refreshRate };

        ptr = (uint64_t*)&response;

        m_mmu.write64(request.address, ptr[0]);
        m_mmu.write64(request.address + 8, ptr[1]);

        m_status = 0;
        break;
    }
    case VideoDeviceCommands::SET_MODE: {
        if (!m_initialised) {
            m_status = 1;
            return;
        }

        VideoCommand::SetModeRequest request;

        uint64_t* ptr = (uint64_t*)&request;

        ptr[0] = m_mmu.read64(m_data);
        ptr[1] = m_mmu.read64(m_data + 8);

        if (request.mode >= m_modes.size()) {
            printf("Invalid mode index: %d\n", request.mode);
            m_status = 1;
            return;
        }

        if (m_memoryRegion != nullptr) {
            // remove the old region
            uint64_t start = m_memoryRegion->getStart();
            uint64_t end = m_memoryRegion->getEnd();
            m_mmu.RemoveMemoryRegion(m_memoryRegion);
            delete m_memoryRegion;
            m_mmu.ReaddRegionSegment(start, end);
        }

        VideoMode mode = m_modes[request.mode];

        size_t size = mode.pitch * mode.height;

        if (!m_mmu.RemoveRegionSegment(request.address, request.address + size)) {
            printf("Failed to remove region segment\n");
            m_status = 1;
            return;
        }

        m_memoryRegion = new VideoMemoryRegion(request.address, request.address + size, HandleVideoMemoryOperation, this);
        m_mmu.AddMemoryRegion(m_memoryRegion);

        m_backend->SetMode(mode);

        m_currentMode = mode;
        m_currentModeIndex = request.mode;
        
        m_status = 0;

        break;
    }
    default:
        break;
    }
}
