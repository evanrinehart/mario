struct Instruction {
    int opcode;
    char mnemonic[4];
    int size;
    int mode;
    int cycles;
    int unofficial;
    int color;
};

// addressing modes
// 0 - implicit
// 1 - accumulator
// 2 - immediate
// 3 - zero page
// 4 - absolute
// 5 - relative
// 6 - indirect

// 7 - d,x
// 8 - d,y
// 9 - a,x
// 9 - a,y
// 9 - (d,x)
// 9 - (d),y

// table of official opcodes
struct Instruction instructions[256] = {
    {0x69, "adc", 2, 0, 2, 0, 0},
    {0x65, "adc", 2, 0, 3, 0, 0},
    {0x75, "adc", 2, 0, 4, 0, 0},
    {0x6d, "adc", 3, 0, 4, 0, 0},
    {0x7d, "adc", 3, 0, 4, 0, 0},
    {0x79, "adc", 3, 0, 4, 0, 0},
    {0x61, "adc", 2, 0, 6, 0, 0},
    {0x71, "adc", 2, 0, 5, 0, 0},

    {0xe9, "sbc", 2, 0, 2, 0, 0},
    {0xe5, "sbc", 2, 0, 3, 0, 0},
    {0xf5, "sbc", 2, 0, 4, 0, 0},
    {0xed, "sbc", 3, 0, 4, 0, 0},
    {0xfd, "sbc", 3, 0, 4, 0, 0},
    {0xf9, "sbc", 3, 0, 4, 0, 0},
    {0xe1, "sbc", 2, 0, 6, 0, 0},
    {0xf1, "sbc", 2, 0, 5, 0, 0},

    {0x29, "and", 2, 0, 2, 0, 0},
    {0x25, "and", 2, 0, 3, 0, 0},
    {0x35, "and", 2, 0, 4, 0, 0},
    {0x2d, "and", 3, 0, 4, 0, 0},
    {0x3d, "and", 3, 0, 4, 0, 0},
    {0x39, "and", 3, 0, 4, 0, 0},
    {0x21, "and", 2, 0, 6, 0, 0},
    {0x31, "and", 2, 0, 5, 0, 0},

    {0x0a, "asl", 1, 0, 2, 0, 0},
    {0x06, "asl", 2, 0, 5, 0, 0},
    {0x16, "asl", 2, 0, 6, 0, 0},
    {0x0e, "asl", 3, 0, 6, 0, 0},
    {0x1e, "asl", 3, 0, 7, 0, 0},
    
    {0x90, "bcc", 2, 0, 2, 0, 0},
    {0xb0, "bcs", 2, 0, 2, 0, 0},
    {0xf0, "beq", 2, 0, 2, 0, 0},
    {0x30, "bmi", 2, 0, 2, 0, 0},
    {0xd0, "bne", 2, 0, 2, 0, 0},
    {0x10, "bpl", 2, 0, 2, 0, 0},
    {0x50, "bvc", 2, 0, 2, 0, 0},
    {0x70, "bvs", 2, 0, 2, 0, 0},

    {0x24, "bit", 2, 0, 3, 0, 0},
    {0x2c, "bit", 3, 0, 4, 0, 0},
    
    {0x18, "clc", 1, 0, 2, 0, 0},
    {0xd8, "cld", 1, 0, 2, 0, 0},
    {0x58, "cli", 1, 0, 2, 0, 0},
    {0xb8, "clv", 1, 0, 2, 0, 0},

    {0x38, "sec", 1, 0, 2, 0, 0},
    {0xf8, "sed", 1, 0, 2, 0, 0},
    {0x78, "sei", 1, 0, 2, 0, 0},

    {0xc9, "cmp", 2, 0, 2, 0, 0},
    {0xc5, "cmp", 2, 0, 3, 0, 0},
    {0xd5, "cmp", 2, 0, 4, 0, 0},
    {0xcd, "cmp", 3, 0, 4, 0, 0},
    {0xdd, "cmp", 3, 0, 4, 0, 0},
    {0xd9, "cmp", 3, 0, 4, 0, 0},
    {0xc1, "cmp", 2, 0, 6, 0, 0},
    {0xd1, "cmp", 2, 0, 5, 0, 0},

    {0xe0, "cpx", 2, 0, 2, 0, 0},
    {0xe4, "cpx", 2, 0, 3, 0, 0},
    {0xec, "cpx", 3, 0, 4, 0, 0},
    {0xc0, "cpy", 2, 0, 2, 0, 0},
    {0xc4, "cpy", 2, 0, 3, 0, 0},
    {0xcc, "cpy", 3, 0, 4, 0, 0},
    
    {0xc6, "dec", 2, 0, 5, 0, 0},
    {0xd6, "dec", 2, 0, 6, 0, 0},
    {0xce, "dec", 3, 0, 6, 0, 0},
    {0xde, "dec", 3, 0, 7, 0, 0},

    {0xe6, "inc", 2, 0, 5, 0, 0},
    {0xf6, "inc", 2, 0, 6, 0, 0},
    {0xee, "inc", 3, 0, 6, 0, 0},
    {0xfe, "inc", 3, 0, 7, 0, 0},

    {0xca, "dex", 1, 0, 2, 0, 0},
    {0x88, "dey", 1, 0, 2, 0, 0},

