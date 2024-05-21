mario: main.c apu.c apu.h rom.h instructions.h colors.h
	gcc -o mario -Wall -I. -I raylib/src main.c apu.c raylib/src/libraylib.a -lm

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
