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

#ifndef _OS_SPECIFIC_FILE_HPP
#define _OS_SPECIFIC_FILE_HPP

#include <stddef.h>

#ifdef __unix__
typedef int FileHandle_t;
#endif /* __unix__ */

FileHandle_t OpenFile(const char* path);
void CloseFile(FileHandle_t handle);
size_t GetFileSize(FileHandle_t handle);

size_t ReadFile(FileHandle_t handle, void* buffer, size_t size, size_t offset);
size_t WriteFile(FileHandle_t handle, const void* buffer, size_t size, size_t offset);

void* MapFile(FileHandle_t handle, size_t size, size_t offset);
void UnmapFile(void* address, size_t size);


#endif /* _OS_SPECIFIC_FILE_HPP */