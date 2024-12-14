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

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>

PreProcessor::PreProcessor()
    : m_current_offset(0) {
}

PreProcessor::~PreProcessor() {
}

void PreProcessor::process(const char* source, size_t source_size, const std::string_view& file_name) {
    // 1. resolve includes in %include "file" format
    {
        char* source_i = new char[source_size + 1];
        strncpy(source_i, source, source_size);
        source_i[source_size] = '\0';
        source = source_i;
    }
    const char* original_source = source;

    HandleIncludes(source, source_size, file_name);

    delete[] original_source;

    // export the buffer to a char const* and size
    size_t source2_size = m_current_offset;
    char const* source2;
    {
        char* source2_i = new char[source2_size + 1];
        m_buffer.Read(0, reinterpret_cast<uint8_t*>(source2_i), source2_size);
        source2_i[source2_size] = '\0';
        source2 = source2_i;
    }

    // clear the buffer
    m_buffer.Clear();
    m_current_offset = 0;

    const char* original_source2 = source2;

    // 2. remove single line comments starting with ;
    while (true) {
        if (char const* comment_start = strchr(source2, ';'); comment_start == nullptr) {
            m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source2), source2_size - (source2 - original_source2));
            m_current_offset += source2_size - (source2 - original_source2);
            break;
        } else {
            m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source2), comment_start - source2);
            m_current_offset += comment_start - source2;
            source2 = comment_start;
            if (char const* comment_end = strchr(source2, '\n'); comment_end == nullptr) {
                comment_end = original_source2 + source2_size;
                break;
            } else
                source2 = comment_end;
        }
    }

    delete[] original_source2;

    // export the buffer to a char const* and size
    size_t source3_size = m_current_offset;
    char const* source3;
    {
        char* source3_i = new char[source3_size + 1];
        m_buffer.Read(0, reinterpret_cast<uint8_t*>(source3_i), source3_size);
        source3_i[source3_size] = '\0';
        source3 = source3_i;
    }

    // clear the buffer
    m_buffer.Clear();
    m_current_offset = 0;

    const char* original_source3 = source3;

    // 3. remove multi line comments starting with /* and ending with */
    while (true) {
        if (char const* comment_start = strstr(source3, "/*"); comment_start == nullptr) {
            m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source3), source3_size - (source3 - original_source3));
            m_current_offset += source3_size - (source3 - original_source3);
            break;
        } else {
            m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source3), comment_start - source3);
            m_current_offset += comment_start - source3;
            source3 = comment_start;
            if (char const* comment_end = strstr(source3, "*/"); comment_end == nullptr)
                error("Unterminated comment2");
            else
                source3 = comment_end + 2;
        }
    }

    delete[] original_source3;

    // export the buffer to a char const* and size
    size_t source4_size = m_current_offset;
    char const* source4;
    {
        char* source4_i = new char[source4_size + 1];
        m_buffer.Read(0, reinterpret_cast<uint8_t*>(source4_i), source4_size);
        source4_i[source4_size] = '\0';
        source4 = source4_i;
    }

    // clear the buffer
    m_buffer.Clear();
    m_current_offset = 0;

    const char* original_source4 = source4;

    // 4. resolve macros in %define name value format

    struct Define {
        std::string name;
        std::string value;
        char const* start;
    };

    std::vector<Define> defines;

    // start by finding all the macros and making a list of them
    while (true) {
        char* define_start = strstr(const_cast<char*>(source4), "%define ");
        if (define_start == nullptr)
            break;
        define_start += 8;
        char* define_end = strchr(define_start, ' ');
        if (define_end == nullptr)
            error("Invalid define directive");
        char* value_end = strchr(define_end + 1, '\n');
        if (value_end == nullptr)
            error("Unterminated define directive");
        defines.push_back({std::string(define_start, define_end), std::string(define_end + 1, value_end), define_start - 8});
        source4 = value_end + 1;
    }

    source4 = original_source4;

    // then remove the macros from the source
    for (Define& define : defines) {
        m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source4), define.start - source4);
        m_current_offset += define.start - source4;
        source4 = define.start + define.name.size() + define.value.size() + 9;
    }
    // write the rest of the source
    m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source4), source4_size - (source4 - original_source4));
    m_current_offset += source4_size - (source4 - original_source4);

    // export into source 5
    delete[] original_source4;

    size_t source5_size = m_current_offset;
    char const* source5;
    {
        char* source5_i = new char[source5_size + 1];
        m_buffer.Read(0, reinterpret_cast<uint8_t*>(source5_i), source5_size);
        source5_i[source5_size] = '\0';
        source5 = source5_i;
    }

    // clear the buffer
    m_buffer.Clear();
    m_current_offset = 0;

    char const* original_source5 = source5;

    // make a list of all references to the macros
    struct DefineReference {
        std::string name;
        char const* start;
    };

    std::map<size_t /* offset */, Define&> define_references;

    for (Define& define : defines) {
        char* define_start = const_cast<char*>(source5);
        while (define_start != nullptr) {
            define_start = strstr(define_start, define.name.c_str());
            if (define_start != nullptr) {
                define_references.insert({define_start - source5, define});
                define_start += define.name.size();
            }
        }
    }

    for (auto& [offset, define] : define_references) {
        if (offset < m_current_offset) { // possible overlap
            // need to ensure it does overlap and isn't before. if it is before, then an error has occurred
            if (offset + define.name.size() <= m_current_offset)
                error("Invalid define reference");

            // no need to copy before the offset
            m_buffer.Write(m_current_offset - (m_current_offset - offset), reinterpret_cast<const uint8_t*>(define.value.c_str()), define.value.size());
            m_current_offset += define.value.size() - (m_current_offset - offset);
            source5 += define.name.size() - (m_current_offset - offset);

        } else {
            m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source5), offset - (source5 - original_source5));
            m_current_offset += offset - (source5 - original_source5);
            m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(define.value.c_str()), define.value.size());
            m_current_offset += define.value.size();
            source5 += offset - (source5 - original_source5) + define.name.size();
        }
    }

    // copy the rest of the source
    m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(source5), source5_size - (source5 - original_source5));
    m_current_offset += source5_size - (source5 - original_source5);

    delete[] original_source5;
}

