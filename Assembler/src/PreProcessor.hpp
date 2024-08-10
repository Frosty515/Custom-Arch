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

#ifndef _PREPROCESSOR_HPP
#define _PREPROCESSOR_HPP

#include <stddef.h>

#include "Buffer.hpp"

class PreProcessor {
public:
    PreProcessor();
    ~PreProcessor();

    void process(const char* source, size_t source_size);

    size_t GetProcessedBufferSize() const;
    void ExportProcessedBuffer(uint8_t* data) const;

private:
    char* GetLine(char* source, size_t source_size, size_t& line_size);

private:
    Buffer m_buffer;
    uint64_t m_current_offset;
};

#endif /* _PREPROCESSOR_HPP */