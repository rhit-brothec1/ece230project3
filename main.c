/*
 * Author:      Cooper Brotherton
 * Date:        January 5, 2021
 * Libraries:   SysTick and GPIO from DriverLib
 */
/******************************************************************************
 * MSP432 Project 3 ECE230 Winter 2020-2021
 *
 * Description: Plays a song. Play/pause is toggled by S2.
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *       S2-->|P1.4          P2.4|--> CCR0
 *            |                  |
 *            |                  |
 *            |                  |
 *******************************************************************************/
/* DriverLib Includes */
#include <music.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#include "Switch.h"
#include "Music.h"

int noteIndex = -1;

#define NOTECNT 48
const uint16_t noteHalfPeriod[NOTECNT] = { NOTEG3, NOTEG3, NOTED4, NOTED4,
NOTEE4,
                                           NOTEE4, NOTED4, REST,
                                           NOTEC4,
                                           NOTEC4, NOTEB3, NOTEB3, NOTEA3,
                                           NOTEA3,
                                           NOTEG3, REST,
                                           NOTED4,
                                           NOTED4, NOTEC4, NOTEC4, NOTEB3,
                                           NOTEB3,
                                           NOTEA3, REST,
                                           NOTED4,
                                           NOTED4, NOTEC4, NOTEC4, NOTEB3,
                                           NOTEB3,
                                           NOTEA3, REST,
                                           NOTEG3,
                                           NOTEG3, NOTED4, NOTED4, NOTEE4,
                                           NOTEE4,
                                           NOTED4, REST,
                                           NOTEC4,
                                           NOTEC4, NOTEB3, NOTEB3, NOTEA3,
                                           NOTEA3,
                                           NOTEG3, REST };

//![Simple PMAP Config]
/* Port mapper configuration register */
const uint8_t port_mapping[] = {
//Port P2:
        PMAP_NONE,
        PMAP_NONE,
        PMAP_NONE, PMAP_NONE, PMAP_TA0CCR0A, PMAP_NONE,
        PMAP_NONE,
        PMAP_NONE };

///* Timer_A Up Configuration Parameter */
const Timer_A_UpModeConfig upConfig = {
TIMER_A_CLOCKSOURCE_SMCLK,                          // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_3,              // SMCLK/3 = 12MHz
        80000,                                     // Tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,             // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,        // Disable CCR0 interrupt
        TIMER_A_DO_CLEAR                            // Clear value
        };

// Timer_A0 PWM config for speaker, initialized to off
const Timer_A_PWMConfig compareConfig_PWM = {
TIMER_A_CLOCKSOURCE_SMCLK,                          // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_3,              // SMCLK/3 = 12MHz
        0,                                          // Tick period
        TIMER_A_CAPTURECOMPARE_REGISTER_0,          // Use CCR0
        TIMER_A_OUTPUTMODE_TOGGLE,                  // Toggle output bit
        0                                           // Duty Cycle
        };

const Timer_A_CompareModeConfig compareConfig_SET = {
TIMER_A_CAPTURECOMPARE_REGISTER_1,                  // Use CCR1
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_SET,                     // Set output bit
        50 };

/*!
 * \brief This function sets up the device
 *
 * This function sets P2.4 as an output, P1.4 as an input,
 * sets up the SysTick timer, and MCLK and SMCLK clocks
 *
 * \return None
 */
void setup()
{
    Switch_init();

    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(3000000);

    // Remapping  TACCR0 to P2.4
    MAP_PMAP_configurePorts((const uint8_t*) port_mapping, PMAP_P2MAP, 1,
    PMAP_DISABLE_RECONFIGURATION);

    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P2,
            GPIO_PIN4,
            GPIO_PRIMARY_MODULE_FUNCTION);

    // Configuring pins for peripheral/crystal usage and LED for output
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2,
            GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setExternalClockSourceFrequency(32000, 48000000);
    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false);

    // Initializing MCLK and SMCLK to HFXT (effectively 12MHz)
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);

    // Initialize compare registers to generate PWM1
//    MAP_Timer_A_initCompare(TIMER_A0_BASE, &compareConfig_PWM1);
    MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_SET);

    // Configuring Timer_A0 for PWM Mode
//    MAP_Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig);
    MAP_Timer_A_generatePWM(TIMER_A0_BASE, &compareConfig_PWM);
    MAP_Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);
}

void play()
{
    MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    while (1)
    {

//        MAP_SysTick_setPeriod(3000000);
        // Play note for 250ms
        MAP_Timer_A_setCompareValue(TIMER_A0_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
                                    noteHalfPeriod[noteIndex]);
        while (((SysTick->CTRL) & SysTick_CTRL_COUNTFLAG_Msk) == 0)
        {
            // Check for S2 press
            if (Switch_pressed())
            {
                // Turn off speaker
                MAP_Timer_A_setCompareValue(TIMER_A0_BASE,
                TIMER_A_CAPTURECOMPARE_REGISTER_0,
                                            0);
                return;
            }
        }
        // Turn off speaker
        MAP_Timer_A_setCompareValue(TIMER_A0_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
                                    0);
//        MAP_SysTick_setPeriod(1 * 3000000);
        while (((SysTick->CTRL) & SysTick_CTRL_COUNTFLAG_Msk) == 0)
        {
            // Check for S2 press
            if (Switch_pressed())
            {
                // Turn off speaker
//                MAP_Timer_A_setCompareValue(TIMER_A0_BASE,
//                TIMER_A_CAPTURECOMPARE_REGISTER_0,
//                                            0);
                return;
            }
        }

        noteIndex = (noteIndex + 1) % NOTECNT;
    }
}

/*!
 * \brief This function debounces switch pressed
 *
 * This function loads a count-down value for a delay.
 * Delay is 5ms
 *
 * \return None
 */
void debounce()
{
    MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    // Wait for 5ms debounce
    while ((TIMER_A1->CCTL[1]) & (1 << 2) == 0)
        ;
}

void loop()
{
// Wait for S2 press and release
    while (!Switch_pressed())
        ;
    debounce();
    while (Switch_pressed())
        ;
    debounce();
    play();
// Wait until S2 is released before looping
    while (Switch_pressed())
        ;
}

int main(void)
{
    MAP_WDT_A_holdTimer();

    setup();

    while (1)
    {
        loop();
    }
}
