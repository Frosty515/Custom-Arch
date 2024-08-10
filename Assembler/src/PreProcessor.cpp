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

#include "PreProcessor.hpp"
#include <cstdio>

PreProcessor::PreProcessor() : m_buffer(), m_current_offset(0) {

}

PreProcessor::~PreProcessor() {

}

void PreProcessor::process(const char* source, size_t source_size) {
    // 1. remove single line comments starting with ;
    char const* line = source;
    while (line != nullptr) {
        size_t line_size = 0;
        char* next_line = GetLine((char*)line, source_size - m_current_offset, line_size);
        if (line_size != 0) {
            for (size_t i = 0; i < line_size; i++) {
                if (line[i] == ';') {
                    source_size -= line_size - i;
                    line_size = i;
                    break;
                }
            }
            m_buffer.Write(m_current_offset, (const uint8_t*)line, line_size);
            m_current_offset += line_size;
        }
        char c = '\n';
        m_buffer.Write(m_current_offset, (const uint8_t*)&c, 1);
        m_current_offset++;
        line = next_line;
    }

    // export the buffer to a char const* and size
    size_t source2_size = m_current_offset;
    char const* source2;
    {
        char* source2_i = new char[source2_size+1];
        m_buffer.Read(0, (uint8_t*)source2_i, source2_size);
        source2_i[source2_size] = '\0';
        source2 = source2_i;
    }

    // clear the buffer
    m_buffer.Clear();
    m_current_offset = 0;

    // 2. remove multi line comments starting with /* and ending with */
    bool in_comment = false;
    for (size_t i = 0; i < source2_size; i++) {
        if (source2[i] == '/' && source2[i + 1] == '*' && !in_comment) {
            in_comment = true;
            i++;
        }
        else if (source2[i] == '*' && source2[i + 1] == '/' && in_comment) {
            in_comment = false;
            i++;
        }
        else if (!in_comment) {
            m_buffer.Write(m_current_offset, (const uint8_t*)&source2[i], 1);
            m_current_offset++;
        }
    }
    
    delete[] source2;
}

size_t PreProcessor::GetProcessedBufferSize() const {
    return m_current_offset;
}

void PreProcessor::ExportProcessedBuffer(uint8_t* data) const {
    m_buffer.Read(0, data, m_current_offset);
}

char* PreProcessor::GetLine(char* source, size_t source_size, size_t& line_size) {
    for (size_t i = 0; i < source_size; i++) {
        if (source[i] == '\n') {
            line_size = i;
            return &(source[i+1]);
        }
    }
    line_size = source_size;
    return nullptr;
}
