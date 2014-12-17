# MCU name
MCU = t13

# Fuse settings for ATTiny13
ifeq ($(MCU), t13)
	#FUSE_BITS = -u -U lfuse:w:0xff:m -U hfuse:w:0xcf:m
	HFUSE=0xFB
	LFUSE=0x38
	HEX_FILE_NAME = aprs
endif

AVRDUDE_PROGRAMMER = USBasp
AVRDUDE_PORT = usb        # programmer connected to USB port
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex $(FUSE_BITS)
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)

# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
AVRDUDE_ERASE += -y
# ---------------------------------------------------------------------------
# Define programs and commands.
SHELL = sh
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

# Programming support using avrdude.
AVRDUDE = avrdude

REMOVE = rm -f
COPY = cp

HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:

# Default target.

# Program the device.  
program: aprs.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -u -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

%.hex: %.asm
	@echo
	gavrasm -S aprs.asm 


