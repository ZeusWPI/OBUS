ARCHITECTURE = avr
BOARD_TAG    = nano
BOARD_SUB    = atmega328old
ARDUINO_LIBS = arduino-mcp2515

ARDUINO_DEV ?= /dev/ttyUSB0

ARDUINO_VERSION    ?= 1.5
ifeq ($(origin ARDUINO_SKETCHBOOK), undefined)
ARDUINO_SKETCHBOOK := $(shell sed -n 's/^sketchbook.path=//p' ~/.arduino$(subst .,,$(ARDUINO_VERSION))/preferences.txt)
endif
ifeq ($(origin BOARDS_TXT), undefined)
BOARDS_TXT         := $(shell locate '*/arduino/*/boards.txt')  # ¯\_(ツ)_/¯
endif

ifeq ($(wildcard /usr/share/arduino),)
ARDUINO_DIR ?= /usr/lib/arduino
else
ARDUINO_DIR ?= /usr/share/arduino
endif

ARDMK_DIR          ?= ./arduino-makefile
AVR_TOOLS_DIR      ?= /usr

include $(ARDMK_DIR)/Arduino.mk
