#include <stm8s.h>

volatile uint16_t sysTick = 0;

@interrupt void sysTickHandler() {
    sysTick++;
    TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
}

void setupHardware(void) {
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    /**
     * Clock output (CCO) on PD0
     * Output Frequency should be 16MHz (HSI with no prescaler)
     * PD0 must be set as PP output
     * Option bit AFR2 must be programmed for PD0 to work as CCO
     */
    FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    FLASH_ProgramOptionByte(0x4803, (1 << 2));
    GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
    CLK_CCOConfig(CLK_OUTPUT_CPU);
    CLK_CCOCmd(ENABLE);

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
     * TIM1 CH3
     * Used for debugging purposes. Should output a 1kHz signal representing the system tick
     */
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);
    TIM1_TimeBaseInit(16, TIM1_COUNTERMODE_UP, 1000, 0);
    TIM1_OC3Init(
        TIM1_OCMODE_PWM1,
        TIM1_OUTPUTSTATE_ENABLE,
        TIM1_OUTPUTNSTATE_DISABLE,
        500,
        TIM1_OCPOLARITY_HIGH,
        TIM1_OCNPOLARITY_LOW,
        TIM1_OCIDLESTATE_RESET,
        TIM1_OCNIDLESTATE_RESET
    );
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    TIM1_CtrlPWMOutputs(ENABLE);
    TIM1_Cmd(ENABLE);
}

int main(void) {
    setupHardware();
    _asm("rim");

	while(1);
}
