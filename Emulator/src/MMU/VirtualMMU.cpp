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

#include "VirtualMMU.hpp"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

#include <Emulator.hpp>
#include <Exceptions.hpp>

#include "MMU/StandardMemoryRegion.hpp"

VirtualMMU::VirtualMMU(MMU* physicalMMU, uint64_t pageTableRoot, PageSize pageSize, PageTableLevelCount pageTableLevelCount)
    : m_physicalMMU(physicalMMU), m_pageTableRoot(pageTableRoot), m_pageSize(pageSize), m_pageTableLevelCount(pageTableLevelCount) {
    assert(m_physicalMMU != nullptr);
}

VirtualMMU::~VirtualMMU() {
}

void VirtualMMU::ReadBuffer(uint64_t address, uint8_t* data, size_t size) {
    m_physicalMMU->ReadBuffer(TranslateAddress(address, PageTranslateMode::Read), data, size);
}

void VirtualMMU::WriteBuffer(uint64_t address, const uint8_t* data, size_t size) {
    m_physicalMMU->WriteBuffer(TranslateAddress(address, PageTranslateMode::Write), data, size);
}

uint8_t VirtualMMU::read8(uint64_t address) {
    return m_physicalMMU->read8(TranslateAddress(address, PageTranslateMode::Read));
}

uint16_t VirtualMMU::read16(uint64_t address) {
    return m_physicalMMU->read16(TranslateAddress(address, PageTranslateMode::Read));
}

uint32_t VirtualMMU::read32(uint64_t address) {
    return m_physicalMMU->read32(TranslateAddress(address, PageTranslateMode::Read));
}

uint64_t VirtualMMU::read64(uint64_t address) {
    return m_physicalMMU->read64(TranslateAddress(address, PageTranslateMode::Read));
}

void VirtualMMU::write8(uint64_t address, uint8_t data) {
    m_physicalMMU->write8(TranslateAddress(address, PageTranslateMode::Write), data);
}

void VirtualMMU::write16(uint64_t address, uint16_t data) {
    m_physicalMMU->write16(TranslateAddress(address, PageTranslateMode::Write), data);
}

void VirtualMMU::write32(uint64_t address, uint32_t data) {
    m_physicalMMU->write32(TranslateAddress(address, PageTranslateMode::Write), data);
}

void VirtualMMU::write64(uint64_t address, uint64_t data) {
    m_physicalMMU->write64(TranslateAddress(address, PageTranslateMode::Write), data);
}

bool VirtualMMU::ValidateRead(uint64_t address, size_t size) {
    uint64_t pageSize = 0;
    switch (m_pageSize) {
    case PS_4KiB:
        pageSize = 4'096;
        break;
    case PS_16KiB:
        pageSize = 16'384;
        break;
    case PS_64KiB:
        pageSize = 65'536;
        break;
    default:
        Emulator::Crash("Invalid page size");
    }
    uint64_t start = ALIGN_DOWN_BASE2(address, pageSize);
    uint64_t end = ALIGN_UP_BASE2(address + size, pageSize);
    for (uint64_t i = start; i < end; i += pageSize) {
        bool success = false;
        TranslateAddress(i, PageTranslateMode::Read, true, &success);
        if (!success)
            return false;
    }
    return true;
}

bool VirtualMMU::ValidateExecute(uint64_t address, size_t size) {
    uint64_t pageSize = 0;
    switch (m_pageSize) {
    case PS_4KiB:
        pageSize = 4'096;
        break;
    case PS_16KiB:
        pageSize = 16'384;
        break;
    case PS_64KiB:
        pageSize = 65'536;
        break;
    default:
        Emulator::Crash("Invalid page size");
    }
    uint64_t start = ALIGN_DOWN_BASE2(address, pageSize);
    uint64_t end = ALIGN_UP_BASE2(address + size, pageSize);
    for (uint64_t i = start; i < end; i += pageSize) {
        bool success = false;
        TranslateAddress(i, PageTranslateMode::Execute, true, &success);
        if (!success)
            return false;
    }
    return true;
}

void VirtualMMU::AddMemoryRegion(MemoryRegion* region) {
    (void)region;
}

void VirtualMMU::RemoveMemoryRegion(MemoryRegion* region) {
    (void)region;
}

void VirtualMMU::DumpMemory() const {
}

bool VirtualMMU::RemoveRegionSegment(uint64_t start, uint64_t end) {
    (void)start;
    (void)end;
    return false;
}

