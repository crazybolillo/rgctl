#ifndef RGCTL_RGCTL_H
#define RGCTL_RGCTL_H

#define FCLK 16000000

/**
 * For some reason Cosmic decided to use @ for some of its keywords, it also decided to use custom keywords where
 * standard ones already existed (inline). This breaks code insights on places where they are used. Macro magic
 * is used to replace these and only expand them to the value the compiler expects during compilation.
 */
#ifndef TYPING
#define isr
#define _inline inline
#else
#define isr @interrupt
#define _inline @inline
#endif

#define us_cycles(x) ((uint16_t)(((x * (FCLK / 1000000UL)) - 3) / 3))
_inline void delay_cycles(uint16_t cycles);
_inline void delay_us(uint16_t micro);

#endif //RGCTL_RGCTL_H
