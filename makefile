ARCH = i386
CC = clang 
AS = nasm
LD = ld
CFLAGS = -ffreestanding -m32 -g -Wall -Wextra -c -fno-stack-protector -fno-pic
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib -z max-page-size=0x1000
OBJS = kernel.o terminal.o video.o keyboard.o sys.o string.o port_io.o pmm.o idt.o isr.o isr_stub.o irq.o timer.o isr_handler.o idt_load.o irq_stub.o login.o boot_screen.o ramdisk.o kernel_panic.o fat.o audio.o gfx.o gfx_window.o gui.o ide.o memory.o port_io_audio.o guess.o calculator.o time.o vixfs.o

ISO_DIR = isofiles
ISO_BOOT = $(ISO_DIR)/boot
ISO_GRUB = $(ISO_DIR)/boot/grub
GRUB_CFG = $(ISO_GRUB)/grub.cfg
ISO_IMAGE = ViXOS_Code_Name_Nova_Build_37.25.iso

BOOTLOADER = boot.bin
BOOT_SRC = boot.asm

RAMDISK_DIR = ramdisk_files
FATROOT_DIR = fatroot
AUDIO_DIR = audio_files
GAMES_DIR = Games
GUESS_GAME_DIR = $(GAMES_DIR)/Guess
UTILS_DIR = Utilities
CALCULATOR_DIR = $(UTILS_DIR)/Calculator
TIME_DIR = $(UTILS_DIR)/Time
DRIVERS_DIR = Drivers
AUDIO_DRIVERS_DIR = $(DRIVERS_DIR)/Audio
STORAGE_DRIVERS_DIR = $(DRIVERS_DIR)/Storage
IDE_DRIVERS_DIR = $(STORAGE_DRIVERS_DIR)/IDE
RTC_DRIVERS_DIR = $(DRIVERS_DIR)/RTC

QEMU = qemu-system-i386
QEMU_FLAGS = -cdrom $(ISO_IMAGE) -m 512M -serial stdio -audio driver=sdl,model=sb16 -drive file=disk.img,format=raw,if=ide -rtc base=localtime

all: $(BOOTLOADER) kernel.bin disk.img $(ISO_IMAGE)

$(BOOTLOADER): $(BOOT_SRC)
	$(AS) -f bin $< -o $@

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

disk.img:
	@echo "Creating test disk image..."
	@dd if=/dev/zero of=disk.img bs=512 count=32768 2>/dev/null
	@echo "Test disk image created: disk.img"

$(ISO_IMAGE): $(BOOTLOADER) kernel.bin disk.img prepare_filesystem
	@echo "Creating ISO image with ATA support, Guess game, Calculator and Time..."
	@mkdir -p $(ISO_BOOT) $(ISO_GRUB)
	@cp $(BOOTLOADER) $(ISO_BOOT)/
	@cp kernel.bin $(ISO_BOOT)/
	@cp disk.img $(ISO_DIR)/
	@echo 'menuentry "ViXOS" {' > $(GRUB_CFG)
	@echo '  multiboot /boot/kernel.bin' >> $(GRUB_CFG)
	@echo '  module /disk.img disk_image' >> $(GRUB_CFG)
	@echo '  boot' >> $(GRUB_CFG)
	@echo '}' >> $(GRUB_CFG)
	@grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR) 2>/dev/null || true
	@echo "ISO image created: $(ISO_IMAGE)"

