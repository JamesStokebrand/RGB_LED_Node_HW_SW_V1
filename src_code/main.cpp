/****************************************************
    RGB LED Node v1 hardware

    File:   main.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com
	
    main.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file contains the main() method and the forever loop.  It 
     creates the RGB Node State machine and the code to sleep the AVR
     chip.
     
    Copyright (C) 2015 - James Stokebrand - 2015 Mar 12

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Nov 18  James Stokebrand   Initial creation.

*****************************************************/

/*****************************************************

// This is the ATmega328p fuse settings for the project

AVR Chip Fuses:

--- FUSES ---
BODLEVEL    = DISABLED
RSTDISBL    = [ ]
DWEN        = [ ]
SPIEN       = [X]
WDTON       = [ ]
EESAVE      = [ ]
BOOTSZ      = 2048W_3800
BOOTRST     = [ ]
CKDIV8      = [ ]
CKOUT       = [ ]
SUT_CKSEL   = INTRCOSC_8MHZ_6CK_14CK_0MS

EXTENDED    = 0xFF (valid)
HIGH        = 0xD9 (valid)
LOW         = 0xC2 (valid)

--- LOCK BITS ---
LB      = NO_LOCK
BLB0    = NO_LOCK
BLB1    = NO_LOCK

LOCKBIT = 0xFF (valid)

*****************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef NEW_H
#include "new.h"
#endif

#ifndef RGBConverter_h
#include "RGBConverter.h"
#endif

#ifndef _RGB_LED_CLASS_H_
#include "rgb_led_class.h"
#endif

#ifndef _EVENT_QUEUE_H_
#include "event_queue.h"
#endif

#ifndef _RGB_NODE_STATE_MACHINE_H_
#include "rgb_node_state_machine.h"
#endif

#ifndef _MCU_SLEEP_CLASS_H_
#include "mcu_sleep_class.h"
#endif


int main(void)
{
    // Enable MCU sleep
    mcu_sleep_class::getInstance()->EnableSleep();

    // Idle is the default power mode ... but set it anyway.
    mcu_sleep_class::getInstance()->SetSleepMode(mcu_sleep_class::E_MCU_SLEEP_MODE_IDLE);

    // Enable the status LED to show when the device
    //  is awake
    mcu_sleep_class::getInstance()->EnableStatusLED();

    // Try to save more power.  Set these pins as input and enable pullup resistor
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PD3);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PD4);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PB6);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PB7);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PC2);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PC1);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PC0);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PB2);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PB1);

    // Event queue 
    EventQueue event_queue;

    // Init the Node state machine
    rgb_node_state_machine RGB_Node(&event_queue);

    // RGB Node class has read the 4 pin DIP.  The pins are no longer needed so set 
    // them as input and enable pullup resistor
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PD5);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PD6);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PD7);
    mcu_sleep_class::getInstance()->SetInputAndPullupResistor(IOPinDefines::E_PinDef::E_PIN_PB0);

    sei();

    event_element_class anEvent;

    // Forever is a long time.
    for (;;) 
    {
        if (event_queue.Dequeue(anEvent))
        {
            // Process events throught the RGB Node state machine
            RGB_Node.process(anEvent);
        } else {

            // Nothing in the queue ... go to sleep
            mcu_sleep_class::getInstance()->GoMakeSleepNow();
        }
    }

    return 0;
}


