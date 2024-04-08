LD = clnk
AS = castm8
CC = cxstm8
PROGRAMMER ?= stlinkv3

INCLUDE := -i src -i vendor/stsw/inc -i vendor/u8x8

CFLAGS := +modsl0 +debug -pw2 -dSTM8S105 +split -dTYPING
CFLAGS += $(INCLUDE)
CFLAGS += $(shell command -v cxstm8 | xargs dirname | xargs printf "-i %s/hstm8")

BUILD_DIR = build
OBJS = $(addprefix build/, \
	vector.o \
	crtsi0.o \
	rgctl.o \
	stm8s_gpio.o \
	stm8s_clk.o \
	stm8s_flash.o \
	stm8s_tim1.o \
	stm8s_tim3.o \
	stm8s_i2c.o \
	u8x8_setup.o \
	u8x8_byte.o \
	u8x8_gpio.o \
	u8x8_fonts.o \
	u8x8_cad.o \
	u8x8_display.o \
	u8x8_8x8.o \
	u8x8_d_ssd1306_128x64_noname.o \
	stm8s_tim4.o \
	led1642.o \
	rgb.o \
	stm8s_tim2.o \
)

VPATH = src:vendor/stsw/src:vendor/u8x8

.PHONY: all clean flash unlock

all: $(BUILD_DIR)/rgctl.hex $(BUILD_DIR)/rgctl.elf

$(BUILD_DIR)/rgctl.elf: $(BUILD_DIR)/rgctl.sm8
	cvdwarf  -o $@ $<
	size $@

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
	stm8flash -c $(PROGRAMMER) -p stm8s105?6 -w build/rgctl.hex

unlock:
	stm8flash -c $(PROGRAMMER) -p stm8s105?6 -u
