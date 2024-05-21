#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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

int main(int argc, char * argv[]){
    dumpRom();
    return 0;
}

