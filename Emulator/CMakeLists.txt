# Copyright (©) 2024  Frosty515

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

if(BUILD_ARCHITECTURE STREQUAL "x86_64")
    set(emulator_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/ALUInstruction.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/spinlock.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/util.asm
    )
endif()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(emulator_sources
        ${emulator_sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/OSSpecific/Linux/Memory.cpp
    )
endif()

set(emulator_sources
    ${emulator_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data-structures/Buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data-structures/LinkedList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Instruction/Instruction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Instruction/Operand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IO/IOBus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IO/IOMemoryRegion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IO/devices/ConsoleDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MMU/MemoryRegion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MMU/MMU.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MMU/StandardMemoryRegion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Emulator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Interrupts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Register.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Stack.cpp
)

add_executable(Emulator ${emulator_sources})

set_target_properties(Emulator PROPERTIES CXX_STANDARD 23)
set_target_properties(Emulator PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(Emulator PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(Emulator PROPERTIES C_STANDARD 23)
set_target_properties(Emulator PROPERTIES C_EXTENSIONS OFF)
set_target_properties(Emulator PROPERTIES C_STANDARD_REQUIRED ON)

target_include_directories(Emulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (BUILD_CONFIG STREQUAL "Debug")
    target_compile_definitions(Emulator PRIVATE EMULATOR_DEBUG=1)
endif()

target_compile_options(Emulator
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-g>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wpedantic>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-packed-bitfield-compat>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)
