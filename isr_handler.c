#include "video.h"
#include "serial.h"
#include "string.h"
#include <stdint.h>

typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t err_code;
    uint32_t int_no;
} regs_t;

static void print_reg(const char* name, uint32_t val) {
    char buf[16];
    itoa(val, buf, 16);
    video_print(name); video_print(": 0x"); video_print(buf); video_print("\n");
    serial_write(name); serial_write(": 0x"); serial_write(buf); serial_write("\n");
}

void isr_handler(regs_t* r) {
    video_print("[ISR] interrupt: ");
    char num[4]; itoa(r->int_no, num, 10);
    video_print(num); video_print("\n");
    serial_write("[ISR] interrupt: "); serial_write(num); serial_write("\n");

    print_reg("EAX", r->eax);
    print_reg("EBX", r->ebx);
    print_reg("ECX", r->ecx);
    print_reg("EDX", r->edx);
    print_reg("ESI", r->esi);
    print_reg("EDI", r->edi);
    print_reg("EBP", r->ebp);
    print_reg("ESP", r->esp);
    print_reg("EIP/ERR", r->err_code);
}