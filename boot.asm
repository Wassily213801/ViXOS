[bits 16]
[org 0x7C00]

; Константы
KERNEL_LOAD_SEGMENT  equ 0x1000
KERNEL_LOAD_OFFSET   equ 0x0000
KERNEL_START_SECTOR  equ 2
KERNEL_SECTOR_COUNT  equ 100
STACK_POINTER        equ 0x7C00
PROT_MODE_STACK      equ 0x90000

jmp start

; Данные и сообщения
boot_msg      db "ViXBoot v1.0 - Loading ViXOS...", 13, 10, 0
loading_msg   db "Loading ViXOS kernel...", 13, 10, 0
error_msg     db "Disk error! System halted.", 0
success_msg   db "Kernel loaded successfully!", 13, 10, 0
a20_msg       db "A20 line enabled", 13, 10, 0
pmode_msg     db "Switching to protected mode...", 13, 10, 0
kernel_missing_msg db "Kernel not found! Halting.", 0

; Функция вывода строки
print_string:
    pusha
    mov ah, 0x0E
    mov bh, 0x00
.print_loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .print_loop
.done:
    popa
    ret

; Функция чтения символа (не используется сейчас)
read_char:
    xor ah, ah
    int 0x16
    ret

; Перевод строки
new_line:
    pusha
    mov ah, 0x0E
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10
    popa
    ret

; Проверка наличия ядра на диске
check_kernel:
    pusha
    mov ah, 0x02
    mov al, 1      ; Читаем 1 сектор для проверки
    mov ch, 0
    mov cl, KERNEL_START_SECTOR
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x7E00 ; Временный буфер
    
    int 0x13
    jc .kernel_not_found
    
    ; Ядро найдено
    popa
    ret
    
.kernel_not_found:
    mov si, kernel_missing_msg
    call print_string
    jmp halt

; Включение A20
enable_a20:
    pusha
    mov si, a20_msg
    call print_string
    
    ; Быстрый метод A20
    in al, 0x92
    or al, 2
    out 0x92, al
    popa
    ret

; Загрузка ядра
load_kernel:
    pusha
    mov si, loading_msg
    call print_string
    
    ; Пробуем загрузить ядро несколько раз
    mov cx, 3
.load_retry:
    push cx
    
    mov ax, KERNEL_LOAD_SEGMENT
    mov es, ax
    mov bx, KERNEL_LOAD_OFFSET
    
    ; Загрузка ядра с диска
    mov ah, 0x02
    mov al, KERNEL_SECTOR_COUNT
    mov ch, 0
    mov cl, KERNEL_START_SECTOR
    mov dh, 0
    mov dl, 0x80
    
    int 0x13
    jnc .load_success
    
    ; Ошибка загрузки, пробуем снова
    pop cx
    loop .load_retry
    
    ; Все попытки неудачны
    jmp .error

.load_success:
    pop cx
    mov si, success_msg
    call print_string
    popa
    ret
    
.error:
    mov si, error_msg
    call print_string
    jmp halt

; Остановка системы
halt:
    cli
    hlt
    jmp halt

; Основная процедура
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, STACK_POINTER
    sti
    
    ; Очистка экрана
    mov ax, 0x0003
    int 0x10
    
    ; Показываем сообщение о загрузке
    mov si, boot_msg
    call print_string
    call new_line
    
    ; Проверяем наличие ядра
    call check_kernel
    
    ; Загружаем ядро
    call load_kernel
    
    ; Проверяем сигнатуру ядра
    call verify_kernel
    
    ; Включаем A20
    call enable_a20
    
    ; Загружаем GDT
    mov si, pmode_msg
    call print_string
    cli
    lgdt [gdt_descriptor]
    
    ; Переход в защищенный режим
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Дальний переход для очистки конвейера
    jmp CODE_SEG:protected_mode
; GDT
gdt_start:
    dq 0x0000000000000000    ; Null descriptor

gdt_code:
    dw 0xFFFF                ; Limit 0-15
    dw 0x0000                ; Base 0-15
    db 0x00                  ; Base 16-23
    db 10011010b             ; P=1, DPL=00, S=1, Type=1010 (code, read/exec)
    db 11001111b             ; G=1, D=1, L=0, Limit 16-19=1111
    db 0x00                  ; Base 24-31

gdt_data:
    dw 0xFFFF                ; Limit 0-15
    dw 0x0000                ; Base 0-15
    db 0x00                  ; Base 16-23
    db 10010010b             ; P=1, DPL=00, S=1, Type=0010 (data, read/write)
    db 11001111b             ; G=1, D=1, L=0, Limit 16-19=1111
    db 0x00                  ; Base 24-31

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Заполнение до 510 байт и сигнатура загрузочного сектора
times 510-($-$$) db 0
dw 0xAA55

; 32-битный код (начинается с сектора 2)
[bits 32]

protected_mode:
    ; Инициализация сегментов данных
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Инициализация стека
    mov esp, PROT_MODE_STACK
    
    ; Очистка экрана (зеленый текст на черном фоне)
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ax, 0x0F20  ; Пробел с атрибутом (белый на черном)
.clear_loop:
    mov [edi], ax
    add edi, 2
    loop .clear_loop
    
    ; Выводим сообщение о переходе к ядру
    mov esi, kernel_msg
    mov edi, 0xB8000
    mov ah, 0x0F    ; Белый на черном
.print_loop:
    lodsb
    test al, al
    jz .jump_to_kernel
    mov [edi], ax
    add edi, 2
    jmp .print_loop
    
.jump_to_kernel:
    ; Переход к ядру по адресу 0x10000
    jmp CODE_SEG:0x10000

kernel_msg db "Bootloader: Jumping to ViXOS kernel at 0x10000...", 0

; Заполняем до 1024 байт (для теста)
times 1024-($-$$) db 0

; Проверка сигнатуры ядра
verify_kernel:
    pusha
    mov ax, KERNEL_LOAD_SEGMENT
    mov es, ax
    mov di, KERNEL_LOAD_OFFSET
    
    ; Проверяем мультизагрузочную сигнатуру
    mov eax, [es:di]
    cmp eax, 0x1BADB002
    je .kernel_valid
    
    ; Сигнатура не совпадает
    mov si, kernel_invalid_msg
    call print_string
    jmp halt

.kernel_valid:
    popa
    ret

kernel_invalid_msg db "Invalid kernel signature!", 13, 10, 0