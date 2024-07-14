#include <stdio.h>
#include <stdint.h>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Assembler.hpp"

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

    Lexer* lexer = new Lexer();
    lexer->tokenize((const char*)file_contents, file_size);

    Parser parser;
    parser.parse(lexer->GetTokens());
#ifdef ASSEMBLER_DEBUG
    parser.PrintSections(stdout);
#endif
    fflush(stdout);

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

    return 0;
}