/*
Copyright (©) 2023-2024  Frosty515

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

#include "Emulator.hpp"

#include <Register.hpp>
#include <Stack.hpp>
#include <Interrupts.hpp>
#include <Exceptions.hpp>

#include <Instruction/Instruction.hpp>
#include <Instruction/Operand.hpp>

#include <IO/IOBus.hpp>

#include <IO/devices/ConsoleDevice.hpp>

#include <MMU/MMU.hpp>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

namespace Emulator {

    void EmulatorMain();

    Register* g_I[2]; // instruction pointers
    Register* g_SCP; // stack current pos
    Register* g_SBP; // stack base
    Register* g_STP; // stack top
    Register* g_GPR[16]; // general purpose registers
    Register* g_flags;  // flags register
    Register* g_Control[8]; // control registers

    ConsoleDevice* g_ConsoleDevice;

    uint64_t g_NextIP;

    MMU g_MMU;
    
    uint64_t g_RAMSize = 0;

    bool g_InstructionInProgress = false;
    
    uint64_t g_CurrentInstruction = 0;

    InstructionState g_CurrentState = InstructionState::OPCODE;
    uint64_t g_CurrentInstructionOffset = 0;

    bool g_EmulatorRunning = false;

    char const* last_error = nullptr;

    bool g_isInProtectedMode = false;
    bool g_AllowUserIO = false;
    bool g_isInUserMode = false;

    void HandleMemoryOperation(uint64_t address, void* data, uint64_t size, uint64_t count, bool write) {
        if (write) {
            for (uint64_t i = 0; i < count; i++) {
                switch (size) {
                case 1:
                    g_MMU.write8(address + i, ((uint8_t*)data)[i]);
                    break;
                case 2:
                    g_MMU.write16(address + i * 2, ((uint16_t*)data)[i]);
                    break;
                case 4:
                    g_MMU.write32(address + i * 4, ((uint32_t*)data)[i]);
                    break;
                case 8:
                    g_MMU.write64(address + i * 8, ((uint64_t*)data)[i]);
                    break;
                default:
                    printf("Invalid size: %lu\n", size);
                    abort();
                }
            }
        }
        else {
            for (uint64_t i = 0; i < count; i++) {
                switch (size) {
                case 1:
                    ((uint8_t*)data)[i] = g_MMU.read8(address + i);
                    break;
                case 2:
                    ((uint16_t*)data)[i] = g_MMU.read16(address + i * 2);
                    break;
                case 4:
                    ((uint32_t*)data)[i] = g_MMU.read32(address + i * 4);
                    break;
                case 8:
                    ((uint64_t*)data)[i] = g_MMU.read64(address + i * 8);
                    break;
                default:
                    printf("Invalid size: %lu\n", size);
                    abort();
                }
            }
        }
    }

    int Start(uint8_t* program, size_t size, const size_t RAMSize) {
        // Run some checks
        /*if (size > RAMSize)
            return SE_TOO_LITTLE_RAM;*/

        g_RAMSize = RAMSize;
        
        // Prepare RAM
        g_MMU.init(RAMSize);

        // Configure the exception handler
        g_ExceptionHandler = new ExceptionHandler();

        // Configure the interrupt handler
        g_InterruptHandler = new InterruptHandler(&g_MMU, g_ExceptionHandler);
        g_ExceptionHandler->SetINTHandler(g_InterruptHandler);

        // Configure the IO bus
        g_IOBus = new IOBus();

        // Configure the console device
        g_ConsoleDevice = new ConsoleDevice(0, 0xF);
        g_IOBus->AddDevice(g_ConsoleDevice);

        // Configure the stack
        g_stack = new Stack(&g_MMU, 0, 0, 0);

        // Load program into RAM
        g_MMU.WriteBuffer(0, program, size < RAMSize ? size : RAMSize); // temp copy size for debugging

        g_I[0] = new Register(RegisterType::Instruction, 0, false, 0); // explicitly initialise instruction pointer to 0
        g_NextIP = 0;

        g_EmulatorRunning = true;

        EmulatorMain();
        return SE_SUCCESS;
    }

    /*bool joinMain() {
        if (Emulator_Thread.joinable()) {
            Emulator_Thread.join();
            return true;
        }
        return false;
    }*/

    int RequestEmulatorStop() {
        return -1;
    }

    int SendInstruction(uint64_t instruction) {
        (void)instruction;
        return -1;
    }

    void DumpRegisters(FILE* fp) {
        fprintf(fp, "Registers:\n");
        fprintf(fp, "R0 =%016lx R1 =%016lx R2 =%016lx R3 =%016lx\n", g_GPR[0]->GetValue(), g_GPR[1]->GetValue(), g_GPR[2]->GetValue(), g_GPR[3]->GetValue());
        fprintf(fp, "R4 =%016lx R5 =%016lx R6 =%016lx R7 =%016lx\n", g_GPR[4]->GetValue(), g_GPR[5]->GetValue(), g_GPR[6]->GetValue(), g_GPR[7]->GetValue());
        fprintf(fp, "R8 =%016lx R9 =%016lx R10=%016lx R11=%016lx\n", g_GPR[8]->GetValue(), g_GPR[9]->GetValue(), g_GPR[10]->GetValue(), g_GPR[11]->GetValue());
        fprintf(fp, "R12=%016lx R13=%016lx R14=%016lx R15=%016lx\n", g_GPR[12]->GetValue(), g_GPR[13]->GetValue(), g_GPR[14]->GetValue(), g_GPR[15]->GetValue());
        fprintf(fp, "SCP=%016lx SBP=%016lx STP=%016lx\n", g_SCP->GetValue(), g_SBP->GetValue(), g_STP->GetValue());
        fprintf(fp, "I0 =%016lx I1 =%016lx\n", g_I[0]->GetValue(), g_I[1]->GetValue());
        fprintf(fp, "CR0=%016lx CR1=%016lx CR2=%016lx CR3=%016lx\n", g_Control[0]->GetValue(), g_Control[1]->GetValue(), g_Control[2]->GetValue(), g_Control[3]->GetValue());
        fprintf(fp, "CR4=%016lx CR5=%016lx CR6=%016lx CR7=%016lx\n", g_Control[4]->GetValue(), g_Control[5]->GetValue(), g_Control[6]->GetValue(), g_Control[7]->GetValue());
        fprintf(fp, "Flags = %016lx\n", g_flags->GetValue());
    }

    void DumpRAM(FILE* fp) {
        fprintf(fp, "RAM:");
        for (uint64_t i = 0; i < g_RAMSize; i++) {
            if ((i % 16) == 0)
                fprintf(fp, "\n%08lx: ", i);
            else if ((i % 8) == 0)
                fprintf(fp, " ");
            fprintf(fp, "%02x ", g_MMU.read8(i));
        }
        fprintf(fp, "\n");
    }

    Register* GetRegisterPointer(uint8_t ID) {
        uint8_t type = ID & 0xF;
        uint8_t index = (ID & 0xF0) >> 4;
        Register* returnVal = nullptr;
        switch (type) {
            case 0: // GPR
                returnVal = g_GPR[index];
                break;
            case 1: // stack
                switch (index) {
                    case 0:
                        returnVal = g_SCP;
                        break;
                    case 1:
                        returnVal = g_SBP;
                        break;
                    case 2:
                        returnVal = g_STP;
                        break;
                    default:
                        break;
                }
                break;
            case 2:
                if (index < 8)
                    returnVal = g_Control[index];
                else {
                    index -= 8;
                    switch (index) {
                        case 0: // flags
                            returnVal = g_flags;
                            break;
                        case 1: // I0
                            returnVal = g_I[0];
                            break;
                        case 2: // I1
                            returnVal = g_I[1];
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
        return returnVal;
    }

    uint64_t ReadRegister(uint8_t ID) {
        Register* pointer = GetRegisterPointer(ID);
        return pointer->GetValue();
    }

    bool WriteRegister(uint8_t ID, uint64_t value) {
        Register* pointer = GetRegisterPointer(ID);
        if (ID == 0x28 || ID == 0x29 || ID == 0x2A)
            return false;
        pointer->SetValue(value);
        return true;
    }

    void EmulatorMain() {
        // Initialise all the registers
        for (int i = 0; i < 16; i++)
            g_GPR[i] = new Register(RegisterType::GeneralPurpose, i, true);


        g_SCP = new Register(RegisterType::Stack, 0, true);
        g_SBP = new Register(RegisterType::Stack, 1, true);
        g_STP = new Register(RegisterType::Stack, 2, true);

        g_I[1] = new Register(RegisterType::Instruction, 1, false);

        for (int i = 0; i < 8; i++)
            g_Control[i] = new Register(RegisterType::Control, i, true);


        g_flags = new Register(RegisterType::Flags, 0, false);


        // run some checks
        if (g_I[0]->GetValue() % 8 != 0) {
            last_error = "Instruction Pointer is not 64-bit aligned";
            return;
        }

        SyncRegisters();

        // setup instruction stuff
        g_InstructionInProgress = false;

        // begin instruction loop.
        ExecuteInstruction(g_I[0]->GetValue(), g_MMU, g_CurrentState, last_error);

        // Dump the registers
        DumpRegisters(stdout);

        // Dump the RAM
        DumpRAM(stdout);

        // Delete all the registers
        for (int i = 0; i < 16; i++)
            delete g_GPR[i];

        delete g_SBP;
        delete g_SCP;
        delete g_STP;

        delete g_I[0];
        delete g_I[1];

        for (int i = 0; i < 8; i++)
            delete g_Control[i];

        delete g_flags;
    }

    void SetCPUFlags(uint64_t mask) {
        g_flags->SetValue(g_flags->GetValue() | mask, true);
    }

    void ClearCPUFlags(uint64_t mask) {
        g_flags->SetValue(g_flags->GetValue() & ~mask, true);
    }

    uint64_t GetCPUFlags() {
        return g_flags->GetValue();
    }

    void SetNextIP(uint64_t value) {
        g_NextIP = value;
    }

    uint64_t GetNextIP() {
        return g_NextIP;
    }

    void SetCPU_IP(uint64_t value) {
        g_I[0]->SetValue(value, true);
    }

    uint64_t GetCPU_IP() {
        return g_I[0]->GetValue();
    }

    void JumpToIP(uint64_t value) {
        SetCPU_IP(value);
        ExecuteInstruction(g_I[0]->GetValue(), g_MMU, g_CurrentState, last_error);
    }

    void SyncRegisters() {
        if (g_SBP->IsDirty()) {
            g_stack->setStackBase(g_SBP->GetValue());
            g_SBP->SetDirty(false);
        }
        else {
            g_SBP->SetValue(g_stack->getStackBase());
            g_SBP->SetDirty(false);
        }
        if (g_STP->IsDirty()) {
            g_stack->setStackTop(g_STP->GetValue());
            g_STP->SetDirty(false);
        }
        else {
            g_STP->SetValue(g_stack->getStackTop());
            g_STP->SetDirty(false);
        }
        if (g_SCP->IsDirty()) {
            g_stack->setStackPointer(g_SCP->GetValue());
            g_SCP->SetDirty(false);
        }
        else {
            g_SCP->SetValue(g_stack->getStackPointer());
            g_SCP->SetDirty(false);
        }
        if (g_Control[0]->IsDirty()) {
            g_isInProtectedMode = g_Control[0]->GetValue() & 1;
            g_AllowUserIO = g_Control[0]->GetValue() & 2;
            g_Control[0]->SetDirty(false);
        }
    }

    void Crash(const char* message) {
        g_EmulatorRunning = false;
        printf("Crash: %s\n", message);
        DumpRegisters(stdout);
        exit(0);
    }

    void HandleHalt() {
        g_EmulatorRunning = false;
        exit(0);
    }

    bool isInProtectedMode() {
        return g_isInProtectedMode;
    }

    bool isUserIOAllowed() {
        return g_AllowUserIO;
    }

    bool isInUserMode() {
        return g_isInUserMode;
    }

    void EnterUserMode() {
        uint64_t flags = g_flags->GetValue();
        g_flags->SetValue(g_Control[1]->GetValue(), true);
        g_Control[1]->SetValue(flags, true);
        g_NextIP = g_I[1]->GetValue();
        g_SCP->SetValue(g_GPR[15]->GetValue());
        g_isInUserMode = true;
    }

    void EnterUserMode(uint64_t address) {
        g_flags->SetValue(0, true);
        g_I[1]->SetValue(0, true);
        g_NextIP = address;
        g_isInUserMode = true;
    }

    void ExitUserMode() {
        g_isInUserMode = false;
        uint64_t flags = g_flags->GetValue();
        g_flags->SetValue(g_Control[1]->GetValue(), true);
        g_Control[1]->SetValue(flags, true);
        g_I[1]->SetValue(GetNextIP(), true);
        g_NextIP = g_Control[2]->GetValue();
        g_GPR[15]->SetValue(g_SCP->GetValue());
    }

    void WriteCharToConsole(char c) {
        fputc(c, stderr);
    }

    char ReadCharFromConsole() {
        return fgetc(stdin);
    }

}
