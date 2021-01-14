/*
 * Timers.c
 *
 *  Created on: Jan 13, 2021
 *      Author: Cooper Brotherton
 */

/* DriverLib Includes */
#include <Timers.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

extern void Timers_init(void)
{
    MAP_WDT_A_holdTimer();

    // Configuring pins for peripheral/crystal usage and LED for output
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_PJ, GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setExternalClockSourceFrequency(32000, 48000000);
    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false);

    // Initializing MCLK and SMCLK with HFXT
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_16);

    // Timer_A0 PWM config for speaker, initialized to off
    const Timer_A_PWMConfig compareConfig_PWM = {
    TIMER_A_CLOCKSOURCE_SMCLK,                          // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_1,              // SMCLK/1 = 3MHz
            0,                                          // Tick period
            TIMER_A_CAPTURECOMPARE_REGISTER_0,          // Use CCR0
            TIMER_A_OUTPUTMODE_TOGGLE,                  // Toggle output bit
            0                                           // Duty Cycle
            };

    // Timer_A1 updown config
    const Timer_A_UpDownModeConfig upDownConfig = {
    TIMER_A_CLOCKSOURCE_SMCLK,                          // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_64,             // SMCLK/64 = 31.25kHz
            BEAT,                                      // 480 millisecond period
            TIMER_A_TAIE_INTERRUPT_ENABLE,             // Enable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,         // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                            // Clear value
            };

    // Timer_A2 up config
    const Timer_A_UpModeConfig upConfig = {
    TIMER_A_CLOCKSOURCE_ACLK,                           // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_1,              // SMCLK/1 = 3MHz
            DEBOUNCE_DELAY,                             // 5ms Tick period
            TIMER_A_TAIE_INTERRUPT_DISABLE,           // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,       // Disable CCR0 interrupt
            TIMER_A_DO_CLEAR                            // Clear value
            };

    const Timer_A_CompareModeConfig compareConfig_SET = {
    TIMER_A_CAPTURECOMPARE_REGISTER_1,                  // Use CCR1
            TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
            TIMER_A_OUTPUTMODE_SET,                     // Set output bit
            50 };

    // Set up timers with configs
    MAP_Timer_A_generatePWM(TIMER_A0_BASE, &compareConfig_PWM);
    MAP_Timer_A_configureUpDownMode(TIMER_A1_BASE, &upDownConfig);
    MAP_Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig);
    MAP_Timer_A_initCompare(TIMER_A2_BASE, &compareConfig_SET);

    MAP_Interrupt_enableInterrupt(INT_TA1_0);
    MAP_Interrupt_enableInterrupt(INT_TA1_N);
}

extern void Timers_start(void)
{
    MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UPDOWN_MODE);
}

extern void Timers_stop(void)
{
    MAP_Timer_A_stopTimer(TIMER_A0_BASE);
    MAP_Timer_A_stopTimer(TIMER_A1_BASE);
}
