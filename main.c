/*
 * Author:      Cooper Brotherton
 * Date:        January 14, 2021
 * Libraries:   GPIO and Timer A from DriverLib
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
 *       S2-->|P1.4          P2.6|--> TA0CCR0
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
#include "Timers.h"

int noteIndex = 0;

#define NOTECNT 28

const uint16_t noteHalfPeriod[NOTECNT] = { C4, F4, F4, F4, F4, A4, C5, A4, F4,
                                           G4, G4, G4, E4, D4, C4, C4, E4, E4,
                                           E4, E4, A4, C5, A4, F4, G4, C4, C4,
                                           F4 };

const double noteBeat[NOTECNT] = { 1, 1, 0.5, 0.5, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
                                   1, 1, 1, 0.5, 0.5, 1, 1, 1, 1, 2, 2, 1.5, 1,
                                   4 };

//![Simple PMAP Config]
/* Port mapper configuration register */
const uint8_t port_mapping[] = {
//Port P2:
        PMAP_NONE,
        PMAP_NONE,
        PMAP_NONE, PMAP_NONE, PMAP_NONE, PMAP_NONE,
        PMAP_TA0CCR0A,
        PMAP_NONE };

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

    Timers_init();

    MAP_Interrupt_enableMaster();

    // Remapping  TA0CCR0 to P2.6
    MAP_PMAP_configurePorts((const uint8_t*) port_mapping, PMAP_P2MAP, 1,
    PMAP_DISABLE_RECONFIGURATION);

    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P2,
            GPIO_PIN6,
            GPIO_PRIMARY_MODULE_FUNCTION);
}

/*!
 * \brief This function debounces switch pressed
 *
 * This function loads a count-down value in TA2 CCR1 for a delay.
 * Delay is 5ms
 *
 * \return None
 */
void debounce()
{
    MAP_Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
    while ((TIMER_A2->CCTL[1]) & (1 << 2) == 0)
        ;
}

/*!
 * \brief This function controls I/O and playing of the song
 *
 * This function waits for an S2 press and release, then starts playing
 * the song. On the next S2 press, it will stop playing. Once S2 is
 * released, it will loop.
 *
 * \return None
 */
void loop()
{
    while (!Switch_pressed())
        ;
    debounce();
    while (Switch_pressed())
        ;
    debounce();
    Timers_start();
    while (!Switch_pressed())
        ;
    Timers_stop();
    debounce();
    while (Switch_pressed())
        ;
    debounce();
}

int main(void)
{
    setup();

    while (1)
    {
        loop();
    }
}

/*!
 * \brief This function handles the interrupt of TA1 CCR0
 *
 * This function turns on the speaker with the appropriate note.
 *
 *\return None
 */
void TA1_0_IRQHandler(void)
{
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0);
    MAP_Timer_A_setCompareValue(TIMER_A0_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
                                noteHalfPeriod[noteIndex]);
}

/*!
 * \brief This function handles the interrupt of TA1 CCRN
 *
 * This function turns off the speaker, loads the next beat into CCR0,
 * and increments to the next note
 */
void TA1_N_IRQHandler(void)
{
    MAP_Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    MAP_Timer_A_setCompareValue(TIMER_A0_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
                                0);
    noteIndex = (noteIndex + 1) % NOTECNT;
    MAP_Timer_A_setCompareValue(TIMER_A1_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0,
                                noteBeat[noteIndex] * BEAT);
}
