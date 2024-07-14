# Custom Architecture

## COPYING

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

Custom architecture

## Design

- 64-bit architecture

Full design can be found in [docs/design.md](docs/design.md)
The design of some internal aspects can be found in [docs/internals.md](docs/internals.md)

## Pre-requisites

### Requirements

- CMake
- Ninja
- A C/C++ compiler that supports C23 and C++23

### Linux (GNU C/C++)

#### Debian

- run `sudo apt update && sudo apt install build-essential cmake ninja-build` to install dependencies

#### Fedora/RHEL

- run `sudo dnf install gcc gcc-c++ cmake ninja-build` to install dependencies

#### Arch

- run `sudo pacman -Syu base-devel cmake ninja` to install dependencies

#### Gentoo

- run `sudo emerge --ask --verbose sys-devel/gcc sys-devel/binutils dev-build/cmake dev-build/ninja` to install dependencies

---

## Building

1. run `mkdir build && cd build` to create a build directory and enter it
2. run `cmake -GNinja ..` to generate the build files. `-DBUILD_CONFIG=<config>` can be added to the cmake command to specify the build type. `<config>` can be `Debug` or `Release`. It defaults to `Release`. `-DBUILD_ARCHITECTURE=<arch>` can be added to the cmake command to specify the architecture. Currently, the only supported architecture is `x86_64`. It defaults to `x86_64`.
3. run `ninja` to build

## Running the Assembler

- In the build directory, run `./Assembler/Assembler <path-to-assembly> <path-to-binary>` to assemble the assembly file

## Running the Emulator

- In the build directory, run `./Emulator/Emulator <path-to-binary>` to run the emulator

## Notes

- The assembler and emulator are still in development and may not work as expected.
