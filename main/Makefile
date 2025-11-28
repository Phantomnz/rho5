
# Targets:
# - make flash-p:   Compiles and flashes the P-only test (control.c)
# - make flash-pid: Compiles and flashes the full PID controller (pid.c)
# - make clean:     Removes compiled files

# Configuration
MCU = atmega644p
F_CPU = 12000000
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
DUDE_FLAGS = -c usbasp -p m644p

# Compiler Flags 
# Both files need the floating point printf and math libraries
# (-Wl,-u,vfprintf -lprintf_flt -lm)
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Wall -Os
LDFLAGS = -Wl,-u,vfprintf -lprintf_flt -lm

# Target for P-Controller (control.c)
flash-dac: dac.c
	$(CC) $(CFLAGS) $(LDFLAGS) dac.c -o prog-dac.elf
	$(OBJCOPY) -O ihex prog-dac.elf prog-dac.hex
	$(AVRDUDE) $(DUDE_FLAGS) -U flash:w:prog-dac.hex

flash-adc: adc.c
	$(CC) $(CFLAGS) $(LDFLAGS) adc.c -o prog-adc.elf
	$(OBJCOPY) -O ihex prog-adc.elf prog-adc.hex
	$(AVRDUDE) $(DUDE_FLAGS) -U flash:w:prog-adc.hex

flash-pid: pid.c
	$(CC) $(CFLAGS) $(LDFLAGS) pid.c -o prog-pid.elf
	$(OBJCOPY) -O ihex prog-pid.elf prog-pid.hex
	$(AVRDUDE) $(DUDE_FLAGS) -U flash:w:prog-pid.hex

# Clean Target
clean:
	rm -f prog-p.elf prog-p.hex prog-pid.elf prog-pid.hex prog-dac.elf prog-dac.hex prog-adc.elf prog-adc.hex