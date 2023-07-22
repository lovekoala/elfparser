//
// Created by lovekoala on 2023/7/21.
//

#ifndef ELFPARSER_ELF32PARSER_H
#define ELFPARSER_ELF32PARSER_H

#include "elf.h"
#include <vector>
struct SegmentRange {
    uint32_t start;
    uint32_t end;
    uint32_t ftov_offset;
};
class Elf32Parser {
public:
    bool parse(const char *filename);
    Elf32_Shdr *searchSection(const char *section_name);
    void patchGot(const char *rel_name,const char *symbol_name,int load_offset);
    void patchAddress(uint32_t target, uint32_t value, uint32_t load_offset);
    void flush();
private:
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdr;
    Elf32_Shdr *shdr;
    uint8_t *sh_string_table;
    uint8_t *file_mmap;
    uint8_t *string_table;
    uint32_t file_size;
    std::vector<SegmentRange> segments;
};


#endif //ELFPARSER_ELF32PARSER_H
