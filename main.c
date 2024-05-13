#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <raylib.h>

#include <rom.h>
#include <instructions.h>

unsigned char memory[65536];

struct InterruptVectors {
    int nmi;
    int reset;
    int irq;
};

struct InterruptVectors vectors;

struct ProcessorStatus {
    int carry;
    int zero;
    int interruptDisable;
    int decimal;
    int overflow;
    int negative;
};

struct Registers {
    unsigned char A;
    unsigned char X;
    unsigned char Y;
    unsigned char S;
    int PC;
    struct ProcessorStatus P;
};

struct Registers regs = {0,0,0,0xfd,0,{0,0,1,0,0,0}};

struct NESHeader {
    unsigned char start[4];
    unsigned char prg_rom_size;
    unsigned char chr_rom_size;
    unsigned char flags6;
    unsigned char flags7;
    unsigned char flags8;
    unsigned char flags9;
    unsigned char flags10;
    unsigned char unused[5];
};

int prgbase = 0x8000;
unsigned char *prg;
unsigned char *chr;

int screenW = 320;
int screenH = 240;
int screenScale = 3;
Image screenImg;
Texture2D screenTex;

unsigned char ppuMemory[16384];
// $0000 - $1fff the CHR ROM
// $2000 - $2fff vram nametables (subject to mirroring)
// $3000 - $3eff unused
// $3f00 - $3f1f palette RAM indexes
// $3f20 - $3fff mirrors
unsigned char oam[256]; // 64 x 4 bytes

struct PPUCtrl {
    int nametableBase;
    int vramAddressIncrement;
    int spritePatternAddress;
    int bgPatternAddress;
    int spriteSize;
    int extMaster;
    int nmiOutput;
};

struct PPUStatus {
    int spriteOverflow;
    int spriteZeroHit;
    int inVblank;
};

struct PPUCtrl ppuCtrl = {0,0,0,0,0,0,0};
struct PPUStatus ppuStatus = {0,0,0};



unsigned char read2002() {
    // reading PPU status register
    // has the effect of clearing bit 7

    unsigned char out = 0;

    out |= (ppuStatus.inVblank << 7);
    out |= (ppuStatus.spriteZeroHit << 6);
    out |= (ppuStatus.spriteOverflow << 5);
    ppuStatus.inVblank = 0;

    return out;
}

void write2000(unsigned char byte) {
    ppuCtrl.nmiOutput = (byte >> 7) & 1; // might have immediate effect
    if(ppuCtrl.nmiOutput) printf("write to PPU ctrl enabled NMI on start of vblank\n");
    ppuCtrl.extMaster = (byte >> 6) & 1;
    ppuCtrl.spriteSize = (byte >> 5) & 1; // 8x8 or 8x16
    ppuCtrl.bgPatternAddress = (byte >> 4) & 1; // 0000 or 1000
    ppuCtrl.spritePatternAddress = (byte >> 3) & 1; // 0000 or 1000
    ppuCtrl.vramAddressIncrement = (byte >> 2) & 1; // add 1 or add 32
    ppuCtrl.nametableBase = byte & 0x03; // 2000, 2400, 2800, or 2c00
}

void write2001(unsigned char byte) {
    printf("write2001(%02x)\n", byte);
}



void resetCPU(){
    regs.P.interruptDisable = 1;
    regs.PC = vectors.reset;
}

void printBits(int byte){
    printf("%c", (byte >> 0) & 1 ? '1' : '0');
    printf("%c", (byte >> 1) & 1 ? '1' : '0');
    printf("%c", (byte >> 2) & 1 ? '1' : '0');
    printf("%c", (byte >> 3) & 1 ? '1' : '0');
    printf("%c", (byte >> 4) & 1 ? '1' : '0');
    printf("%c", (byte >> 5) & 1 ? '1' : '0');
    printf("%c", (byte >> 6) & 1 ? '1' : '0');
    printf("%c", (byte >> 7) & 1 ? '1' : '0');
}

