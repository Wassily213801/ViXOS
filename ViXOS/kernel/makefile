ARCH = x86_64
CC = gcc
CFLAGS = -ffreestanding -m32 -g -Wall -Wextra

SOURCES = kernel.c terminal.c video.c keyboard.c sys.c string.c port_io.c
OBJECTS = $(SOURCES:.c=.o)

all: kernel.elf terminal.i

kernel.elf: $(OBJECTS)
	ld -m elf_i386 -T linker.ld -o kernel.elf $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

terminal.i: terminal.c
	$(CC) $(CFLAGS) -E terminal.c -o terminal.i

clean:
	rm -f *.o *.elf *.i