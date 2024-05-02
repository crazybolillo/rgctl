#include "rgctl.h"

#include <stm8s.h>
#include <u8x8.h>

#include "led1642.h"
#include "rgb.h"
#include "ui.h"

const uint8_t CPU_CLK_MHZ = 16;

const uint32_t I2C_SPEED = 100000;
const uint8_t I2C_SLAVE_ADDRESS_7 = 0x78;

volatile uint8_t i2c_current = 0;
volatile uint8_t i2c_bytes_left = 0;
volatile uint8_t *i2c_bytes = NULL;

u8x8_t display;

struct Message message = {0};
const volatile uint8_t *ui_state;
uint8_t prev_state, ui_red, ui_green, ui_blue, counter;
uint8_t *chosen_color;
uint32_t chosen_color_addr;

const uint16_t brightness_delta = 32;
uint16_t brightness;

const uint32_t flash_red_addr = FLASH_DATA_START_PHYSICAL_ADDRESS;
const uint32_t flash_green_addr = flash_red_addr + 1;
const uint32_t flash_blue_addr = flash_green_addr + 1;
const uint32_t flash_bright_addr_high = flash_blue_addr + 1;
const uint32_t flash_bright_addr_low = flash_bright_addr_high + 1;

_inline void delay_cycles(uint16_t cycles) { _asm("nop\n $N:\n decw X\n jrne $L\n nop\n", cycles); }
_inline void delay_us(uint16_t micro) { delay_cycles(us_cycles(micro)); }

/**
 * Generic helper function to standardize UI delays across the program. Waits for approx. 600 ms.
 * @return
 */
_inline void delay_user(void) {
    for (counter = 0; counter < 10; counter++) { delay_us(60000); }
}

uint8_t i2c_hw_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            i2c_current = 0;
            i2c_bytes_left = arg_int;
            i2c_bytes = arg_ptr;
            while (i2c_current < i2c_bytes_left) {
                I2C->DR = i2c_bytes[i2c_current++];
                while (!(I2C->SR1 & I2C_SR1_TXE))
                    ;
            }
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            I2C->CR2 |= I2C_CR2_START;
            while (!(I2C->SR1 & I2C_SR1_SB))
                ;

            I2C->DR = I2C_SLAVE_ADDRESS_7;
            while (!(I2C->SR1 & I2C_SR1_ADDR))
                ;
            (void)I2C->SR3;
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            I2C->CR2 |= I2C_CR2_STOP;
            while (I2C->SR3 & I2C_SR3_MSL)
                ;
            break;
        case U8X8_MSG_BYTE_INIT:
        case U8X8_MSG_BYTE_SET_DC:
            break;
        default:
            return 0;
    }

    return 1;
}

uint8_t gpio_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    if (msg == U8X8_MSG_DELAY_MILLI) { delay_us(arg_int * 1000); }

    return 1;
}

void setupHardware(void) {
    _asm("sim");
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
    FLASH_Unlock(FLASH_MEMTYPE_DATA);

    ui_red = FLASH_ReadByte(flash_red_addr);
    ui_red -= ui_red % 16;

    ui_green = FLASH_ReadByte(flash_green_addr);
    ui_green -= ui_green % 16;

    ui_blue = FLASH_ReadByte(flash_blue_addr);
    ui_blue -= ui_blue % 16;
    if ((ui_red == 0) && (ui_green == 0) && (ui_blue == 0)) {
        ui_red = 80;
        ui_blue = 32;
    }

    brightness = FLASH_ReadByte(flash_bright_addr_high);
    brightness = (int16_t)(brightness << 8);
    brightness |= FLASH_ReadByte(flash_bright_addr_low);
    if (brightness > 4095) {
        brightness = 4095;
    } else if (brightness == 0) {
        brightness = brightness_delta;
    }

    led1642_init(&message);
    rgb_init(ui_red, ui_green, ui_blue);

    ui_state = ui_init();
    prev_state = *ui_state;

    GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_LOW_SLOW);

    /**
     * I2C Setup for SSD1306 OLED screen
     * Run at a 100 KHz in Master Transmit mode
     */
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, ENABLE);
    I2C_DeInit();
    I2C_Init(I2C_SPEED, 0x00, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, CPU_CLK_MHZ);

    /**
     * Pin whose only purpose is to alert that the program has been interrupted by a TRAP ISR.
     */
    GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUT_PP_LOW_SLOW);
}