struct Instruction * instructionFromOpcode(int opcode){
    struct Instruction *ptr = &instructions[0];
    for(int i = 0; i < 256; i++){
        if(ptr->opcode == opcode) return ptr;
        ptr++;
    }
    printf("unknown opcode (%02x)\n", opcode);
    exit(1);
}

void printInstruction(int addr){
    int opcode = memory[addr];
    struct Instruction * ins = instructionFromOpcode(opcode);
    printf("%s", ins->mnemonic);
    if(ins->size == 3) printf(" %02x %02x", memory[addr+1], memory[addr+2]);
    if(ins->size == 2) printf(" %02x", memory[addr+1]);
    printf("\n");
}

void showCPU(){
    printf("A = $%02x\n", regs.A);
    printf("X = $%02x\n", regs.X);
    printf("Y = $%02x\n", regs.Y);
    printf("S = $%02x\n", regs.S);
    printf("PC = $%04x\n", regs.PC);
    printf("carry = %d\n", regs.P.carry);
    printf("zero = %d\n", regs.P.zero);
    printf("interruptDisable = %d\n", regs.P.interruptDisable);
    printf("decimal = %d\n", regs.P.decimal);
    printf("overflow = %d\n", regs.P.overflow);
    printf("negative = %d\n", regs.P.negative);
    printf("instruction @ PC:\n");
    printInstruction(regs.PC);
    printf("\n");
}

#define UNCOMPLEMENT(X) ((X) < 128 ? (X) : (X - 256))


struct Instruction * fetchInstruction(int addr, int * arg1, int * arg2){
    int opcode = memory[addr];
    struct Instruction * ins = instructionFromOpcode(opcode);
    if(ins->size > 1) *arg1 = memory[addr+1];
    if(ins->size > 2) *arg2 = memory[addr+2];
    return ins;
}

unsigned char readMemory(int addr){
    if(addr == 0x2000 || addr == 0x2001 || addr == 0x2003 || addr == 0x2005 || addr == 0x2006){
        printf("read from $%04x (normally write only)\n", addr);
        return 0;
    }
    else if(addr == 0x2002){
        printf("read from $%04x (PPU status)\n", addr);
        return read2002();
    }
    else if(addr == 0x2004){
        printf("read from $2004 (OAM data)\n");
        return 0;
    }
    else if(addr == 0x2007){
        printf("read from $2007 (PPU data)\n");
        return 0;
    }
    else if(addr >= 0x4000 && addr <= 0x4014){
        printf("read from $%04x\n", addr);
        exit(1);
    }
    else if(addr == 0x4015){
        printf("read from $%04x (sound channels and IRQ status)\n", addr);
        return 0;
    }
    else if(addr == 0x4016){
        printf("read from $%04x (joystick 1 data)\n", addr);
        return 0;
    }
    else if(addr == 0x4017){
        printf("read from $%04x (joystick 2 data)\n", addr);
        return 0;
    }
    else if(addr >= 0x4018 && addr <= 0x401f){
        printf("read from $%04x (disabled functionality)\n", addr);
        return 0;
    }
    else return memory[addr];
}