    {0xe8, "inx", 1, 0, 2, 0, 0},
    {0xc8, "iny", 1, 0, 2, 0, 0},

    {0x49, "eor", 2, 0, 2, 0, 0},
    {0x45, "eor", 2, 0, 3, 0, 0},
    {0x55, "eor", 2, 0, 4, 0, 0},
    {0x4d, "eor", 3, 0, 4, 0, 0},
    {0x5d, "eor", 3, 0, 4, 0, 0},
    {0x59, "eor", 3, 0, 4, 0, 0},
    {0x41, "eor", 2, 0, 6, 0, 0},
    {0x51, "eor", 2, 0, 5, 0, 0},

    {0x4c, "jmp", 3, 0, 3, 0, 0},    
    {0x6c, "jmp", 3, 0, 5, 0, 0},
    {0x20, "jsr", 3, 0, 6, 0, 0},
    {0x40, "rti", 1, 0, 6, 0, 0},
    {0x60, "rts", 1, 0, 6, 0, 0},

    {0xa9, "lda", 2, 0, 2, 0, 0},
    {0xa5, "lda", 2, 0, 3, 0, 0},
    {0xb5, "lda", 2, 0, 4, 0, 0},
    {0xad, "lda", 3, 0, 4, 0, 0},
    {0xbd, "lda", 3, 0, 4, 0, 0},
    {0xb9, "lda", 3, 0, 4, 0, 0},
    {0xa1, "lda", 2, 0, 6, 0, 0},
    {0xb1, "lda", 2, 0, 5, 0, 0},

    {0xa2, "ldx", 2, 0, 2, 0, 0},
    {0xa6, "ldx", 2, 0, 2, 0, 0},
    {0xb6, "ldx", 2, 0, 4, 0, 0},
    {0xae, "ldx", 3, 0, 4, 0, 0},
    {0xbe, "ldx", 3, 0, 4, 0, 0},
    
    {0xa0, "ldy", 2, 0, 2, 0, 0},
    {0xa4, "ldy", 2, 0, 2, 0, 0},
    {0xb4, "ldy", 2, 0, 4, 0, 0},
    {0xac, "ldy", 3, 0, 4, 0, 0},
    {0xbc, "ldy", 3, 0, 4, 0, 0},

    {0x85, "sta", 2, 0, 3, 0, 0},
    {0x95, "sta", 2, 0, 4, 0, 0},
    {0x8d, "sta", 3, 0, 4, 0, 0},
    {0x9d, "sta", 3, 0, 5, 0, 0},
    {0x99, "sta", 3, 0, 5, 0, 0},
    {0x81, "sta", 2, 0, 6, 0, 0},
    {0x91, "sta", 2, 0, 6, 0, 0},

    {0x86, "stx", 2, 0, 3, 0, 0},
    {0x96, "stx", 2, 0, 4, 0, 0},
    {0x8e, "stx", 3, 0, 4, 0, 0},

    {0x84, "sty", 2, 0, 3, 0, 0},
    {0x94, "sty", 2, 0, 4, 0, 0},
    {0x8c, "sty", 3, 0, 4, 0, 0},

    {0xaa, "tax", 1, 0, 2, 0, 0},
    {0xa8, "tay", 1, 0, 2, 0, 0},
    {0xba, "tsx", 1, 0, 2, 0, 0},
    {0x8a, "txa", 1, 0, 2, 0, 0},
    {0x9a, "txs", 1, 0, 2, 0, 0},
    {0x98, "tya", 1, 0, 2, 0, 0},
    
    {0x4a, "lsr", 1, 0, 2, 0, 0},
    {0x46, "lsr", 2, 0, 5, 0, 0},
    {0x56, "lsr", 2, 0, 6, 0, 0},
    {0x4e, "lsr", 3, 0, 6, 0, 0},
    {0x5e, "lsr", 3, 0, 7, 0, 0},

    {0xea, "nop", 1, 0, 2, 0, 0},

    {0x09, "ora", 2, 0, 2, 0, 0},
    {0x05, "ora", 2, 0, 3, 0, 0},
    {0x15, "ora", 2, 0, 4, 0, 0},
    {0x0d, "ora", 3, 0, 4, 0, 0},
    {0x1d, "ora", 3, 0, 4, 0, 0},
    {0x19, "ora", 3, 0, 4, 0, 0},
    {0x01, "ora", 2, 0, 6, 0, 0},
    {0x11, "ora", 2, 0, 5, 0, 0},

    {0x48, "pha", 1, 0, 3, 0, 0},
    {0x08, "php", 1, 0, 3, 0, 0},

    {0x68, "pla", 1, 0, 4, 0, 0},
    {0x28, "plp", 1, 0, 4, 0, 0},

    {0x2a, "rol", 1, 0, 2, 0, 0},
    {0x26, "rol", 2, 0, 5, 0, 0},
    {0x36, "rol", 2, 0, 6, 0, 0},
    {0x2e, "rol", 3, 0, 6, 0, 0},
    {0x3e, "rol", 3, 0, 7, 0, 0},

    {0x6a, "ror", 1, 0, 2, 0, 0},
    {0x66, "ror", 2, 0, 5, 0, 0},
    {0x76, "ror", 2, 0, 6, 0, 0},
    {0x6e, "ror", 3, 0, 6, 0, 0},
    {0x7e, "ror", 3, 0, 7, 0, 0}
};
    
    
