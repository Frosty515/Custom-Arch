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

#ifndef _FILE_UTIL_HPP
#define _FILE_UTIL_HPP

#include <stddef.h>
#include <string>

namespace FileUtil {

    size_t fileSizeInBytes(const char* path);
    size_t fileSizeInBytes(const std::string& path);
    bool isPathExist(const std::string& path);
    bool isPathExist(const char* path);
    bool isDirectory(const std::string& path);
    bool isDirectory(const char* path);
    bool isFile(const std::string& path);
    bool isFile(const char* path);

}

#endif /* _FILE_UTIL_HPP */