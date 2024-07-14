/*
Copyright (©) 2023  Frosty515

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

#include "FileUtil.hpp"

#include <sys/stat.h>

namespace FileUtil {

    size_t fileSizeInBytes(const char* path) {
        struct stat stat_buf;
        int rc = stat(path, &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
    }

    size_t fileSizeInBytes(const std::string& path) {
        struct stat stat_buf;
        int rc = stat(path.c_str(), &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
    }

    bool isPathExist(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

    bool isPathExist(const char* path) {
        struct stat buffer;
        return (stat(path, &buffer) == 0);
    }

    bool isDirectory(const std::string& path) {
        struct stat s;
        if (stat(path.c_str(), &s) == 0) {
            if (s.st_mode & S_IFDIR) return true;
        }
        return false;
    }

    bool isDirectory(const char* path) {
        struct stat s;
        if (stat(path, &s) == 0) {
            if (s.st_mode & S_IFDIR) return true;
        }
        return false;
    }

    bool isFile(const std::string& path) {
        struct stat s;
        if (stat(path.c_str(), &s) == 0){
            if (s.st_mode & S_IFREG) return true;
        }
        return false;
    }

    bool isFile(const char* path) {
        struct stat s;
        if (stat(path, &s) == 0) {
            if (s.st_mode & S_IFREG) return true;
        }
        return false;
    }

}
