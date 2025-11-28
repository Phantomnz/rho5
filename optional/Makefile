
# Hardware Configuration 
MCU     = atmega644p
F_CPU   = 12000000
PROGRAMMER = usbasp
BAUD    = 19200 
# (Baud is ignored by usbasp but required by some versions of avrdude args)

# Toolchain 
CC      = avr-gcc
CXX     = avr-g++
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Flags
# -Os: Optimize for size (critical for microcontrollers)
# -mmcu: Specify the chip
# -DF_CPU: Define clock speed for delay macros
CXXFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall
# Adding support for printf (sending floats) and scanf (reading floats)
LDFLAGS  = -mmcu=$(MCU) -lm -Wl,--allow-multiple-definition

# Source Files 
SOURCES = firmware.cpp \
          ADCReader.cpp \
          AVRSerial.cpp \
          PIDController.cpp \
          PWMTimer.cpp

# Output Names
TARGET_ELF = firmware.elf
TARGET_HEX = firmware.hex


# Targets

# Default: Just Compile
all: $(TARGET_HEX)

# Link: Create ELF from object files
$(TARGET_ELF): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET_ELF)

# Hex: Convert ELF to HEX (required for flashing)
$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET_ELF) $(TARGET_HEX)

# Flash: Write to the board
flash: $(TARGET_HEX)
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -U flash:w:$(TARGET_HEX)

# Clean: Remove build artifacts
clean:
	rm -f *.elf *.hex *.o