bool VirtualMMU::ReaddRegionSegment(uint64_t start, uint64_t end) {
    (void)start;
    (void)end;
    return false;
}

void VirtualMMU::SetPageTableRoot(uint64_t pageTableRoot) {
    m_pageTableRoot = pageTableRoot;
}

uint64_t VirtualMMU::TranslateAddress(uint64_t address, PageTranslateMode mode, bool safe, bool* success) const {
    uint8_t levelCount = 0;
    switch (m_pageTableLevelCount) {
    case PTLC_3:
        levelCount = 3;
        break;
    case PTLC_4:
        levelCount = 4;
        break;
    case PTLC_5:
        levelCount = 5;
        break;
    default:
        assert(false); // deal with error handling later...
    }
    uint64_t page = 0;
    uint16_t offset = 0;
    uint8_t pageShift = 0;
    switch (m_pageSize) {
    case PS_4KiB:
        page = address >> 12;
        offset = address & 0xFFF;
        pageShift = 12;
        break;
    case PS_16KiB:
        page = address >> 14;
        offset = address & 0x3FFF;
        pageShift = 14;
        break;
    case PS_64KiB:
        page = address >> 16;
        offset = address & 0xFFFF;
        pageShift = 16;
        break;
    default:
        assert(false); // deal with error handling later...
    }
    bool inUserMode = Emulator::isInProtectedMode() && Emulator::isInUserMode();
    PageTableEntry table;
    uint64_t physicalAddress = 0;
    for (uint8_t i = 0; i < levelCount; i++) {
        uint64_t index = (page >> (10 * (levelCount - i - 1))) & 0x3FF;
        if (i == 0) {
            // need to fetch the table from guest memory
            if (!m_physicalMMU->ValidateRead(m_pageTableRoot + index * 8, 8))
                g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, m_pageTableRoot + index * 8);
            {
                uint64_t raw = m_physicalMMU->read64(m_pageTableRoot + index * 8);
                // printf("Table: %lx\n", raw);
                PageTableEntry* temp = reinterpret_cast<PageTableEntry*>(&raw);
                table = *temp;
            }
        } else if (table.Lowest) {
            if (safe && success != nullptr)
                *success = true;
            return ((table.PhysicalAddress | (page & ((1 << (10 * (i - 1))) - 1))) << pageShift) | offset;
        } else if (!GetNextTableLevel(table, index, &table)) {
            if (safe) {
                if (success != nullptr)
                    *success = false;
                return 0;
            }
            PagingViolationErrorCode code = {false, false, false, false, false, false, 0};
            code.read = mode == PageTranslateMode::Read;
            code.write = mode == PageTranslateMode::Write;
            code.execute = mode == PageTranslateMode::Execute;
            code.user = inUserMode;
            g_ExceptionHandler->RaiseException(Exception::PAGING_VIOLATION, address, code);
        }
        if (!table.Present) {
            if (safe) {
                if (success != nullptr)
                    *success = false;
                return 0;
            }
            PagingViolationErrorCode code = {false, false, false, false, false, false, 0};
            code.read = mode == PageTranslateMode::Read;
            code.write = mode == PageTranslateMode::Write;
            code.execute = mode == PageTranslateMode::Execute;
            code.user = inUserMode;
            g_ExceptionHandler->RaiseException(Exception::PAGING_VIOLATION, address, code);
        }
        if ((!table.Readable && mode == PageTranslateMode::Read) || (!table.Writable && mode == PageTranslateMode::Write) || (!table.Executable && mode == PageTranslateMode::Execute) || (inUserMode && !table.User)) {
            if (safe) {
                if (success != nullptr)
                    *success = false;
                return 0;
            }
            PagingViolationErrorCode code = {false, false, false, false, false, false, 0};
            code.read = mode == PageTranslateMode::Read;
            code.write = mode == PageTranslateMode::Write;
            code.execute = mode == PageTranslateMode::Execute;
            code.user = inUserMode;
            g_ExceptionHandler->RaiseException(Exception::PAGING_VIOLATION, address, code);
        }
        physicalAddress = table.PhysicalAddress;
    }
    if (safe && success != nullptr)
        *success = true;
    return (physicalAddress << pageShift) | offset;
}

