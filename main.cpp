//
// Created by lovekoala on 2023/7/21.
//

#include <iostream>
#include "Elf32Parser.h"
using namespace std;
int main(int argc,char **argv) {
    Elf32Parser parser;
    parser.parse(argv[1]);
    parser.patchGot(".rel.dyn", ".dynsym",0x10000);
    parser.patchGot(".rel.plt", ".dynsym",0x10000);
    parser.flush();
    return 0;
}
