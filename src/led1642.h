#ifndef RGCTL_LED1642_H
#define RGCTL_LED1642_H

#include <stm8s.h>
#include "rgctl.h"

#define L1642_WR_SW_LATCH 2
#define L1642_WR_CR_LATCH 7

#define L1642_TWO_BYTES 15
#define L1642_ALL_ONES 0xFFFF

struct Message {
    uint16_t data;
    int8_t size;
    uint16_t latch;
};

void led1642_init(struct Message *message);
void led1642_transmit(void);
void led1642_set_brightness(uint32_t brightness);
void led1642_isr(void);


#endif //RGCTL_LED1642_H
