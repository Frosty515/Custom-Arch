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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Buffer.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Assembler.hpp"
#include "PreProcessor.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <file> <out-file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == nullptr) {
        printf("Error: could not open file %s\n", argv[1]);
        return 1;
    }

    FILE* output_file = fopen(argv[2], "w");
    if (output_file == nullptr) {
        printf("Error: could not open output file %s\n", argv[2]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* file_contents = new uint8_t[file_size];
    fread(file_contents, 1, file_size, file);
    fclose(file);

    PreProcessor pre_processor;
    pre_processor.process((const char*)file_contents, file_size);
    size_t processed_buffer_size = pre_processor.GetProcessedBufferSize();
    uint8_t* processed_buffer_data = new uint8_t[processed_buffer_size];
    pre_processor.ExportProcessedBuffer(processed_buffer_data);

    Lexer* lexer = new Lexer();
    lexer->tokenize((const char*)processed_buffer_data, processed_buffer_size);
    // lexer->tokenize((const char*)file_contents, file_size);

    Parser parser;
    parser.parse(lexer->GetTokens());
#ifdef ASSEMBLER_DEBUG
    parser.PrintSections(stdout);
    fflush(stdout);
#endif

    Assembler assembler;
    assembler.assemble(parser.GetLabels());

    const Buffer& buffer = assembler.GetBuffer();
    size_t buffer_size = buffer.GetSize();
    uint8_t* buffer_data = new uint8_t[buffer_size];
    buffer.Read(0, buffer_data, buffer_size);

    if (buffer_size != fwrite(buffer_data, 1, buffer_size, output_file)) {
        perror("Error: could not write to output file");
        return 1;
    }
    fclose(output_file);

    delete[] file_contents;
    delete[] processed_buffer_data;
    delete[] buffer_data;

    assembler.Clear();

    parser.Clear();

    lexer->Clear();

    delete lexer;

    return 0;
}