prepare_filesystem:
	@echo "Preparing filesystem with ATA support, Guess game, Calculator, Time and ViXFS..."
	@mkdir -p $(ISO_DIR)/$(RAMDISK_DIR)
	@echo "RAMDISK initialization script" > $(ISO_DIR)/$(RAMDISK_DIR)/init.vix
	@echo "Welcome to ViXOS!" > $(ISO_DIR)/$(RAMDISK_DIR)/motd.txt
	@echo "System configuration file" > $(ISO_DIR)/$(RAMDISK_DIR)/config.sys
	@echo "ATA Driver loaded successfully" > $(ISO_DIR)/$(RAMDISK_DIR)/ata_status.txt
	@echo "RTC Time Module loaded" > $(ISO_DIR)/$(RAMDISK_DIR)/time_status.txt
	@echo "ViXFS Filesystem initialized" > $(ISO_DIR)/$(RAMDISK_DIR)/vixfs_status.txt
	
	# Создаем все необходимые директории
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/bin
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/etc
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/home/user
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(FS_DRIVERS_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(VIXFS_DRIVERS_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(GAMES_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(GUESS_GAME_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(UTILS_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(CALCULATOR_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/$(TIME_DIR)
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/dev
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/var/log
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/var/lib/vixfs
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/usr/bin
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/usr/lib/vixfs
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/usr/include/vixfs
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/usr/share/doc/vixfs
	@mkdir -p $(ISO_DIR)/$(FATROOT_DIR)/sys/fs/vixfs  # ДОБАВЛЕНО
	
	@echo "System initialization script" > $(ISO_DIR)/$(FATROOT_DIR)/init.vix
	@echo "System configuration" > $(ISO_DIR)/$(FATROOT_DIR)/config.sys
	@echo "Welcome to ViXOS FAT16 filesystem!" > $(ISO_DIR)/$(FATROOT_DIR)/etc/motd.txt
	@echo "User readme file" > $(ISO_DIR)/$(FATROOT_DIR)/home/user/readme.txt
	@echo "Shell application binary" > $(ISO_DIR)/$(FATROOT_DIR)/bin/shell.app
	@echo "Test application binary" > $(ISO_DIR)/$(FATROOT_DIR)/bin/test.app
	@echo "Disk utility" > $(ISO_DIR)/$(FATROOT_DIR)/bin/diskutil.app
	@echo "Guess the Number game" > $(ISO_DIR)/$(FATROOT_DIR)/bin/guess.app
	@echo "Calculator application" > $(ISO_DIR)/$(FATROOT_DIR)/bin/calculator.app
	@echo "Time utility" > $(ISO_DIR)/$(FATROOT_DIR)/bin/time.app
	
	# Добавляем файлы для утилиты времени
	@echo "ViX Time Utility v1.0" > $(ISO_DIR)/$(FATROOT_DIR)/$(TIME_DIR)/readme.txt
	@echo "Time configuration" > $(ISO_DIR)/$(FATROOT_DIR)/$(TIME_DIR)/time.conf
	@echo "Time zone: Europe/Vienna" > $(ISO_DIR)/$(FATROOT_DIR)/$(TIME_DIR)/timezone.conf
	@echo "Time synchronization log" > $(ISO_DIR)/$(FATROOT_DIR)/$(TIME_DIR)/timesync.log
	@echo "RTC hardware interface" > $(ISO_DIR)/$(FATROOT_DIR)/$(TIME_DIR)/rtc_interface.sys
	
	# Добавляем файлы для калькулятора
	@echo "ViX Calculator v1.0" > $(ISO_DIR)/$(FATROOT_DIR)/$(CALCULATOR_DIR)/readme.txt
	@echo "Calculator configuration" > $(ISO_DIR)/$(FATROOT_DIR)/$(CALCULATOR_DIR)/calculator.conf
	@echo "Calculator history log" > $(ISO_DIR)/$(FATROOT_DIR)/$(CALCULATOR_DIR)/history.log
	@echo "Calculator resources" > $(ISO_DIR)/$(FATROOT_DIR)/$(CALCULATOR_DIR)/resources.res
	
	# Добавляем файлы для игры Guess
	@echo "Guess the Number Game" > $(ISO_DIR)/$(FATROOT_DIR)/$(GUESS_GAME_DIR)/readme.txt
	@echo "Game configuration file" > $(ISO_DIR)/$(FATROOT_DIR)/$(GUESS_GAME_DIR)/game.conf
	@echo "High scores will be stored here" > $(ISO_DIR)/$(FATROOT_DIR)/$(GUESS_GAME_DIR)/scores.dat
	@echo "Game resources" > $(ISO_DIR)/$(FATROOT_DIR)/$(GUESS_GAME_DIR)/resources.res
	
	# Создаем драйверы RTC
	@echo "[RTC CMOS Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	@echo "Name: CMOS RTC Driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	@echo "Version: 1.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	@echo "Type: Built-in" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	@echo "Status: Active" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	@echo "Description: CMOS Real-Time Clock driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	@echo "Ports: 0x70, 0x71" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	@echo "Features: BCD conversion, Time/Date reading" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/rtc_cmos.drv
	
	@echo "[Time Service Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/timesvc.drv
	@echo "Name: Time Service" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/timesvc.drv
	@echo "Version: 1.1" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/timesvc.drv
	@echo "Type: Service" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/timesvc.drv
	@echo "Status: Active" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/timesvc.drv
	@echo "Description: System time service and management" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/timesvc.drv
	@echo "Features: Vienna time, CET/CEST, Timezone support" >> $(ISO_DIR)/$(FATROOT_DIR)/$(RTC_DRIVERS_DIR)/timesvc.drv
	
	# Создаем файлы устройств для IDE
	@echo "Character device: Primary Master" > $(ISO_DIR)/$(FATROOT_DIR)/dev/hda
	@echo "Character device: Primary Slave" > $(ISO_DIR)/$(FATROOT_DIR)/dev/hdb
	@echo "Character device: Secondary Master" > $(ISO_DIR)/$(FATROOT_DIR)/dev/hdc
	@echo "Character device: Secondary Slave" > $(ISO_DIR)/$(FATROOT_DIR)/dev/hdd
	@echo "Character device: RTC" > $(ISO_DIR)/$(FATROOT_DIR)/dev/rtc
	
	# Создаем конфигурационные файлы для IDE
	@echo "[IDE Configuration]" > $(ISO_DIR)/$(FATROOT_DIR)/etc/ide.conf
	@echo "PrimaryBase = 0x1F0" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/ide.conf
	@echo "PrimaryControl = 0x3F6" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/ide.conf
	@echo "SecondaryBase = 0x170" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/ide.conf
	@echo "SecondaryControl = 0x376" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/ide.conf
	@echo "Timeout = 100000" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/ide.conf
	@echo "LBAEnabled = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/ide.conf
	
	# Создаем конфигурационные файлы для времени
	@echo "[Time Configuration]" > $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "Timezone = Europe/Vienna" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "DefaultFormat = 24h" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "DaylightSaving = Auto" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "RTCPort = 0x70" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "UpdateFrequency = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	
	# Создаем драйверы IDE
	@echo "[IDE Primary Master Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "Name: IDE Primary Master" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "Version: 1.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "Type: Built-in" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "Status: Active" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "Description: IDE Primary Master controller driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "BasePort: 0x1F0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "ControlPort: 0x3F6" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	@echo "IRQ: 14" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_pm.drv
	
	@echo "[IDE Primary Slave Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "Name: IDE Primary Slave" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "Version: 1.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "Type: Built-in" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "Status: Ready" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "Description: IDE Primary Slave controller driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "BasePort: 0x1F0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "ControlPort: 0x3F6" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	@echo "IRQ: 14" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ps.drv
	
	@echo "[IDE Secondary Master Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "Name: IDE Secondary Master" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "Version: 1.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "Type: Built-in" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "Status: Ready" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "Description: IDE Secondary Master controller driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "BasePort: 0x170" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "ControlPort: 0x376" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	@echo "IRQ: 15" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_sm.drv
	
	@echo "[IDE Secondary Slave Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "Name: IDE Secondary Slave" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "Version: 1.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "Type: Built-in" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "Status: Ready" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "Description: IDE Secondary Slave controller driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "BasePort: 0x170" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "ControlPort: 0x376" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	@echo "IRQ: 15" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ide_ss.drv
	
	@echo "[ATA Protocol Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	@echo "Name: ATA Protocol Driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	@echo "Version: 1.2" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	@echo "Type: Protocol" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	@echo "Status: Active" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	@echo "Description: ATA/ATAPI protocol implementation" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	@echo "Features: LBA28, PIO, Identify" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	@echo "MaxSectors: 256" >> $(ISO_DIR)/$(FATROOT_DIR)/$(IDE_DRIVERS_DIR)/ata.drv
	
	@echo "Setting up audio files..."
	@mkdir -p $(ISO_DIR)/$(AUDIO_DIR)/sounds
	@echo "Audio driver configuration" > $(ISO_DIR)/$(AUDIO_DIR)/audio.conf
	@echo "PC Speaker driver" > $(ISO_DIR)/$(AUDIO_DIR)/pcspk.drv
	@echo "Sound Blaster driver" > $(ISO_DIR)/$(AUDIO_DIR)/sb.drv
	@echo "Intel HD Audio driver" > $(ISO_DIR)/$(AUDIO_DIR)/hda.drv
	@echo "dummy WAV content" > $(ISO_DIR)/$(AUDIO_DIR)/sounds/beep.wav
	@echo "dummy WAV content" > $(ISO_DIR)/$(AUDIO_DIR)/sounds/error.wav
	@echo "dummy WAV content" > $(ISO_DIR)/$(AUDIO_DIR)/sounds/login.wav
	
	@echo "Setting up audio drivers..."
	@echo "[PC Speaker Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/pcspk.drv
	@echo "Name: PC Speaker Driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/pcspk.drv
	@echo "Version: 1.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/pcspk.drv
	@echo "Type: Built-in" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/pcspk.drv
	@echo "Status: Active" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/pcspk.drv
	@echo "Description: Standard PC speaker audio driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/pcspk.drv
	
	@echo "[Sound Blaster 1.0 Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "Name: Sound Blaster 1.0/2.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "Version: 2.1" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "Type: External" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "Status: Ready" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "Description: Creative Sound Blaster 1.0/2.0 compatible driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "Ports: 0x220, 0x240, 0x260, 0x280" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "IRQ: 5,7" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	@echo "DMA: 1" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb10.drv
	
	@echo "[Sound Blaster 16 Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "Name: Sound Blaster 16" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "Version: 4.5" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "Type: External" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "Status: Ready" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "Description: Creative Sound Blaster 16 compatible driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "Ports: 0x220, 0x240, 0x260, 0x280" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "IRQ: 5,7,9,10" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "DMA: 1,5" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	@echo "Features: 16-bit audio, Stereo, MIDI support" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/sb16.drv
	
	@echo "[ICH AC97 Audio Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	@echo "Name: Intel ICH AC97 Audio" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	@echo "Version: 3.2" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	@echo "Type: External" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	@echo "Status: Ready" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	@echo "Description: Intel ICH AC97 compatible audio controller driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	@echo "Base: 0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	@echo "Features: 16/20-bit audio, 48kHz sampling, Stereo" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/ac97.drv
	
	@echo "[Intel HD Audio Driver]" > $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/hda.drv
	@echo "Name: Intel High Definition Audio" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/hda.drv
	@echo "Version: 1.0" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/hda.drv
	@echo "Type: Experimental" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/hda.drv
	@echo "Status: Development" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/hda.drv
	@echo "Description: Intel High Definition Audio controller driver" >> $(ISO_DIR)/$(FATROOT_DIR)/$(AUDIO_DRIVERS_DIR)/hda.drv
	
	@echo "Creating system configuration..."
	@echo "[Audio]" > $(ISO_DIR)/$(FATROOT_DIR)/etc/audio.conf
	@echo "DefaultDevice = AUTO" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/audio.conf
	@echo "SampleRate = 44100" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/audio.conf
	@echo "Channels = 2" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/audio.conf
	@echo "Volume = 80" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/audio.conf
	@echo "EnableBeep = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/audio.conf
	
	@echo "[Storage]" > $(ISO_DIR)/$(FATROOT_DIR)/etc/storage.conf
	@echo "PrimaryIDE = Enabled" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/storage.conf
	@echo "SecondaryIDE = Enabled" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/storage.conf
	@echo "LBAMode = Enabled" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/storage.conf
	@echo "Timeout = 100000" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/storage.conf
	
	@echo "[Time]" > $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "Timezone = Europe/Vienna" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "DefaultFormat = 24h" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "DaylightSaving = Auto" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "RTCPort = 0x70" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	@echo "UpdateFrequency = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/time.conf
	
	@echo "[Games]" > $(ISO_DIR)/$(FATROOT_DIR)/etc/games.conf
	@echo "GuessEnabled = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/games.conf
	@echo "GuessMaxAttempts = 10" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/games.conf
	@echo "GuessMinNumber = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/games.conf
	@echo "GuessMaxNumber = 100" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/games.conf
	
	@echo "[Utilities]" > $(ISO_DIR)/$(FATROOT_DIR)/etc/utils.conf
	@echo "CalculatorEnabled = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/utils.conf
	@echo "CalculatorMaxHistory = 50" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/utils.conf
	@echo "TimeEnabled = 1" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/utils.conf
	@echo "TimeCommand = time" >> $(ISO_DIR)/$(FATROOT_DIR)/etc/utils.conf
	
	@echo "Filesystem preparation with ATA support, Guess game, Calculator and Time complete."
	
run: all
	@echo "Starting QEMU with ViXOS, audio, IDE support, Guess game, Calculator and Time..."
	@$(QEMU) $(QEMU_FLAGS)

run_with_pc_speaker: all
	@echo "Starting QEMU with PC Speaker support, Guess game, Calculator and Time..."
	@$(QEMU) -cdrom $(ISO_IMAGE) -m 512M -serial stdio -no-reboot -no-shutdown -audio driver=sdl,model=pcspk -drive file=disk.img,format=raw,if=ide -rtc base=localtime

run_with_sb16: all
	@echo "Starting QEMU with Sound Blaster 16 support, Guess game, Calculator and Time..."
	@$(QEMU) -cdrom $(ISO_IMAGE) -m 512M -serial stdio -no-reboot -no-shutdown -audio driver=sdl,model=sb16 -drive file=disk.img,format=raw,if=ide -rtc base=localtime

run_with_ac97: all
	@echo "Starting QEMU with AC97 support, Guess game, Calculator and Time..."
	@$(QEMU) -cdrom $(ISO_IMAGE) -m 512M -serial stdio -no-reboot -no-shutdown -audio driver=sdl,model=ac97 -drive file=disk.img,format=raw,if=ide -rtc base=localtime

run_no_audio: all
	@echo "Starting QEMU without audio support, with Guess game, Calculator and Time..."
	@$(QEMU) -cdrom $(ISO_IMAGE) -m 512M -serial stdio -no-reboot -no-shutdown -drive file=disk.img,format=raw,if=ide -rtc base=localtime

clean:
	@echo "Cleaning up..."
	@rm -f *.o *.elf $(BOOTLOADER) kernel.bin disk.img
	@rm -rf $(ISO_DIR)
	@rm -f $(ISO_IMAGE)
	@echo "Clean complete."

.PHONY: all clean prepare_filesystem run run_with_pc_speaker run_with_sb16 run_with_ac97 run_no_audio