void writeMemory(int addr, unsigned char byte){
    if(addr == 0x2000 /*|| addr == 0x0778 */) {
        printf("write %02x to %04x (PPU ctrl)\n", byte, addr);
        write2000(byte);
    }
    else if(addr == 0x2001 /* || addr == 0x0779 */){
        printf("write %02x to %04x (PPU mask)\n", byte, addr);
        write2001(byte);
    }
    else if(addr == 0x2003){
        printf("write %02x to $2003 (OAM addr)\n", byte);
    }
    else if(addr == 0x2004){
        printf("write %02x to $2004 (OAM data)\n", byte);
    }
    else if(addr == 0x2005){
        printf("write %02x to $2005 (PPU scroll)\n", byte);
    }
    else if(addr == 0x2006){
        printf("write %02x to $2006 (PPU addr)\n", byte);
    }
    else if(addr == 0x2007){
        printf("write %02x to $2007 (PPU data)\n", byte);
    }
    else if(addr == 0x4014){
        printf("write %02x to $4014 (OAM DMA)\n", byte);
    }
    else if(addr >= 0x4000 && addr <= 0x4015){
        // TODO
        printf("write %02x to $%04x, some sound chip control\n", byte, addr);
    }
    else if(addr == 0x4016){
        printf("write %02x to $4016 (joystick stobe)\n", byte);
    }
    else if(addr == 0x4017){
        printf("write %02x to $4016 (apu frame counter)\n", byte);
    }
    else if(addr >= 0x4018 && addr <= 0x401f){
        printf("write %02x to $%04x, i/o functionality that is disabled\n", byte, addr);
    }
    else if(addr >= 0x4020){
        printf("attempting to write to unmapped memory ($%04x <= $%02x)\n", addr, byte);
        exit(1);
    }
    else if(addr >= 0x6000){
        printf("attempting to write to cart RAM (not there) ($%04x <= $%02x)\n", addr, byte);
        exit(1);
    }
    else if(addr >= 0x8000){
        printf("attempting to write to PRG ROM ($%04x <= $%02x)\n", addr, byte);
        exit(1);
    }
    else if(addr < 0 || addr > 0xffff){
        printf("attempting to write out of bounds ($%04x <= $%02x)\n", addr, byte);
        exit(1);
    }
    else memory[addr] = byte;
}

int nextCPUDelay(){
    // this could easily be a table, opcode -> cycles
    int arg1, arg2;
    struct Instruction * ins = fetchInstruction(regs.PC, &arg1, &arg2);
    return ins->cycles;
}

// 7 cycle interrupt sequence, transfer control to NMI vector
void nmiCPU(){
    // ignore the incoming instruction (2 cycles)
    // save 2 byte PC to stack (2 cycles)
    // push status register on stack (1 cycle)
    // set I flag and fetch FFFE (1 cycle)
    // fetch FFFF and update PC (1 cycle)
}

