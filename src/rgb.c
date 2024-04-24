#include "rgb.h"

enum { RED_GPIO = GPIO_PIN_1, GREEN_GPIO = GPIO_PIN_2, BLUE_GPIO = GPIO_PIN_3 };

enum STATE { RED, GREEN, BLUE };

volatile static enum STATE state = RED;
volatile static uint8_t red, red_lim, green, green_lim, blue, blue_lim;

_inline void rgb_red_on(void) {
    GPIO_WriteHigh(GPIOB, GREEN_GPIO);
    GPIO_WriteHigh(GPIOB, BLUE_GPIO);

    GPIO_WriteLow(GPIOB, RED_GPIO);
}

_inline void rgb_green_on(void) {
    GPIO_WriteHigh(GPIOB, RED_GPIO);
    GPIO_WriteHigh(GPIOB, BLUE_GPIO);

    GPIO_WriteLow(GPIOB, GREEN_GPIO);
}

_inline void rgb_blue_on(void) {
    GPIO_WriteHigh(GPIOB, RED_GPIO);
    GPIO_WriteHigh(GPIOB, GREEN_GPIO);

    GPIO_WriteLow(GPIOB, BLUE_GPIO);
}

isr void rgb_isr(void) {
    switch (state) {
        case RED:
            red++;
            if (red >= red_lim) {
                red = 0;
                state = GREEN;
                if (green_lim != 0) { rgb_green_on(); }
            }
            break;
        case GREEN:
            green++;
            if (green >= green_lim) {
                green = 0;
                state = BLUE;
                if (blue_lim != 0) { rgb_blue_on(); }
            }
            break;
        case BLUE:
            blue++;
            if (blue >= blue_lim) {
                blue = 0;
                state = RED;
                if (red_lim != 0) { rgb_red_on(); }
            }
            break;
    }

    TIM2_ClearITPendingBit(TIM2_IT_UPDATE);
}

void rgb_init(uint8_t r, uint8_t g, uint8_t b) {
    red = green = blue = 0;
    red_lim = r;
    green_lim = g;
    blue_lim = b;

    GPIO_Init(GPIOB, RED_GPIO, GPIO_MODE_OUT_PP_HIGH_SLOW);
    GPIO_Init(GPIOB, GREEN_GPIO, GPIO_MODE_OUT_PP_HIGH_SLOW);
    GPIO_Init(GPIOB, BLUE_GPIO, GPIO_MODE_OUT_PP_HIGH_SLOW);

    TIM2_TimeBaseInit(TIM2_PRESCALER_1, 120);
    TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
}

void rgb_start(void) {
    red = green = blue = 0;
    state = RED;
    if (red_lim != 0) { rgb_red_on(); }

    TIM2_Cmd(ENABLE);
}

void rgb_set(uint8_t r, uint8_t g, uint8_t b) {
    red_lim = r;
    green_lim = g;
    blue_lim = b;
}

void rgb_off(void) {
    TIM2_Cmd(DISABLE);
    GPIO_WriteHigh(GPIOB, RED_GPIO | GREEN_GPIO | BLUE_GPIO);
}

uint8_t rgb_read_red() { return red_lim; }
uint8_t rgb_read_green() { return green_lim; }
uint8_t rgb_read_blue() { return blue_lim; }
