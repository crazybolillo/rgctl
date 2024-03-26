#include <stm8s.h>
#include <u8x8.h>
#include "rgctl.h"
#include "led1642.h"


const uint8_t CPU_CLK_MHZ = 16;

const uint32_t I2C_SPEED = 100000;
const uint8_t  I2C_SLAVE_ADDRESS_7 = 0x78;

volatile uint16_t sysTick = 0;
volatile uint8_t i2c_current = 0;
volatile uint8_t i2c_bytes_left = 0;
volatile uint8_t *i2c_bytes = NULL;

u8x8_t display;

struct Message message = {0};

_inline void delay_cycles(uint16_t cycles) {
    _asm("nop\n $N:\n decw X\n jrne $L\n nop\n", cycles);
}

_inline void delay_us(uint16_t micro) {
    delay_cycles(us_cycles(micro));
}

isr void sysTickHandler() {
    sysTick++;
    TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
}

uint8_t i2c_hw_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_BYTE_SEND:
            i2c_current = 0;
            i2c_bytes_left = arg_int;
            i2c_bytes = arg_ptr;
            while (i2c_current < i2c_bytes_left) {
                I2C->DR = i2c_bytes[i2c_current++];
                while (!(I2C->SR1 & I2C_SR1_TXE));
            }
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            I2C->CR2 |= I2C_CR2_START;
            while (!(I2C->SR1 & I2C_SR1_SB));

            I2C->DR = I2C_SLAVE_ADDRESS_7;
            while (!(I2C->SR1 & I2C_SR1_ADDR));
            (void)I2C->SR3;
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            I2C->CR2 |= I2C_CR2_STOP;
            while (I2C->SR3 & I2C_SR3_MSL);
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
    if (msg == U8X8_MSG_DELAY_MILLI) {
        delay_us(arg_int * 1000);
    }

    return 1;
}

void setupHardware(void) {
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    led1642_init(&message);

    /**
     * TIM1 CH3
     * Used for debugging purposes. Should output a 1kHz signal representing the system tick
     */
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);
    TIM1_TimeBaseInit(16, TIM1_COUNTERMODE_UP, 1000, 0);
    TIM1_OC3Init(
        TIM1_OCMODE_PWM1,
        TIM1_OUTPUTSTATE_ENABLE,
        TIM1_OUTPUTNSTATE_DISABLE,
        500,
        TIM1_OCPOLARITY_HIGH,
        TIM1_OCNPOLARITY_LOW,
        TIM1_OCIDLESTATE_RESET,
        TIM1_OCNIDLESTATE_RESET
    );
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    TIM1_CtrlPWMOutputs(ENABLE);
    TIM1_Cmd(ENABLE);

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
    GPIO_WriteLow(GPIOC, GPIO_PIN_7);
}

int main(void) {
    setupHardware();
    _asm("rim");

//    u8x8_Setup(&display, u8x8_d_ssd1306_128x64_noname, u8x8_cad_ssd13xx_i2c, i2c_hw_byte_cb, gpio_delay_cb);
//    u8x8_InitDisplay(&display);
//    u8x8_SetPowerSave(&display, 0);
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

    led1642_set_brightness(1024);

	while(1) {}
}
