//
// Created by anton on 4/21/24.
//

#include "ui.h"

enum {
    CLICK_PIN = GPIO_PIN_5,
    CLICK_OFF = 0,
    CLICK_ON = 0x20,
    ENCODER_OFF = 0,
    ENCODER_ON = 0x06,
    ENC_A_PIN = GPIO_PIN_1,
    ENC_B_PIN = GPIO_PIN_2
};

static volatile uint8_t state, rotation, timeout;

isr void encoder_isr(void) {
    if ((GPIO_ReadInputData(GPIOC) & 0x06) == 0x02) {
        rotation = UI_ENCODER_COUNTERCLOCK;
    } else {
        rotation = UI_ENCODER_CLOCKWISE;
    }
    GPIOC->CR2 = ENCODER_OFF;
}

isr void click_isr(void) {
    GPIOE->CR2 = CLICK_OFF;
    timeout = 1;
    TIM1_Cmd(DISABLE);
    switch (state) {
        case 0:
            state = UI_EVENT_RED_SEL;
            break;
        case UI_EVENT_RED_SEL:
            state = UI_EVENT_GREEN_SEL;
            break;
        case UI_EVENT_GREEN_SEL:
            state = UI_EVENT_BLUE_SEL;
            break;
        case UI_EVENT_BLUE_SEL:
            state = UI_EVENT_BRIGHTNESS_SEL;
            break;
        case UI_EVENT_BRIGHTNESS_SEL:
            state = UI_EVENT_RED_SEL;
            break;
        default:
            state = UI_EVENT_RED_SEL;
    }
}

isr void timeout_isr(void) {
    TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
    timeout = 1;
}

const volatile uint8_t *ui_init(void) {
    state = 0;
    rotation = 0;
    timeout = 0;

    GPIO_Init(GPIOC, ENC_A_PIN, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOC, ENC_B_PIN, GPIO_MODE_IN_PU_NO_IT);
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, EXTI_SENSITIVITY_FALL_ONLY);

    GPIO_Init(GPIOE, CLICK_PIN, GPIO_MODE_IN_PU_IT);
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_FALL_ONLY);

    // Approx. 3 seconds
    TIM1_TimeBaseInit(10000, TIM1_COUNTERMODE_UP, 4800, 0);
    TIM1_UpdateRequestConfig(TIM1_UPDATESOURCE_REGULAR);
    TIM1_SelectOnePulseMode(TIM1_OPMODE_SINGLE);
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);

    return &state;
}

void ui_enable_click(void) { GPIOE->CR2 = CLICK_ON; }
void ui_enable_encoder(void) { GPIOC->CR2 = ENCODER_ON; }

volatile uint8_t ui_read_encoder(void) {
    uint8_t tmp = rotation;
    rotation = 0;
    return tmp;
}

volatile uint8_t ui_timeout(void) { return timeout; }

void ui_reset_timeout(void) {
    TIM1_Cmd(DISABLE);
    TIM1_GenerateEvent(TIM1_EVENTSOURCE_UPDATE);
    TIM1->SR1 = 0;
    TIM1->SR2 = 0;
    TIM1_Cmd(ENABLE);
}

void ui_start_timeout(void) {
    timeout = 0;
    TIM1_GenerateEvent(TIM1_EVENTSOURCE_UPDATE);
    TIM1->SR1 = 0;
    TIM1->SR2 = 0;
    TIM1_Cmd(ENABLE);
}

void ui_stop_timeout(void) {
    timeout = 0;
    TIM1_Cmd(DISABLE);
}
