/*
 * Switch.h
 *
 *  Created on: Dec 18, 2020
 *  Edited on:  Jan 5, 2021
 *      Author: Cooper Brotherton
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define SWITCH_PORT                                                 GPIO_PORT_P1
#define SWITCH_PIN                                                  (0x0010)

/*!
 * \brief This function configures the switches as inputs
 *
 * This function configures P1.4 as an input pin with a pull-up resistor
 *
 * \return None
 */
extern void Switch_init(void);

/*!
 * \brief This function determines whether switch S2 is pressed
 *
 * This function gets the value at 2.4 and transforms it into a boolean to
 * convey whether or not the switch is pressed
 *
 * \return true if the switch is pressed, false otherwise
 */
extern bool Switch_pressed();

#ifdef __cplusplus
}
#endif

#endif /* SWITCH_H_ */
