a.out: main.c rom.h instructions.h colors.h
	gcc -Wall -I. -I raylib/src main.c raylib/src/libraylib.a -lm