// fetch next instruction and execute effects
// leaves the CPU in some state after N cycles (see nextCPUDelay)
void stepCPU(){
    int arg1, arg2;
    struct Instruction * ins = fetchInstruction(regs.PC, &arg1, &arg2);
    int size = ins->size;
    int arg21;
    int addr;
    unsigned char m;

    //printf(" pc=%04x %s ", regs.PC, ins->mnemonic);

    regs.PC += size;

    switch(ins->opcode){
        case 0x78: // SEI
            regs.P.interruptDisable = 1;
            break;
        case 0xd8: // CLD
            regs.P.decimal = 0;
            break;

        case 0xc9: // CMP
            regs.P.carry = regs.A >= arg1;
            regs.P.zero  = regs.A == arg1;
            regs.P.negative  = regs.A < arg1;
            break;

        case 0xe0: // CPX
            regs.P.carry = regs.X >= arg1;
            regs.P.zero  = regs.X == arg1;
            regs.P.negative  = regs.X < arg1;
            break;

        case 0xc0: // CPY
            regs.P.carry = regs.Y >= arg1;
            regs.P.zero  = regs.Y == arg1;
            regs.P.negative  = regs.Y < arg1;
            break;
            

        case 0xa9: // LDA #$7f
            regs.A = arg1;
            regs.P.zero     = regs.A == 0;
            regs.P.negative = regs.A > 0x7f;
            break;

        case 0xad: // LDA $0205
            // load from arbitrary address
            // this potentially has side effects e.g. if reading from $2002
            arg21 = (arg2 << 8) | arg1;
            regs.A = readMemory(arg21);
            regs.P.zero = regs.A == 0;
            regs.P.negative = regs.A > 0x7f;
            break;

        case 0xbd: // LDA $0205, X
            arg21 = (((arg2 << 8) | arg1) + regs.X) & 0xffff;
            regs.A = readMemory(arg21);
            regs.P.zero     = regs.A == 0;
            regs.P.negative = regs.A > 0x7f;
            break;

        case 0x85: // STA $06
            printf("sta %02x\n", arg1);
            memory[arg1] = regs.A;
            break;

        case 0x8d: // STA $0205
            printf("sta %02x %02x\n", arg1, arg2);
            arg21 = (arg2 << 8) | arg1;
            writeMemory(arg21, regs.A);
            break;

        case 0x91: // STA ($06), Y
            addr = memory[arg1];
            addr |= memory[(arg1 + 1) & 0xff] << 8;
            addr += regs.Y;
            writeMemory(addr, regs.A);
            memory[addr] = regs.A;
            break;

        case 0x99: // STA $0205, Y
            printf("sta %02x %02x, Y where Y=%02x\n", arg1, arg2, regs.Y);
            arg21 = (arg2 << 8) | arg1;
            printf("writing A=%02x to %04x\n", regs.A, arg21 + regs.Y);
            writeMemory(arg21 + regs.Y, regs.A);
            break;

        case 0xa2: // LDX #$7f
            regs.X = arg1;
            regs.P.zero     = regs.X == 0;
            regs.P.negative = regs.X > 0x7f;
            break;

        case 0xa0: // LDY #$7f
            regs.Y = arg1;
            regs.P.zero     = regs.Y == 0;
            regs.P.negative = regs.Y > 0x7f;
            break;

        case 0x86: // STX $07
            memory[arg1] = regs.X;
            break;

        case 0x9a: // TXS
            regs.S = regs.X;
            break;

        case 0x8a: // TXA
            regs.A = regs.X;
            break;

        case 0x10: // BPL #7 branch if positive
            if(regs.P.negative == 0) regs.PC += UNCOMPLEMENT(arg1);
            break;

        case 0xb0: // BCS #3 branch if carry
            if(regs.P.carry) regs.PC += UNCOMPLEMENT(arg1);
            break;

        case 0xd0: // BNE #4 branch if not equal
            if(regs.P.zero == 0) regs.PC += UNCOMPLEMENT(arg1);
            break;

        case 0x09: // ORA #$1f
            regs.A = regs.A | arg1;
            regs.P.zero     = regs.A == 0;
            regs.P.negative = regs.A > 0x7f;
            break;

        case 0x29: // AND #$1f
            regs.A = regs.A & arg1;
            regs.P.zero     = regs.A == 0;
            regs.P.negative = regs.A > 0x7f;
            break;

        case 0x2c: // BIT $0203
            arg21 = (arg2 << 8) | arg1;
            m = readMemory(arg21);
            regs.P.overflow = (m >> 6) & 1;
            regs.P.negative = (m >> 7) & 1;
            regs.P.zero = (m & regs.A) == 0;
            break;
            
        case 0xca: // DEX
            regs.X--;
            regs.P.zero     = regs.X == 0;
            regs.P.negative = regs.X > 0x7f;
            break;

        case 0x88: // DEY
            regs.Y--;
            regs.P.zero     = regs.Y == 0;
            regs.P.negative = regs.Y > 0x7f;
            break;

        case 0xee: // INC $0203
            arg21 = (arg2 << 8) | arg1;
            printf("increment memory %04x\n", arg21);
            m = readMemory(arg21);
            printf("%02x => %02x\n", m, m + 1);
            writeMemory(arg21, m + 1);
            regs.P.zero     = m + 1 == 0;
            regs.P.negative = m + 1 > 0x7f;
            break;

        case 0xc8: // INY
            regs.Y++;
            regs.P.zero     = regs.Y == 0;
            regs.P.negative = regs.Y > 0x7f;
            break;

        case 0x20: // JSR $8100
            arg21 = (arg2 << 8) | arg1;
            printf("S=%02x, JSR from %04x to %04x\n", regs.S, regs.PC, arg21);
            memory[0x0100 + regs.S] = regs.PC >> 8;
            regs.S--;
            memory[0x0100 + regs.S] = regs.PC & 0xff;
            regs.S--;
            regs.PC = arg21;
            break;

        case 0x60: // RTS
            printf("S=%02x, RTS from %04x to ", regs.S, regs.PC);
            regs.S++;
            regs.PC = memory[0x0100 + regs.S];
            regs.S++;
            regs.PC |= memory[0x0100 + regs.S] << 8;
            printf("%04x\n", regs.PC);
            break;

        case 0x4c: // JMP $810c
            arg21 = (arg2 << 8) | arg1;
            regs.PC = arg21;
            break;

        default:
            printf("opcode not implemented (%02x) (%s)\n", ins->opcode, ins->mnemonic);
            exit(1);
    }
}

