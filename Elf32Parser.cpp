//
// Created by lovekoala on 2023/7/21.
//

#include "Elf32Parser.h"
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
using namespace std;
inline void Elf32Parser::patchAddress(uint32_t orig_target, uint32_t orig_value,uint32_t load_offset) {
    uint32_t target = orig_target, value = orig_value;
    for(auto segment: segments) {
        if(segment.start < target && segment.end > target) {
            target = orig_target - segment.ftov_offset;
        }
        if(segment.start < value && segment.end > value) {
            value  = orig_value - segment.ftov_offset;
        }
    }
    value += load_offset;
    cout << hex << "in " << target << "(" << orig_target << ")"
    << " with value " << value << "(" << orig_value << ")" << endl;
    uint32_t *patching_addr = (uint32_t *)&file_mmap[target];
    *patching_addr = value;
}

bool Elf32Parser::parse(const char *filename) {
    int fd = open(filename,O_RDWR);
    struct stat st;
    if(fd < 0) {
        cout << "open " << filename << " failed." << endl;
        return false;
    }
    if(fstat(fd,&st) < 0) {
        cout << "get file status failed, file name:" << filename << endl;
        return false;
    }
    file_size = st.st_size;
    file_mmap = (uint8_t *)mmap(NULL,file_size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(MAP_FAILED == file_mmap) {
        cout << "mmap file failed,file name:" << filename << endl;
        return false;
    }
    if(file_mmap[0] != 0x7f && strcmp((const char *)&file_mmap[1],"ELF")) {
        cout << filename << " not an elf file" << endl;
        return false;
    }
    ehdr = (Elf32_Ehdr *)file_mmap;
    if(ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
        cout << filename << " is not a valid executable file " << endl;
        return false;
    }
    phdr = (Elf32_Phdr *)&file_mmap[this->ehdr->e_phoff];
    shdr = (Elf32_Shdr *)&file_mmap[this->ehdr->e_shoff];
    sh_string_table = &file_mmap[shdr[ehdr->e_shstrndx].sh_offset];
    string_table = &file_mmap[searchSection(".dynstr")->sh_offset];
    uint32_t segment_count = ehdr->e_phnum;
    for(int i = 0;i < segment_count; i++) {
        SegmentRange segment_range;
        Elf32_Phdr *segment = &phdr[i];
        segment_range.ftov_offset = segment->p_vaddr - segment->p_offset;
        segment_range.start = segment->p_vaddr;
        segment_range.end = segment->p_vaddr + segment->p_memsz;
        segments.push_back(segment_range);
    }
    return true;
}

Elf32_Shdr *Elf32Parser::searchSection(const char *section_name) {
    int section_count = ehdr->e_shentsize;
    for( int i = 0; i < section_count; i++) {
        if(!strcmp(section_name,(const char *)&sh_string_table[shdr[i].sh_name])) {
            return &shdr[i];
        }
    }
    return nullptr;
}

void Elf32Parser::patchGot(const char *rel_name,const char *symbol_name,int load_offset) {
    cout << "patching " << rel_name << endl;
    Elf32_Shdr *reldyn_header = searchSection(rel_name);
    int count = reldyn_header->sh_size / reldyn_header->sh_entsize;
    Elf32_Rel *reldyns = (Elf32_Rel *)&file_mmap[reldyn_header->sh_offset];
    Elf32_Shdr *symbols_header = searchSection(symbol_name);
    Elf32_Sym *symbols = (Elf32_Sym *)&file_mmap[symbols_header->sh_offset];
    for(int i = 0; i < count; i++) {
        Elf32_Sym *sym = (Elf32_Sym *)&symbols[ELF32_R_SYM(reldyns[i].r_info)];
        if(sym->st_value == 0) {
            continue;
        }
        cout << "patching "  << &string_table[sym->st_name] << ":";
        patchAddress(reldyns[i].r_offset,sym->st_value,load_offset);
    }
}

void Elf32Parser::flush() {
    msync(file_mmap,file_size, MS_SYNC);
}