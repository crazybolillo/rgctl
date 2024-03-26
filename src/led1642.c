#include "led1642.h"

enum {
    LE = GPIO_PIN_6,
    SCLK = GPIO_PIN_5,
    SDO = GPIO_PIN_4,
    BRIGHTNESS_LATCH = 4,
    BRIGHTNESS_GLOBAL_LATCH = 6,
    BRIGHTNESS_REG_COUNT = 16
};

static volatile bool finished_tx = FALSE;
static volatile bool rising = TRUE;
static struct Message *message;

_inline static void clock_on(void) {
    GPIO_WriteHigh(GPIOD, SCLK);
    GPIO_WriteHigh(GPIOC, SCLK);
}

_inline static void clock_off(void) {
    GPIO_WriteLow(GPIOD, SCLK);
    GPIO_WriteLow(GPIOC, SCLK);
}

_inline static void sdo_on(void) {
    GPIO_WriteHigh(GPIOD, SDO);
    GPIO_WriteHigh(GPIOC, SDO);
}

_inline static void sdo_off(void) {
    GPIO_WriteLow(GPIOD, SDO);
    GPIO_WriteLow(GPIOC, SDO);
}

_inline static void le_on(void) {
    GPIO_WriteHigh(GPIOD, LE);
    GPIO_WriteHigh(GPIOC, LE);
}

_inline static void le_off(void) {
    GPIO_WriteLow(GPIOD, LE);
    GPIO_WriteLow(GPIOC, LE);
}

void led1642_init(struct Message *msg) {
    message = msg;

    /**
     * TIM3 CH1
     * Generates PWCLK signal for LED1642, 500kHz
     */
    GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_SLOW);
    TIM3_TimeBaseInit(TIM3_PRESCALER_1, 32);
    TIM3_OC1Init(
            TIM3_OCMODE_PWM1,
            TIM3_OUTPUTSTATE_ENABLE,
            16,
            TIM3_OCPOLARITY_HIGH
    );
    TIM3_Cmd(ENABLE);

    /**
     * TIM4
     * Generates the CLK signal for communicating.
     * Interrupt runs at 200 kHz, CLK therefore runs at 100kHz.
     * Only active during transmission.
     */
    TIM4_DeInit();
    TIM4_TimeBaseInit(TIM4_PRESCALER_1, 80);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

    GPIO_Init(GPIOD, SDO, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIOD, SCLK, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIOD, LE, GPIO_MODE_OUT_PP_LOW_SLOW);

    GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_LOW_SLOW);
}

void led1642_transmit(void) {
    finished_tx = FALSE;
    TIM4_Cmd(ENABLE);
    while (!finished_tx);
}

void led1642_set_brightness(const uint32_t brightness) {
    int idx;
    message->data = brightness;
    for (idx = 0; idx < BRIGHTNESS_REG_COUNT - 1; idx++) {
        message->size = L1642_TWO_BYTES;
        message->latch = BRIGHTNESS_LATCH;
        led1642_transmit();
    }
    message->size = L1642_TWO_BYTES;
    // Last brightness value must be sent with a different latch, see datasheet
    message->latch = BRIGHTNESS_GLOBAL_LATCH;
    led1642_transmit();
}

isr void led1642_isr(void) {
    if (message->size >= 0) {
        if (rising) {
            if ((message->data >> message->size) & 0x01) { sdo_on(); }
            if (message->latch > message->size) { le_on(); }
            message->size--;
            clock_on();
        } else {
            sdo_off();
            clock_off();
        }
        rising = !rising;
    } else {
        sdo_off();
        le_off();
        clock_off();
        TIM4_Cmd(DISABLE);
        finished_tx = TRUE;
        rising = TRUE;
    }

    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}