void readRom(){
    struct NESHeader hdr;

    for(int i = 0; i < 4; i++) hdr.start[i] = rom[i];
    hdr.prg_rom_size = rom[4];
    hdr.chr_rom_size = rom[5];
    hdr.flags6 = rom[6];
    hdr.flags7 = rom[7];
    hdr.flags8 = rom[8];
    hdr.flags9 = rom[9];
    hdr.flags10 = rom[10];
    for(int i = 0 ; i < 5; i++) hdr.unused[i] = rom[11+i];

    printf("{\n");
    printf("\tstart = [%c %c %c %02x]\n", hdr.start[0], hdr.start[1], hdr.start[2], hdr.start[3]);

    printf("\tprg_rom_size = %d\n", hdr.prg_rom_size);
    printf("\tchr_rom_size = %d\n", hdr.chr_rom_size);
    printf("\tflags6 = "); printBits(hdr.flags6); printf("\n");
    printf("\tflags7 = "); printBits(hdr.flags7); printf("\n");
    printf("\tflags8 = "); printBits(hdr.flags8); printf("\n");
    printf("\tflags9 = "); printBits(hdr.flags9); printf("\n");
    printf("\tflags10 = "); printBits(hdr.flags10); printf("\n");
    printf("\tunused = [");
    for(int i = 0; i < 5; i++){
        int c = hdr.unused[i];
        if(isprint(c)) printf("%c", c);
        else printf("%02x", c);
        if(i < 4) printf(" ");
    }
    printf("]\n");
    printf("}\n\n");


    unsigned char mapper = 0;
    mapper = hdr.flags6 >> 4;
    mapper |= hdr.flags7 & 0xf0;
    printf("mapper = %d\n", mapper);

    int prgsize = hdr.prg_rom_size * 16 * 1024;
    int chrsize = hdr.chr_rom_size * 8 * 1024;

    printf("prg rom size = %d\n", prgsize);
    printf("chr rom size = %d\n", chrsize);

    prg = malloc(prgsize);
    chr = malloc(chrsize);

    for(int i = 0; i < prgsize; i++) {
        prg[i] = rom[16 + i];
        memory[0x8000 + i] = rom[16 + i];
    }

    for(int i = 0; i < chrsize; i++) {
        chr[i] = rom[16 + prgsize + i];
    }

    vectors.nmi   = (memory[0xfffb] << 8) | memory[0xfffa];
    vectors.reset = (memory[0xfffd] << 8) | memory[0xfffc];
    vectors.irq   = (memory[0xffff] << 8) | memory[0xfffe];

    for(int j = 0; j < prgsize/16; j++){
        printf("%4x: ", j*16);
        for(int i = 0; i < 16; i++){
            printf("%02x ", prg[j*16 + i]);
        }
        printf("\n");
    }

    printf("nmi   @ $%04x\n", vectors.nmi);
    printf("reset @ $%04x\n", vectors.reset);
    printf("irq   @ $%04x\n", vectors.irq);

}


/*
void dumpRom(){
    FILE* file = fopen("rom.nes", "r");
    if(file==NULL){
        printf("failed to open rom\n");
        exit(1);
    }

    int nread;
    unsigned char buf[16];

    int total = 0;

    printf("unsigned char rom[2561 * 16] = {\n");

    for(;;){
        nread = fread(buf, 1, 16, file);
        if(nread < 0){
            printf("read error\n");
            exit(1);
        }

        if(nread == 0) break;

        total += nread;

        printf("\t");
        for(int i=0; i < nread; i++){
            int c = buf[i];
            if(c == '\\') printf("%3d", c);
            else if(c == '\'') printf("%3d", c);
            else if(isprint(c)) printf("'%c'", c);
            else printf("%3d", c);
            if(i != nread - 1) printf(",");
        }

        if(total == 40976){
            printf("\n};\n");
            break;
        }
        else{
            printf(",\n");
        }

    }

}
*/


