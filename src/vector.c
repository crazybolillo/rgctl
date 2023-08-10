extern void _stext();
extern void sysTickHandler();

#define NULL 0
#pragma section const {vector}

void (* const @vector vector_table[32])() = {
    _stext,			// RESET
    NULL,			// TRAP
    NULL,			// TLI
    NULL,			// AWU
    NULL,			// CLK
    NULL,			// EXTI PORTA
    NULL,			// EXTI PORTB
    NULL,			// EXTI PORTC
    NULL,			// EXTI PORTD
    NULL,			// EXTI PORTE
    NULL,			// RESERVED
    NULL,			// RESERVED
    NULL,			// SPI EOF
    sysTickHandler,	// TIMER 1 OVF
    NULL,			// TIMER 1 CAP
    NULL,			// TIMER 2 OVF
    NULL,			// TIMER 2 CAP
    NULL,			// TIMER 3 OVF
    NULL,			// TIMER 3 CAP
    NULL,			// RESERVED
    NULL,			// RESERVED
    NULL,			// I2C
    NULL,			// UART TX
    NULL,			// UART RX
    NULL,			// ADC
    NULL,			// TIMER 4 OVF
    NULL,			// EEPROM ECC
    NULL,			// Reserved
    NULL,			// Reserved
    NULL,			// Reserved
    NULL,			// Reserved
    NULL,			// Reserved
};
