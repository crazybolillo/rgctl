/**
 * Provides routines to control the red_lim, green_lim and blue_lim tones of LEDs. Since the three different LEDs
 * must be turned on separately each color is given a timeslot. The ratio of said timeslots determines
 * the color perceived by the human eye.
*/

#include <stm8s.h>
#include "rgctl.h"

isr void rgb_isr(void);

void rgb_init(uint8_t red, uint8_t green, uint8_t blue);
void rgb_start(void);
void rgb_stop(void);

void rgb_red(uint8_t val);
void rgb_green(uint8_t val);
void rgb_blue(uint8_t val);