void handle_color_encoder() {
    while (!ui_timeout()) {
        switch (ui_read_encoder()) {
            case UI_ENCODER_CLOCKWISE:
                *chosen_color += 16;
                break;
            case UI_ENCODER_COUNTERCLOCK:
                *chosen_color -= 16;
                break;
            default:
                continue;
        }
        ui_reset_timeout();
        rgb_set(ui_red, ui_green, ui_blue);
        while ((GPIO_ReadInputData(GPIOC) & 0x06) != 0x06) {}
        ui_enable_encoder();
    }
    ui_stop_timeout();
    ui_stop_timeout();
    if ((chosen_color_addr >= flash_red_addr) && (chosen_color_addr <= flash_blue_addr)) {
        FLASH_ProgramByte(chosen_color_addr, *chosen_color);
    }
}

void handle_brightness_encoder() {
    rgb_start();
    while (!ui_timeout()) {
        switch (ui_read_encoder()) {
            case UI_ENCODER_CLOCKWISE:
                brightness += brightness_delta;
                if (brightness > 4095) { brightness = 4095; }
                break;
            case UI_ENCODER_COUNTERCLOCK:
                brightness -= brightness_delta;
                if (brightness == 0) { brightness = brightness_delta; }
                break;
            default:
                continue;
        }
        ui_reset_timeout();
        led1642_set_brightness(brightness);
        while ((GPIO_ReadInputData(GPIOC) & 0x06) != 0x06) {}
        ui_enable_encoder();
    }
    ui_stop_timeout();
    FLASH_ProgramByte(flash_bright_addr_high, brightness >> 8);
    FLASH_ProgramByte(flash_bright_addr_low, brightness);
}

int main(void) {
    setupHardware();
    _asm("rim");

    //    u8x8_Setup(&display, u8x8_d_ssd1306_128x64_noname, u8x8_cad_ssd13xx_i2c, i2c_hw_byte_cb,
    //    gpio_delay_cb); u8x8_InitDisplay(&display); u8x8_SetPowerSave(&display, 0);
    //    u8x8_ClearDisplay(&display);
    //
    //    u8x8_SetFont(&display, u8x8_font_courB18_2x3_r);
    //    u8x8_DrawString(&display, 1, 10, "Coracao");

    // Set CFG-15=1, enables 12 bit PWM counter
    message.data = 1 << L1642_TWO_BYTES;
    message.size = L1642_TWO_BYTES;
    message.latch = L1642_WR_CR_LATCH;
    led1642_transmit();

    // Turn all outputs ON
    message.data = L1642_ALL_ONES;
    message.size = L1642_TWO_BYTES;
    message.latch = L1642_WR_SW_LATCH;
    led1642_transmit();

    led1642_set_brightness(brightness);
    rgb_start();

    while (1) {
        if ((*ui_state == 0) || (*ui_state == prev_state)) { continue; }

        ui_red = rgb_read_red();
        ui_green = rgb_read_green();
        ui_blue = rgb_read_blue();
        switch (*ui_state) {
            case UI_EVENT_RED_SEL:
                rgb_set(255, 0, 0);
                chosen_color = &ui_red;
                chosen_color_addr = flash_red_addr;
                break;
            case UI_EVENT_GREEN_SEL:
                rgb_set(0, 255, 0);
                chosen_color = &ui_green;
                chosen_color_addr = flash_green_addr;
                break;
            case UI_EVENT_BLUE_SEL:
                rgb_set(0, 0, 255);
                chosen_color = &ui_blue;
                chosen_color_addr = flash_blue_addr;
                break;
            case UI_EVENT_BRIGHTNESS_SEL:
                rgb_off();
                chosen_color = NULL;
                break;
            default:
                continue;
        }

        delay_user();
        rgb_set(ui_red, ui_green, ui_blue);
        prev_state = *ui_state;
        ui_enable_click();

        ui_start_timeout();
        ui_enable_encoder();
        if (chosen_color != NULL) {
            handle_color_encoder();
        } else {
            handle_brightness_encoder();
        }

        rgb_off();
        delay_user();
        rgb_start();
    }
}
