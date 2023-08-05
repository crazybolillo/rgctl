#include <stdint.h>
#include <iostm8s105.h>

#define CPU_FREQ 2000000

void delay_ms(uint32_t ms) {
    uint32_t count;
    for (count = 0; count < ((CPU_FREQ / 18 / 1000) * ms); count++) {
        _asm("nop");
    }
}

int main(void) {
    PB_ODR = 0x00;
    PB_DDR = 0x01;
    PB_CR1 = 0x01;
    PB_CR2 = 0x00;

	while(1) {
		PB_ODR = 0x01;
        delay_ms(1000);
        PB_ODR = 0x00;
        delay_ms(1000);
	}
}
