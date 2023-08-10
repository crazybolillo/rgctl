LD = clnk
AS = castm8
CC = cxstm8

INCLUDE := -i src -i vendor/stsw/inc

CFLAGS := +modsl0 +debug +warn +strict -dSTM8S105 +split
CFLAGS += $(INCLUDE)
CFLAGS += $(shell command -v cxstm8 | xargs dirname | xargs printf "-i %s/hstm8")

BUILD_DIR = build
OBJS = $(addprefix build/, vector.o crtsi0.o rgctl.o stm8s_gpio.o stm8s_clk.o stm8s_flash.o stm8s_tim1.o stm8s_tim3.o)

VPATH = src:vendor/stsw/src

.PHONY: all clean flash

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

flash:
	stm8flash -c stlinkv2 -p stm8s105?6 -w build/rgctl.hex
