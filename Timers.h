/*
 * Timers.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Cooper Brotherton
 */

#ifndef TIMERS_H_
#define TIMERS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

// NOTE: Because TA is 16-bit, a whole note is the longest valid duration
#define BEAT                                                11719
#define DEBOUNCE_DELAY                                      15000
/*!
 * \brief This function initializes the timers for the project
 *
 * This function configures the timers as follows:
 * WDT is stopped.
 * HFXT is set to 48MHz, MCLK to 12MHz, SMCLK to 3MHz.
 * TA0 in PWM mode using CCR0. Sources SMCLK with divider of 1
 *      Interrupts are disabled.
 * TA1 in updown mode using CCR0. Sources SMCLK with a divider of 64
 *      Interrupts are enabled.
 * TA2 in up mode using CCR1. Sources SMCLK with a divider of 1
 *      Interrupts are disabled.
 *
 * \return None
 */
extern void Timers_init(void);

/*!
 * \brief This function starts the timers
 *
 * This function starts TA0 in up mode, TA1 in updown mode.
 *
 * It allows the song to resume playing.
 *
 * \return None
 */
extern void Timers_start(void);

/*!
 * \brief This function stops the timers
 *
 * This function stops TA0 and TA1.
 *
 * It pauses the song.
 *
 * \return None
 */
extern void Timers_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMERS_H_ */
