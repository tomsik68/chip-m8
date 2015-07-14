CFLAGS=-ansi -Wall -g
LDFLAGS=-lSDL2
OBJECTS=chip8.o chip8_impl.o chipm8.o chip8_cpu.o

chipm8: $(OBJECTS)

.PHONY: clean

clean:
	rm -rf *.o chipm8