size_t PreProcessor::GetProcessedBufferSize() const {
    return m_current_offset;
}

void PreProcessor::ExportProcessedBuffer(uint8_t* data) const {
    m_buffer.Read(0, data, m_current_offset);
}

char* PreProcessor::GetLine(char* source, size_t source_size, size_t& line_size) {
    if (void* line = memchr(source, '\n', source_size); line != nullptr) {
        line_size = static_cast<char*>(line) - source;
        return static_cast<char*>(line) + 1;
    }
    line_size = source_size;
    return nullptr;
}

void PreProcessor::HandleIncludes(const char* source, size_t source_size, const std::string_view& file_name) {
    char* include_start = const_cast<char*>(source);
    char* include_end = nullptr;
    char const* i_source = source;
    while (include_start != nullptr) {
        include_start = strstr(include_start, "%include \"");
        if (include_start != nullptr) {
            m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(i_source), include_start - i_source);
            m_current_offset += include_start - i_source;
            include_start += 10;
            include_end = strchr(include_start, '"');
            if (include_end != nullptr) {
                std::string include_string = std::filesystem::path(file_name).replace_filename(std::string(include_start, include_end));
                if (FILE* file = fopen(include_string.c_str(), "r"); file != nullptr) {
                    fseek(file, 0, SEEK_END);
                    size_t file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    char* file_data = new char[file_size];
                    fread(file_data, 1, file_size, file);
                    fclose(file);
                    PreProcessor preprocessor;
                    preprocessor.HandleIncludes(file_data, file_size, include_string);
                    size_t processed_size = preprocessor.GetProcessedBufferSize();
                    uint8_t* processed_data = new uint8_t[processed_size];
                    preprocessor.ExportProcessedBuffer(processed_data);
                    m_buffer.Write(m_current_offset, processed_data, processed_size);
                    m_current_offset += processed_size;
                    delete[] processed_data;
                    delete[] file_data;
                } else
                    error("Could not open include file");
                i_source = include_end + 1;
            } else
                error("Unterminated include directive");
        }
    }
    // read the rest of the source
    if (include_end == nullptr)
        include_end = const_cast<char*>(source);
    else
        include_end += 1;
    m_buffer.Write(m_current_offset, reinterpret_cast<const uint8_t*>(include_end), source_size - (include_end - source));
    m_current_offset += source_size - (include_end - source);
}


[[noreturn]] void PreProcessor::error(const char* message) {
    fprintf(stderr, "PreProcessor error: %s\n", message);
    exit(1);
}
