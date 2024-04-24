#ifndef RGCTL_UI_H
#define RGCTL_UI_H

#include "rgctl.h"

#define UI_EVENT_RED_SEL 1
#define UI_EVENT_GREEN_SEL 2
#define UI_EVENT_BLUE_SEL 3
#define UI_EVENT_BRIGHTNESS_SEL 4

#define UI_ENCODER_CLOCKWISE 1
#define UI_ENCODER_COUNTERCLOCK 2

const volatile uint8_t *ui_init(void);
volatile uint8_t ui_read_encoder(void);
volatile uint8_t ui_timeout(void);

void ui_enable_click(void);
void ui_enable_encoder(void);

void ui_reset_timeout(void);
void ui_start_timeout(void);
void ui_stop_timeout(void);

#endif  // RGCTL_UI_H
