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
#include <cstdlib>
#include <cstring>

PreProcessor::PreProcessor() : m_buffer(), m_current_offset(0) {

}

PreProcessor::~PreProcessor() {

}

void PreProcessor::process(const char* source, size_t source_size) {
    // 1. resolve includes in %include "file" format
    {
        char* source_i = new char[source_size+1];
        strncpy((char*)source_i, source, source_size);
        source_i[source_size] = '\0';
        source = source_i;
    }

    char* include_start = (char*)source;
    char* include_end = nullptr;
    char const* i_source = source;
    while (include_start != nullptr) {
        include_start = strstr(include_start, "%include \"");
        if (include_start != nullptr) {
            m_buffer.Write(m_current_offset, (const uint8_t*)i_source, include_start - i_source);
            m_current_offset += include_start - i_source;
            include_start += 10;
            include_end = strchr(include_start, '"');
            if (include_end != nullptr) {
                char* include_file = new char[include_end - include_start + 1];
                strncpy(include_file, include_start, include_end - include_start);
                include_file[include_end - include_start] = '\0';
                FILE* file = fopen(include_file, "r"); // FIXME: should be relative to the source file
                if (file != nullptr) {
                    fseek(file, 0, SEEK_END);
                    size_t file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    char* file_data = new char[file_size];
                    fread(file_data, 1, file_size, file);
                    fclose(file);
                    PreProcessor preprocessor;
                    preprocessor.process(file_data, file_size);
                    size_t processed_size = preprocessor.GetProcessedBufferSize();
                    uint8_t* processed_data = new uint8_t[processed_size];
                    preprocessor.ExportProcessedBuffer(processed_data);
                    m_buffer.Write(m_current_offset, processed_data, processed_size);
                    m_current_offset += processed_size;
                    delete[] processed_data;
                    delete[] file_data;
                }
                else
                    error("Could not open include file");
                delete[] include_file;
                i_source = include_end + 1;
            }
            else
                error("Unterminated include directive");
        }
    }
    const char* original_source = source;
    // read the rest of the source
    if (include_end == nullptr)
        include_end = (char*)source;
    else
        include_end += 1;
    m_buffer.Write(m_current_offset, (const uint8_t*)include_end, source_size - (include_end - source));
    m_current_offset += source_size - (include_end - source);

    delete[] original_source;

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

    const char* original_source2 = source2;

    // 2. remove single line comments starting with ;
    while (true) {
        char const* comment_start = strchr(source2, ';');
        if (comment_start == nullptr) {
            m_buffer.Write(m_current_offset, (const uint8_t*)source2, source2_size - (source2 - original_source2));
            m_current_offset += source2_size - (source2 - original_source2);
            break;
        }
        else {
            m_buffer.Write(m_current_offset, (const uint8_t*)source2, comment_start - source2);
            m_current_offset += comment_start - source2;
            source2 = comment_start;
            char const* comment_end = strchr(source2, '\n');
            if (comment_end == nullptr)
                error("Unterminated comment");
            else
                source2 = comment_end;
        }
    }

    delete[] original_source2;

    // export the buffer to a char const* and size
    size_t source3_size = m_current_offset;
    char const* source3;
    {
        char* source3_i = new char[source3_size+1];
        m_buffer.Read(0, (uint8_t*)source3_i, source3_size);
        source3_i[source3_size] = '\0';
        source3 = source3_i;
    }

    // clear the buffer
    m_buffer.Clear();
    m_current_offset = 0;

    const char* original_source3 = source3;

    // 3. remove multi line comments starting with /* and ending with */
    while (true) {
        char const* comment_start = strstr(source3, "/*");
        if (comment_start == nullptr) {
            m_buffer.Write(m_current_offset, (const uint8_t*)source3, source3_size - (source3 - original_source3));
            m_current_offset += source3_size - (source3 - original_source3);
            break;
        }
        else {
            m_buffer.Write(m_current_offset, (const uint8_t*)source3, comment_start - source3);
            m_current_offset += comment_start - source3;
            source3 = comment_start;
            char const* comment_end = strstr(source3, "*/");
            if (comment_end == nullptr)
                error("Unterminated comment");
            else
                source3 = comment_end + 2;
        }
    }
    
    delete[] original_source3;
}

size_t PreProcessor::GetProcessedBufferSize() const {
    return m_current_offset;
}

void PreProcessor::ExportProcessedBuffer(uint8_t* data) const {
    m_buffer.Read(0, data, m_current_offset);
}

char* PreProcessor::GetLine(char* source, size_t source_size, size_t& line_size) {
    void* line = memchr(source, '\n', source_size);
    if (line != nullptr) {
        line_size = (char*)line - source;
        return (char*)line + 1;
    }
    // for (size_t i = 0; i < source_size; i++) {
    //     if (source[i] == '\n') {
    //         line_size = i;
    //         return &(source[i+1]);
    //     }
    // }
    line_size = source_size;
    return nullptr;
}

void PreProcessor::error(const char* message) {
    fprintf(stderr, "PreProcessor error: %s\n", message);
    exit(1);
}
