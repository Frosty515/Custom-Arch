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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <util.h>

#include <Emulator.hpp>

#define MAX_PROGRAM_FILE_SIZE 0x1000'0000
#define MIN_PROGRAM_FILE_SIZE 1

#define DEFAULT_RAM MiB(1)

/* Argument layout: <file name> [RAM size]*/

int main(int argc, char** argv) {

    if (argc < 2 || argc > 3) {
        printf("Usage: %s <file name> [RAM size]\n", argv[0]);
        return 1;
    }

    size_t RAM_Size;

    if (argc == 3)
        RAM_Size = strtoull(argv[2], nullptr, 0); // automatically detects base
    else
        RAM_Size = DEFAULT_RAM;

    // open and read file
    FILE* fp = fopen(argv[1], "r");
    if (fp == nullptr) {
        perror("fopen");
        return 1;
    }

    size_t fileSize;

    int rc = fseek(fp, 0, SEEK_END);
    if (rc != 0) {
        perror("fseek");
        return 1;
    }

    fileSize = ftell(fp);

    if (fileSize < MIN_PROGRAM_FILE_SIZE) {
        printf("File is too small to be a valid program.\n");
        return 1;
    }

    if (fileSize > MAX_PROGRAM_FILE_SIZE) {
        printf("File is too large to be a valid program.\n");
        return 1;
    }

    rc = fseek(fp, 0, SEEK_SET);
    if (rc != 0) {
        perror("fseek");
        return 1;
    }

    uint8_t* data = new uint8_t[fileSize];
    if (data == nullptr) {
        perror("new[]");
        return 1;
    }

    for (size_t i = 0; i < fileSize; i++) {
        int c = fgetc(fp);
        if (c == EOF)
            break;
        data[i] = (uint8_t)c;
    }

    fclose(fp);

    // Actually start emulator

    int status = Emulator::Start(data, fileSize, RAM_Size);
    if (status != 0) {
        printf("Emulator failed to start: %d\n", status);
        return 1;
    }

    // Cleanup

    delete[] data;
    

    return 0;
}