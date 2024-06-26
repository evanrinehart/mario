mario: main.c apu.c posix_stash.c rom.h instructions.h colors.h
	gcc -o mario -Wall -I. -I raylib/src main.c apu.c posix_stash.c raylib/src/libraylib.a -lm -lpthread

mario.exe:
	gcc -o mario.exe -Wall -I. -I raylib/src main.c apu.c windows_stash.c raylib/src/libraylib.a -lm -lpthread -lgdi32 -lwinmm

headerize: headerize.c
	gcc -o headerize -Wall headerize.c

rom.h: headerize rom.nes
	./headerize rom.nes > rom.h

rom.nes:
	@echo "*"
	@echo "*"
	@echo "* You there. Do: ln -s <the rom file> rom.nes"
	@echo "*"
	@echo "*"
	@exit 1

clean:
	rm headerize
	rm rom.h
	rm mario
