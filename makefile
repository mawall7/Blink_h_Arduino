# ---- Project ----
TARGET = app
MCU    = atmega328p
F_CPU  = 16000000UL

# ---- Directories ----
SRCDIR   = src
INCDIR   = include
BUILDDIR = build

# ---- Toolchain ----
CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size

# ---- Sources (auto) ----
SRC  := $(wildcard $(SRCDIR)/*.c)
OBJ  := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))
DEP  := $(OBJ:.o=.d)

# ---- Flags ----
CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=c11 \
          -ffunction-sections -fdata-sections -I$(INCDIR) \
          -MMD -MP
LDFLAGS = -Wl,--gc-sections

# ---- Avrdude / Upload (Windows) ----
PORT       = COM5
PROGRAMMER = arduino
BAUD       = 115200

AVRDUDE = avrdude
AVRDUDE_FLAGS = -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD)

# ---- Targets ----
ELF = $(BUILDDIR)/$(TARGET).elf
HEX = $(BUILDDIR)/$(TARGET).hex

all: $(HEX) size

# Skapa build-katalog vid behov
$(BUILDDIR):
	@if not exist $(BUILDDIR) mkdir $(BUILDDIR)

# Länka
$(ELF): $(BUILDDIR) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

# .hex
$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Kompilera varje .c -> build/*.o
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

size: $(ELF)
	$(SIZE) $<

flash: $(HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -D -U flash:w:$<:i

clean:
	@if exist $(BUILDDIR) rmdir /S /Q $(BUILDDIR)

.PHONY: all flash clean size

-include $(DEP)