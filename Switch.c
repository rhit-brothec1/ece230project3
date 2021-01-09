/*
 * Switch.c
 *
 *  Created on: Dec 18, 2020
 *  Edited on:  Jan 5, 2021
 *      Author: Cooper Brotherton
 */
/* DriverLib Includes */
#include <Switch.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

void Switch_init(void)
{
    // set S2 pin as input using GPIO driver functions
    GPIO_setAsInputPinWithPullUpResistor(SWITCH_PORT, SWITCH_PIN);
}

extern bool Switch_pressed()
{
    return GPIO_getInputPinValue(SWITCH_PORT,
    SWITCH_PIN) == GPIO_INPUT_PIN_HIGH ? false : true;
}
