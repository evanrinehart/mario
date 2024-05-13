a.out: main.c
	gcc -Wall -I. -I raylib/src main.c raylib/src/libraylib.a -lm
