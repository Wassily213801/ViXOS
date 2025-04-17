CC=gcc
AS=as
LD=ld
CFLAGS=-ffreestanding -m32 -g -Wall -Wextra -c
LDFLAGS=-m elf_i386 -T linker.ld
OBJS=kernel.o terminal.o video.o keyboard.o sys.o string.o port_io.o pmm.o idt.o isr.o isr_stub.o irq.o timer.o isr_handler.o idt_load.o irq_stub.o login.o

all: kernel.elf

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

terminal.i: terminal.c
	$(LD) $(LDFLAGS) -E $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

isr_stub.o: isr_stub.s
	$(AS) --32 $< -o $@

idt_load.o: idt_load.s
	$(AS) --32 $< -o $@

irq_stub.o: irq_stub.s
	as --32 $< -o $@

clean:
	rm -f *.o *.elf
