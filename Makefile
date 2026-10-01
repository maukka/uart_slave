# ==============================================================================
# Project:      USART Slave AVR Chip v0.1
# Description:  Build C -language program that control AVR chip over USART
#               Chip act as a slave and takes commands form Master chip.
#               For example sending command TEMP0 to AVR chip will have next sequence
#               Master ->[TX][USART][RX]->[AVR]->[READ][Pheriperal (temp)]
#               AVR->TX[USART][RX]->[MASTER]->[Memory]->[UI] 
#				
# Owner:        Markus Veijola [markus.veijola@gmail.com]
# Usage:        make            - Builds development build (debug)
#               make release=1  - Builds optimized production version
#               make clean      - Removes compiled artifacts and other products
#               make flash      - Flashes the product (.hex) to AVR flash memory
#               make test       - Runs test cases to verify your code changes
# ==============================================================================

# ==============================================================================
# Installing AVR GCC compiler and needed libraries
# ------------------------------------------------------------------------------
# Windows: Use MSYS2 and pacman package manager to install 
# 1. Update package list
#    pacman -Sy
# 2. List all available avr packages
#    pacman -Ss avr
# 3. Verify the package names from the list. Install these packages  
#    pacman -S mingw-w64-ucrt-x86_64-avr-toolchain ucrt64/mingw-w64-ucrt-x86_64-avrdude
# 4. Verify installation. Note! you must define the compiler paths to environment variables
#    avr-gcc --version
#    avrdude -v
# ------------------------------------------------------------------------------
# Linux
# 1. Install needed libraries and tools
#    sudo apt update
#    sudo apt install gcc-avr avr-libc avrdude
# ------------------------------------------------------------------------------
# MacOS
# 1. Install needed libraries and tools
#    brew tap osx-cross/avr
#    brew install avr-gcc avrdude
# ==============================================================================

# ------------------------------------------------------------------------------
# OS Spesific settings (Cross-platform)
# ------------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    # Jos ajo tapahtuu MSYS2 / Git Bash / Cygwin -ympäristössä, käytetään sh/bash-komentotulkkia
    ifneq ($(MSYSTEM),)
        SHELL := sh.exe
        RM    := rm -rf
    else ifneq ($(CYGWIN),)
        SHELL := sh.exe
        RM    := rm -rf
    else
        SHELL := cmd.exe
        RM    := del /F /Q
    endif
    PORT := COM3
else
    DETECTED_OS := $(shell uname -s)
    SHELL       := /bin/bash
    RM          := rm -rf
    PORT        := /dev/ttyUSB0
endif

# ------------------------------------------------------------------------------
# Make-usage safe settings (Pro-level standard)
# ------------------------------------------------------------------------------

# Delete all zombie files if compilation or linkage fails. 
.DELETE_ON_ERROR:
# Warn about unused variables. This is nice way to find typos also.
MAKEFLAGS += --warn-undefined-variables
# Do not use build in implicit rules of Make. Just use rules 
# defined in this file.
MAKEFLAGS += --no-builtin-rules

# ------------------------------------------------------------------------------
# Arduino Uno / ATmega328P settings
# ------------------------------------------------------------------------------
MCU          ?= atmega328p
F_CPU        ?= 16000000UL     # Uno clock frequency is 16 MHz
BAUD         ?= 115200         # Arduino Uno bootloader file transfer baudrate