// if the CPU reads $2002, then poll the ppuStatus bits and clear bit 7
// if the PPU reaches dot 1 of line 241, set bit 7
//    also at this time, generate an NMI if NMI-on-vblank enabled
// if the PPU dot 1 of "pre-render line" clear bit 7

void writeScreen(int row, int col, int r, int g, int b){
    if(row < 0 || row > screenH - 1){
        printf("row out of bounds (%d) screenH = %d\n", row, screenH);
        exit(-1);
    }
    else if(col < 0 || col > screenW - 1){
        printf("col out of bounds (%d)\n", col);
        exit(-1);
    }
    unsigned char * pixel = screenImg.data + row*screenW*4 + col*4;
    pixel[0] = r;
    pixel[1] = g;
    pixel[2] = b;
}

void resetPPU(){
    memory[0x2000] = 0;
    memory[0x2001] = 0;
    memory[0x2002] = 0;
    memory[0x2003] = 0;
    memory[0x2005] = 0;
    memory[0x2006] = 0;
    memory[0x2007] = 0;
    memory[0x4014] = 0;
}

void uploadScreen(){
    UpdateTexture(screenTex, screenImg.data);
}

int main(){

    readRom();
    resetCPU();
    showCPU();

/*
    for(;;){
        executeInstruction(regs.PC);
        showCPU();
    }
*/

    InitWindow(screenW * screenScale, screenH * screenScale, "Zintendo Entertainment System");
    SetTargetFPS(60);

    screenImg = GenImageColor(screenW,screenH,BLUE);
    screenTex = LoadTextureFromImage(screenImg);

    // screenImg.format probably = R8G8B8A8
    printf("screenImg.width   = %d\n", screenImg.width);
    printf("screenImg.height  = %d\n", screenImg.height);
    printf("screenImg.mipmaps = %d\n", screenImg.mipmaps);
    printf("screenImg.format  = %d\n", screenImg.format);

    int cpuDots = 3 * nextCPUDelay();
    int ptr = 0;

    while(!WindowShouldClose()) {

        // 1 frame is 262 lines, each line is 341 dots.
        // if the next CPU instruction would take N cycles
        // the effects must be done in N*3 dots. So after each CPU cycle
        // reset a dot counter.
        for(int scanline = 0; scanline < 262; scanline++){
            for(int dot = 0; dot < 341; dot++){

                // if we just started line 241, do vertical blanking dance
                if(scanline == 241 && dot == 0){
                    ppuStatus.inVblank = 1;
                    // if NMI out = 1, signal NMI
                }

                // if we just started line 0, undo vertical blanking
                if(scanline == 0 && dot == 0){
                    ppuStatus.inVblank = 0;
                }

                if(scanline >= 1 && scanline <= 240 && dot < 256){
                    int r = chr[ptr];
                    int g = chr[ptr+1];
                    int b = chr[ptr+2];
                    ptr += 4;
                    if(ptr > 0x1ff0) ptr = 0;
/*
                    int r = ((rand()%20) * 255) / 20;
                    int g = ((rand()%20) * 255) / 20;
                    int b = ((rand()%20) * 255) / 20;
                    int r = (rand() % 2) * 255;
*/
                    writeScreen(scanline - 1, dot + 32, r, g, b);
                }

                cpuDots--;
                if(cpuDots == 0){
//printf("(%d,%d) step cpu... ", dot, scanline);
                    stepCPU();
                    cpuDots = 3 * nextCPUDelay();
//printf("... next instruction in %d dots\n", cpuDots);
                }
            }
        }

        uploadScreen();

        BeginDrawing();
            ClearBackground(RAYWHITE);
            Vector2 zero = {0,0};
            DrawTextureEx(screenTex, zero, 0.0f, screenScale, WHITE);
        EndDrawing();
    }

    CloseWindow(); 

    return 0;
}
