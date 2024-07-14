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

#include <util.h>

#include <Emulator.hpp>
#include <FileUtil.hpp>

#define MAX_PROGRAM_FILE_SIZE MiB(25)
#define MIN_PROGRAM_FILE_SIZE 8

#define DEFAULT_RAM MiB(8)

/* Argument layout: <file name> [RAM size]*/

int main(int argc, char** argv) {

    if (argc < 2 || argc > 3) {
        printf("Usage: %s <file name> [RAM size]\n", argv[0]);
        return 1;
    }

    // check if file exists
    if (!FileUtil::isPathExist(argv[1])) {
        printf("File doesn't exist.\n");
        return 1;
    }

    // check if file is actually a file
    if (!FileUtil::isFile(argv[1])) {
        printf("Item isn't a file.\n");
        return 1;
    }

    size_t RAM_Size;

    if (argc == 3)
        RAM_Size = atoi(argv[2]);
    else
        RAM_Size = DEFAULT_RAM;

    // Check File size
    const size_t fileSize = FileUtil::fileSizeInBytes(argv[1]);

    if (fileSize > MAX_PROGRAM_FILE_SIZE) {
        printf("Max file size is 25MiB.\n");
        return 1;
    }

    if (fileSize < MIN_PROGRAM_FILE_SIZE) {
        printf("Min file size is 8B.\n");
        return 1;
    }

    /*if (fileSize % 8 != 0) {
        printf("Invalid file. Each instruction must be 8B.\n");
        return 1;
    }*/

    // open and read file
    FILE* fp = fopen(argv[1], "r");
    if (fp == nullptr) {
        perror("fopen");
        return 1;
    }

    uint8_t* data = (uint8_t*)malloc(fileSize);
    if (data == nullptr) {
        perror("malloc");
        return 1;
    }

    for (size_t i = 0; i < fileSize; i++) {
        int c = fgetc(fp);
        if (c == EOF)
            break;
        data[i] = (uint8_t)c;
    }

    fclose(fp); // done with file, so we can close it.

    // Actually start emulator

    int status = Emulator::Start(data, fileSize, RAM_Size);

    switch (status) {
        case Emulator::SE_SUCCESS:
            printf("Emulator has successfully started!\n");
            break;
        case Emulator::SE_MALLOC_FAIL:
            printf("malloc failed while starting emulator.\n");
            break;
        case Emulator::SE_TOO_LITTLE_RAM:
            printf("Too little RAM allocated to Emulator.\n");
            break;
        default:
            printf("Error %d occurred while starting emulator.\n", status);
            break;
    }

    // Cleanup

    free(data);
    

    return 0;
}