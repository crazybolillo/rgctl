AS=castm8
CC=cxstm8
CFLAGS=+modsl0 +debug +warn +strict
CFLAGS+=$(shell command -v cxstm8 | xargs dirname | xargs printf "-i %s/hstm8")
LD=clnk

BUILD_DIR=build
OBJS=$(addprefix build/, vector.o crtsi0.o rgctl.o)

VPATH = src

.PHONY: all clean

all: $(BUILD_DIR)/rgctl.hex $(BUILD_DIR)/rgctl.elf

$(BUILD_DIR)/rgctl.elf: $(BUILD_DIR)/rgctl.sm8
	cvdwarf  -o $@ $<

$(BUILD_DIR)/rgctl.hex: $(BUILD_DIR)/rgctl.sm8
	chex -fi -o $@ $<

$(BUILD_DIR)/rgctl.sm8: $(OBJS)
	$(LD) -o $@ -m $(BUILD_DIR)/rgctl.map rgctl.lkf $(OBJS)

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) -o $@ $<

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -co $(BUILD_DIR) $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -dr $(BUILD_DIR)/*