# ------------------------------------------------------------------------------
# Directories
# ------------------------------------------------------------------------------
SRC_DIR     := src
INC_DIR     := include
# Find autonmatic all .c files in src/-folder (faster if there is no subfolders)
SRCS := $(wildcard $(SRC_DIR)/*.c)
# Search every .c file in src/ folder and it's subfolders (slower)
#SRCS := $(shell find $(SRC_DIR) -name '*.c')

# Toggle between Release (build/) and Debug (debug/)
RELEASE     ?= 0

ifeq ($(RELEASE), 1)
    BUILD_DIR := build
    BUILD_MODE := Release
else
    BUILD_DIR := debug
    BUILD_MODE := Debug
endif

# Target names
TARGET_BASE := app
ELF         := $(BUILD_DIR)/$(TARGET_BASE).elf
HEX         := $(BUILD_DIR)/$(TARGET_BASE).hex
TARGET := $(BUILD_DIR)/$(TARGET_BASE)

# ------------------------------------------------------------------------------
# AVR-toolchain compiler and other needed tools
# ------------------------------------------------------------------------------
CC      := avr-gcc
# Needed to transfer .elf binary to .hex
OBJCOPY := avr-objcopy
# Needed to calculate object binary size
SIZE    := avr-size
# Needed for flashing to actual board
AVRDUDE := avrdude

# ------------------------------------------------------------------------------
# Compiler flags (CFLAGS)
# ------------------------------------------------------------------------------
CFLAGS       += -mmcu=$(MCU)
CFLAGS       += -DF_CPU=$(F_CPU)
CFLAGS       += -Wall -Wextra  # Warnings on

# ------------------------------------------------------------------------------
# Linker flags for AVR chip
# ------------------------------------------------------------------------------
LDFLAGS := -mmcu=$(MCU)
LDFLAGS += -Wl,-Map=$(TARGET).map,--cref   # Generates a memory map file
LDFLAGS += -Wl,--gc-sections              # Removes unused functions/data to save space

# Math library (if using <math.h>)
LDLIBS  := -lm

# ------------------------------------------------------------------------------
# Object Files
# ------------------------------------------------------------------------------

# We need list of object files names in linking step
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
# Add -Iinclude to compiler flags so #include "header.h" works
CFLAGS += -I$(INC_DIR)

# Mode-dependent compiler flags
ifeq ($(RELEASE), 1)
    CFLAGS += -Os -DNDEBUG
else
    CFLAGS += -O1 -g -DDEBUG
endif

# ------------------------------------------------------------------------------
# Dependency Tracking & Phony Targets
# ------------------------------------------------------------------------------
# Generate .d files automatically alongside .o files
CPPFLAGS := -MMD -MP
CFLAGS   += $(CPPFLAGS)

# Map object files to dependency files
DEPS := $(OBJS:.o=.d)

# Phony targets (targets that aren't actual files on disk)
.PHONY: all info clean flash test

# ------------------------------------------------------------------------------
# 1. Compilation Rule: Compiles source files into object files
# ------------------------------------------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "[CC] $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# ------------------------------------------------------------------------------
# 2. Directory Creation Rule. Note that this rule is executed before 
#    compilation rule. This is verified in compilation rule by piping
#    | $(BUILD_DIR)
# ------------------------------------------------------------------------------
$(BUILD_DIR):
	@echo "[DIR] Creating directory: $@"
ifeq ($(OS),Windows_NT)
  ifneq ($(MSYSTEM),)
	@mkdir -p $@
  else
	@if not exist $(subst /,\,$(BUILD_DIR)) mkdir $(subst /,\,$(BUILD_DIR))
  endif
else
	@mkdir -p $@
endif

# ------------------------------------------------------------------------------
# 3. Linking Rule: Combines all object files into an ELF binary
# ------------------------------------------------------------------------------
$(ELF): $(OBJS) | $(BUILD_DIR)
	@echo "[LINK] $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# ------------------------------------------------------------------------------
# 4. HEX Generation & Size Check Rule
# ------------------------------------------------------------------------------
$(HEX): $(ELF)
	@echo "[OBJCOPY] $@"
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@echo "[SIZE]"
	$(SIZE) --format=avr --mcu=$(MCU) $<

# ------------------------------------------------------------------------------
# 5. Default Target & Informative Banner
# ------------------------------------------------------------------------------
all: info $(HEX)

info:
	@echo "=================================================="
	@echo " Project:    USART Slave AVR Chip"
	@echo " Target:     $(TARGET_BASE)"
	@echo " OS:         $(DETECTED_OS)"
	@echo " Mode:       $(BUILD_MODE) -> ($(BUILD_DIR)/)"
	@echo " Sources:    $(SRCS)"
	@echo "=================================================="

# Include automatically generated header dependencies (.d files)
-include $(DEPS)

# ------------------------------------------------------------------------------
# 6. Flashing Rule (AVRDUDE)
# ------------------------------------------------------------------------------
flash: $(HEX)
	@echo "[FLASH] Flashing $< to $(MCU) on $(PORT)..."
	$(AVRDUDE) -c arduino -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<:i

clean:
	@echo "[CLEAN] Removing build directories..."
ifeq ($(OS),Windows_NT)
  ifneq ($(MSYSTEM),)
	@rm -rf debug build
  else
	@if exist debug rmdir /S /Q debug
	@if exist build rmdir /S /Q build
  endif
else
	@rm -rf debug build
endif

PYTHON   ?= python3

test:
	@echo "[TEST] Running test suite..."
	@$(PYTHON) test/test.py