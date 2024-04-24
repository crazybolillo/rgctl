#include <stm8s.h>

#include "rgctl.h"

extern void _stext();
extern void led1642_isr();
extern void rgb_isr();
extern void click_isr();
extern void encoder_isr();
extern void timeout_isr();

void trapHandler() {
    GPIO_WriteHigh(GPIOC, GPIO_PIN_7);
    while (1)
        ;
}

#define NULL 0
#pragma section const {vector }

void (*const @vector vector_table[32])() = {
    _stext,       // RESET
    trapHandler,  // TRAP
    NULL,         // TLI
    NULL,         // AWU
    NULL,         // CLK
    NULL,         // EXTI PORTA
    NULL,         // EXTI PORTB
    encoder_isr,  // EXTI PORTC
    NULL,         // EXTI PORTD
    click_isr,    // EXTI PORTE
    NULL,         // RESERVED
    NULL,         // RESERVED
    NULL,         // SPI EOF
    timeout_isr,  // TIMER 1 OVF
    NULL,         // TIMER 1 CAP
    rgb_isr,      // TIMER 2 OVF
    NULL,         // TIMER 2 CAP
    NULL,         // TIMER 3 OVF
    NULL,         // TIMER 3 CAP
    NULL,         // RESERVED
    NULL,         // RESERVED
    NULL,         // I2C
    NULL,         // UART TX
    NULL,         // UART RX
    NULL,         // ADC
    led1642_isr,  // TIMER 4 OVF
    NULL,         // EEPROM ECC
    NULL,         // Reserved
    NULL,         // Reserved
    NULL,         // Reserved
    NULL,         // Reserved
    NULL,         // Reserved
};