bool VirtualMMU::GetNextTableLevel(PageTableEntry table, uint64_t tableIndex, PageTableEntry* out) const { // tableIndex = index within table
    assert(tableIndex < 1'024);

    if (!table.Present)
        return false;

    if (table.Lowest)
        return false; // should already be handled by the caller

    uint64_t pageSizeShift = 0;
    switch (m_pageSize) {
    case PS_4KiB:
        pageSizeShift = 12;
        break;
    case PS_16KiB:
        pageSizeShift = 14;
        break;
    case PS_64KiB:
        pageSizeShift = 16;
        break;
    }

    if (!m_physicalMMU->ValidateRead(((uint64_t)table.PhysicalAddress << pageSizeShift) + tableIndex * 8, 8))
        g_ExceptionHandler->RaiseException(Exception::PHYS_MEM_VIOLATION, ((uint64_t)table.PhysicalAddress << pageSizeShift) + tableIndex * 8);

    uint64_t raw = m_physicalMMU->read64(((uint64_t)table.PhysicalAddress << pageSizeShift) + tableIndex * 8);
    PageTableEntry* temp = reinterpret_cast<PageTableEntry*>(&raw);
    if (out != nullptr)
        *out = *temp;
    return true;
}

[[noreturn]] void TestVMMU() {
    MMU* physicalMMU = new MMU();

    physicalMMU->AddMemoryRegion(new StandardMemoryRegion(0, 16 * 1'024 * 1'024)); // 16 MiB

    // 0x000000-0x400000 - test region - gets identity mapped
    // 0x400000-0x402000 - Level 5 table
    // 0x402000-0x404000 - Level 4 table
    // 0x404000-0x406000 - Level 3 table
    // 0x406000-0x408000 - Level 2 table
    // 0x408000-0x40A000 - Level 1 table

    PageTableEntry* level5 = new PageTableEntry[1'024];
    PageTableEntry* level4 = new PageTableEntry[1'024];
    PageTableEntry* level3 = new PageTableEntry[1'024];
    PageTableEntry* level2 = new PageTableEntry[1'024];
    PageTableEntry* level1 = new PageTableEntry[1'024];

    memset(level5, 0, 1'024 * sizeof(PageTableEntry));
    memset(level4, 0, 1'024 * sizeof(PageTableEntry));
    memset(level3, 0, 1'024 * sizeof(PageTableEntry));
    memset(level2, 0, 1'024 * sizeof(PageTableEntry));
    memset(level1, 0, 1'024 * sizeof(PageTableEntry));

    for (uint64_t i = 0; i < 1'024; i++) {
        level5[i].Present = true;
        level5[i].Readable = true;
        level5[i].Writable = true;
        level5[i].Lowest = false;
        level5[i].PhysicalAddress = 0x402000 + i * 8;
        level4[i].Present = true;
        level4[i].Readable = true;
        level4[i].Writable = true;
        level4[i].Lowest = false;
        level4[i].PhysicalAddress = 0x404000 + i * 8;
        level3[i].Present = true;
        level3[i].Readable = true;
        level3[i].Writable = true;
        level3[i].Lowest = false;
        level3[i].PhysicalAddress = 0x406000 + i * 8;
        level2[i].Present = true;
        level2[i].Readable = true;
        level2[i].Writable = true;
        level2[i].Lowest = false;
        level2[i].PhysicalAddress = 0x408000 + i * 8;
        level1[i].Present = true;
        level1[i].Readable = true;
        level1[i].Writable = true;
        level1[i].Lowest = true;
        level1[i].PhysicalAddress = i;
    }

    physicalMMU->WriteBuffer(0x400000, reinterpret_cast<uint8_t*>(level5), 8 * 1'024);
    physicalMMU->WriteBuffer(0x402000, reinterpret_cast<uint8_t*>(level4), 8 * 1'024);
    physicalMMU->WriteBuffer(0x404000, reinterpret_cast<uint8_t*>(level3), 8 * 1'024);
    physicalMMU->WriteBuffer(0x406000, reinterpret_cast<uint8_t*>(level2), 8 * 1'024);
    physicalMMU->WriteBuffer(0x408000, reinterpret_cast<uint8_t*>(level1), 8 * 1'024);

    MMU* virtualMMU = new VirtualMMU(physicalMMU, 0x400000, PS_4KiB, PTLC_5);

    for (uint64_t i = 0; i < 0x400000; i += 8) {
        virtualMMU->write64(i, i);
        assert(virtualMMU->read64(i) == i);
    }

    physicalMMU->DumpMemory();

    exit(0);
}
