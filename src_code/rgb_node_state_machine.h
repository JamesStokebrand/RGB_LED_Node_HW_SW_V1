#ifndef _RGB_NODE_STATE_MACHINE_H_
#define _RGB_NODE_STATE_MACHINE_H_

/****************************************************
    RGB LED Node State Machine

    File:   rgb_node_state_machine.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    rgb_node_state_machine.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements the RGB LED Node State Machine.  This machine
     will accept commands from the RGB LED Controller to change the colors
     of the common anode RGB LED.

    Copyright (C) 2015 - James Stokebrand - 2015 Mar 12
    jamesstokebrand AT gmail DOT com

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
    2014 Oct 31  James Stokebrand   Initial creation.

*****************************************************/


#ifndef _STATE_CLASS_H_
#include "state_class.h"
#endif

#ifndef _COMM_CLASS_H_
#include "comm_class.h"
#endif

#ifndef _EVENT_QUEUE_H_
#include "event_queue.h"
#endif

#ifndef _PIN_CLASS_H_
#include "pin_class.h"
#endif

#define DEBUG 0

class rgb_node_state_machine
:public base_state_class
{
public:

    rgb_node_state_machine(EventQueue *event_queue)
    : base_state_class((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_INTENSITY)
    , _RGB_Led(IOPinDefines::E_PinDef::E_PIN_PC3
              ,IOPinDefines::E_PinDef::E_PIN_PC4
              ,IOPinDefines::E_PinDef::E_PIN_PC5
              // this "false" sets this to common anode RGB LED
              ,false)
    , _NODE_ADDRESS(0)
    , RGB_adjust_value(RGB_LARGE_ADJUST_VALUE)
    , RGB_color_adjust_temp(0)
    , HSL_adjust_value(HSL_LARGE_ADJUST_VALUE)
    , HSL_color_adjust_temp(0)
    , _event_queue(event_queue)
    {
        // Initial state of the RGB LED is OFF.
        _RGB_Led.HSL_Off();

        // Init in HSL mode.  Set Saturation to .99 for best color.
        _RGB_Led.setSaturation(0.99);

        // Init in HSL mode.  Set Intensity to 25%
        _RGB_Led.setIntensity(0.25);

        // Attach the UART object to start receiving events
        _Comm.Attach(event_queue);

        (this->*state)(ENTER_EVENT);

        // Read the DIP switch
        InputPinClass DipSwitch_01(IOPinDefines::E_PinDef::E_PIN_PD5);
        InputPinClass DipSwitch_02(IOPinDefines::E_PinDef::E_PIN_PD6);
        InputPinClass DipSwitch_03(IOPinDefines::E_PinDef::E_PIN_PD7);
        InputPinClass DipSwitch_04(IOPinDefines::E_PinDef::E_PIN_PB0);

        // Set the node address as appropriate
        _NODE_ADDRESS |= (DipSwitch_01.Read() << 0);
        _NODE_ADDRESS |= (DipSwitch_02.Read() << 1);
        _NODE_ADDRESS |= (DipSwitch_03.Read() << 2);
        _NODE_ADDRESS |= (DipSwitch_04.Read() << 3);

#if 0
// For debugging.  Send the node address over the comm link
        event_element_class _temp;
        _temp.set(RGB_NODE,(E_InputEvent)0x00,_NODE_ADDRESS);
        _Comm.encode(_temp);
#endif

        // Disable the node's status LED.
        mcu_sleep_class::getInstance()->DisableStatusLED();

    }

    virtual ~rgb_node_state_machine() {}

private:

    void STATE_ADJ_MODE_RED(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        // Is a msg we should process?
        if (!(act_on_this_msg(A.get_current_data()))) return;
        
        // process the msg
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Send RED PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
            }
        break;
        case E_RGB_CONTROLLER:
            switch(A.get_current_event())
            {
            case E_SET_RED:
                // Already in RED state ... do nothing.
                send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
            break;
            case E_SET_GREEN:
                // Green State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_GREEN);
            break;
            case E_SET_BLUE:
                // Blue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_BLUE);
            break;
            case E_SET_HUE:
                // Hue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_HUE);
            break;
            case E_SET_SATURATION:
                // Saturation State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_SATURATION);
            break;
            case E_SET_INTENSITY:
                // Intensity State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_INTENSITY);
            break;
            case E_RE_CW:
                // Rotary Encoder Counter Clockwise turn the LED Up
                RGB_color_adjust_temp = _RGB_Led.getRed() + RGB_adjust_value;

                // Out of bounds?
                if (RGB_color_adjust_temp > 255) RGB_color_adjust_temp = 255;
                _RGB_Led.setRed((uint8_t)RGB_color_adjust_temp);

                // Send RED PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
            break;
            case E_RE_CCW:
                // Rotary Encoder Counter Clockwise turn the LED Down
                RGB_color_adjust_temp = _RGB_Led.getRed() - RGB_adjust_value;

                // Out of bounds?
                if (RGB_color_adjust_temp < 0) RGB_color_adjust_temp = 0;
                _RGB_Led.setRed((uint8_t)RGB_color_adjust_temp);

                // Send RED PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
            break;
            case E_RE_PRESSED:
                // Set adjust value to small
               RGB_adjust_value = RGB_SMALL_ADJUST_VALUE;
            break;
            case E_RE_RELEASED:
                // Set adjust value to large
                RGB_adjust_value = RGB_LARGE_ADJUST_VALUE;
            break;
            case E_ONLY_RED:
                {
                    // Set color to 50% red.
                    RgbColor _color_temp(128,_RGB_Led.getGreen(),_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send RED PWM value feedback.
                    send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
                }
            break;
            case E_ONLY_GREEN:
                {
                    // Set color to 50% green.
                    RgbColor _color_temp(_RGB_Led.getRed(),128,_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);
                }
            break;
            case E_ONLY_BLUE:
                {
                    // Set color to 50% blue.
                    RgbColor _color_temp(_RGB_Led.getRed(),_RGB_Led.getGreen(),128);
                    _RGB_Led.set(_color_temp);
                }
            break;
            case E_ALL_OFF:
                _RGB_Led.RGB_Off();

                // Send RED PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
            break;
            case E_ALL_HALF:
                {
                    // Set color to 50% blue.
                    RgbColor _color_temp(128,128,128);
                    _RGB_Led.set(_color_temp);

                    // Send RED PWM value feedback.
                    send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
                }
            break;
            case E_ALL_ON:
                _RGB_Led.RGB_On();

                // Send RED PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
            break;
            case E_SELECT:
                Blink(A.get_current_data());
            break;
            case E_FORCE_FEEDBACK:
                // Send RED PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_RED_PWM, _RGB_Led.getRed());
            break;
            case E_ENABLE_STATUS_LED:
                // Enable the node's status LED.
                mcu_sleep_class::getInstance()->EnableStatusLED();
            break;
            case E_DISABLE_STATUS_LED:
                // Disable the node's status LED.
                mcu_sleep_class::getInstance()->DisableStatusLED();
            break;
            default:
            break;
            }
        break;
        default:
        break;
        }
    }

    void STATE_ADJ_MODE_GREEN(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        // Is a msg we should process?
        if (!(act_on_this_msg(A.get_current_data()))) return;

        // process the msg
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Send GREEN PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
            }
        break;
        case E_RGB_CONTROLLER:
            switch(A.get_current_event())
            {
            case E_SET_RED:
                // Red State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_RED);
            break;
            case E_SET_GREEN:
                // Already in Green state ... do nothing.
                send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
            break;
            case E_SET_BLUE:
                // Blue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_BLUE);
            break;
            case E_SET_HUE:
                // Hue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_HUE);
            break;
            case E_SET_SATURATION:
                // Saturation State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_SATURATION);
            break;
            case E_SET_INTENSITY:
                // Intensity State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_INTENSITY);
            break;
            case E_RE_CW:
                // Rotary Encoder Counter Clockwise turn the LED Up
                RGB_color_adjust_temp = _RGB_Led.getGreen() + RGB_adjust_value;

                // Out of bounds?
                if (RGB_color_adjust_temp > 255) RGB_color_adjust_temp = 255;
                _RGB_Led.setGreen((uint8_t)RGB_color_adjust_temp);

                // Send GREEN PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
            break;
            case E_RE_CCW:
                // Rotary Encoder Counter Clockwise turn the LED Down
                RGB_color_adjust_temp = _RGB_Led.getGreen() - RGB_adjust_value;

                // Out of bounds?
                if (RGB_color_adjust_temp < 0) RGB_color_adjust_temp = 0;
                _RGB_Led.setGreen((uint8_t)RGB_color_adjust_temp);

                // Send GREEN PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
            break;
            case E_RE_PRESSED:
                // Set adjust value to small
                RGB_adjust_value = RGB_SMALL_ADJUST_VALUE;
            break;
            case E_RE_RELEASED:
                // Set adjust value to large
                RGB_adjust_value = RGB_LARGE_ADJUST_VALUE;
            break;
            case E_ONLY_RED:
                {
                    // Set color to 50% red.
                    RgbColor _color_temp(128,_RGB_Led.getGreen(),_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);
                }
            break;
            case E_ONLY_GREEN:
                {
                    // Set color to 50% green.
                    RgbColor _color_temp(_RGB_Led.getRed(),128,_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send Green PWM value feedback.
                    send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
                }
            break;
            case E_ONLY_BLUE:
                {
                    // Set color to 50% Blue.
                    RgbColor _color_temp(_RGB_Led.getRed(),_RGB_Led.getGreen(),128);
                    _RGB_Led.set(_color_temp);
                }
            break;
            case E_ALL_OFF:
                _RGB_Led.RGB_Off();

                // Send GREEN PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
            break;
            case E_ALL_HALF:
                {
                    // Set color to 50% blue.
                    RgbColor _color_temp(128,128,128);
                    _RGB_Led.set(_color_temp);

                    // Send GREEN PWM value feedback.
                    send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
                }
            break;
            case E_ALL_ON:
                _RGB_Led.RGB_On();

                // Send GREEN PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
            break;
            case E_SELECT:
                Blink(A.get_current_data());
            break;
            case E_FORCE_FEEDBACK:
                // Send GREEN PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_GREEN_PWM, _RGB_Led.getGreen());
            break;
            case E_ENABLE_STATUS_LED:
                // Enable the node's status LED.
                mcu_sleep_class::getInstance()->EnableStatusLED();
            break;
            case E_DISABLE_STATUS_LED:
                // Disable the node's status LED.
                mcu_sleep_class::getInstance()->DisableStatusLED();
            break;
            default:
            break;
            }
        break;
        default:
        break;
        }
    }

    void STATE_ADJ_MODE_BLUE(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        // Is a msg we should process?
        if (!(act_on_this_msg(A.get_current_data()))) return;

        // process the msg
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Send BLUE PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
            }
        break;
        case E_RGB_CONTROLLER:
            switch(A.get_current_event())
            {
            case E_SET_RED:
                // Red State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_RED);
            break;
            case E_SET_GREEN:
                // Green State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_GREEN);
            break;
            case E_SET_BLUE:
                // Already in BLUE State ... do nothing
                send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
            break;
            case E_SET_HUE:
                // Hue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_HUE);
            break;
            case E_SET_SATURATION:
                // Saturation State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_SATURATION);
            break;
            case E_SET_INTENSITY:
                // Intensity State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_INTENSITY);
            break;
            case E_RE_CW:
                // Rotary Encoder Counter Clockwise turn the LED Up
                RGB_color_adjust_temp = _RGB_Led.getBlue() + RGB_adjust_value;

                // Out of bounds?
                if (RGB_color_adjust_temp > 255) RGB_color_adjust_temp = 255;
                _RGB_Led.setBlue((uint8_t)RGB_color_adjust_temp);

                // Send BLUE PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
            break;
            case E_RE_CCW:
                // Rotary Encoder Counter Clockwise turn the LED Down
                RGB_color_adjust_temp = _RGB_Led.getBlue() - RGB_adjust_value;

                // Out of bounds?
                if (RGB_color_adjust_temp < 0) RGB_color_adjust_temp = 0;
                _RGB_Led.setBlue((uint8_t)RGB_color_adjust_temp);

                // Send BLUE PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
            break;
            case E_RE_PRESSED:
                // Set adjust value to small
                RGB_adjust_value = RGB_SMALL_ADJUST_VALUE;
            break;
            case E_RE_RELEASED:
                // Set adjust value to large
                RGB_adjust_value = RGB_LARGE_ADJUST_VALUE;
            break;
            case E_ONLY_RED:
                {
                    // Set color to 50% red.
                    RgbColor _color_temp(128,_RGB_Led.getGreen(),_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);
                }
            break;
            case E_ONLY_GREEN:
                {
                    // Set color to 50% green.
                    RgbColor _color_temp(_RGB_Led.getRed(),128,_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);
                }
            break;
            case E_ONLY_BLUE:
                {
                    // Set color to 50% Blue.
                    RgbColor _color_temp(_RGB_Led.getRed(),_RGB_Led.getGreen(),128);
                    _RGB_Led.set(_color_temp);

                    // Send BLUE PWM value feedback.
                    send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
                }
            break;
            case E_ALL_OFF:
                _RGB_Led.RGB_Off();

                // Send BLUE PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
            break;
            case E_ALL_HALF:
                {
                    // Set color to 50% red.
                    RgbColor _color_temp(128,128,128);
                    _RGB_Led.set(_color_temp);

                    // Send BLUE PWM value feedback.
                    send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
                }
            break;
            case E_ALL_ON:
                _RGB_Led.RGB_On();

                // Send BLUE PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
            break;
            case E_SELECT:
                Blink(A.get_current_data());
            break;
            case E_FORCE_FEEDBACK:
                // Send BLUE PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_BLUE_PWM, _RGB_Led.getBlue());
            break;
            case E_ENABLE_STATUS_LED:
                // Enable the node's status LED.
                mcu_sleep_class::getInstance()->EnableStatusLED();
            break;
            case E_DISABLE_STATUS_LED:
                // Disable the node's status LED.
                mcu_sleep_class::getInstance()->DisableStatusLED();
            break;
            default:
            break;
            }
        break;
        default:
        break;
        }
    }


    void STATE_ADJ_MODE_HUE(event_element_class &A)
    {

#if DEBUG
_Comm.encode(A);
#endif
        // Is a msg we should process?
        if (!(act_on_this_msg(A.get_current_data()))) return;
        
        // process the msg
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Send HUE value feedback.
                send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
            }
        break;
        case E_RGB_CONTROLLER:
            switch(A.get_current_event())
            {
            case E_SET_RED:
                // Red State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_RED);
            break;
            case E_SET_GREEN:
                // Green State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_GREEN);
            break;
            case E_SET_BLUE:
                // Blue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_BLUE);
            break;
            case E_SET_HUE:
                // Already in Hue State. Do nothing.
                send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
            break;
            case E_SET_SATURATION:
                // Saturation State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_SATURATION);
            break;
            case E_SET_INTENSITY:
                // Intensity State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_INTENSITY);
            break;
            case E_RE_CW:
                // Rotary Encoder Counter Clockwise turn the LED Up
                HSL_color_adjust_temp = _RGB_Led.getHue() + HSL_adjust_value;

                // Out of bounds?
                if (HSL_color_adjust_temp > 0.99) HSL_color_adjust_temp -= 1;
                _RGB_Led.setHue(HSL_color_adjust_temp);

                // Send HUE value feedback.
                send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
            break;
            case E_RE_CCW:
                // Rotary Encoder Counter Clockwise turn the LED Down
                HSL_color_adjust_temp = _RGB_Led.getHue() - HSL_adjust_value;

                // Adjust this to rollover!
                if (HSL_color_adjust_temp < 0.01) HSL_color_adjust_temp += 1;
                _RGB_Led.setHue(HSL_color_adjust_temp);

                // Send RED PWM value feedback.
                send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
            break;
            case E_RE_PRESSED:
                // Set adjust value to small
                HSL_adjust_value = HSL_SMALL_ADJUST_VALUE;
            break;
            case E_RE_RELEASED:
                // Set adjust value to large
                HSL_adjust_value = HSL_LARGE_ADJUST_VALUE;
            break;
            case E_ONLY_RED:
                {
                    // Set color to 50% red.
                    RgbColor _color_temp(128,_RGB_Led.getGreen(),_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send HUE value feedback.
                    send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
                }
            break;
            case E_ONLY_GREEN:
                {
                    // Set color to 50% green.
                    RgbColor _color_temp(_RGB_Led.getRed(),128,_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send HUE value feedback.
                    send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
                }
            break;
            case E_ONLY_BLUE:
                {
                    // Set color to 50% blue.
                    RgbColor _color_temp(_RGB_Led.getRed(),_RGB_Led.getGreen(),128);
                    _RGB_Led.set(_color_temp);

                    // Send HUE value feedback.
                    send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
                }
            break;
            case E_ALL_OFF:
                {
                    // Set color to 0(Zero) Intensity
                    _RGB_Led.setIntensity(0);

                    // Send HUE value feedback.
                    send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
                }
            break;
            case E_ALL_HALF:
                {
                    // Set color to 50% Intensity
                    _RGB_Led.setIntensity(0.5);

                    // Send HUE value feedback.
                    send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
                }
            break;
            case E_ALL_ON:
                {
                    // Set color to 100% Intensity
                    _RGB_Led.setIntensity(0.99);

                    // Send HUE value feedback.
                    send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
                }
            break;
            case E_SELECT:
                Blink(A.get_current_data());
            break;
            case E_FORCE_FEEDBACK:
                // Send HUE value feedback.
                send_feedback(A.get_current_data(), E_LED_HUE_PWM, _RGB_Led.getHue()*255);
            break;
            case E_ENABLE_STATUS_LED:
                // Enable the node's status LED.
                mcu_sleep_class::getInstance()->EnableStatusLED();
            break;
            case E_DISABLE_STATUS_LED:
                // Disable the node's status LED.
                mcu_sleep_class::getInstance()->DisableStatusLED();
            break;
            default:
            break;
            }
        break;
        default:
        break;
        }
    }


    void STATE_ADJ_MODE_SATURATION(event_element_class &A)
    {

#if DEBUG
_Comm.encode(A);
#endif
        // Is a msg we should process?
        if (!(act_on_this_msg(A.get_current_data()))) return;
        
        // process the msg
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Send Saturation value feedback.
                send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
            }
        break;
        case E_RGB_CONTROLLER:
            switch(A.get_current_event())
            {
            case E_SET_RED:
                // Red State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_RED);
            break;
            case E_SET_GREEN:
                // Green State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_GREEN);
            break;
            case E_SET_BLUE:
                // Blue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_BLUE);
            break;
            case E_SET_HUE:
                // Hue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_HUE);
            break;
            case E_SET_SATURATION:
                // Already in Saturation State. Do nothing.
                send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
            break;
            case E_SET_INTENSITY:
                // Intensity State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_INTENSITY);
            break;
            case E_RE_CW:
                // Rotary Encoder Counter Clockwise turn the LED Up
                HSL_color_adjust_temp = _RGB_Led.getSaturation() + HSL_adjust_value;

                // Out of bounds?
                if (HSL_color_adjust_temp > 0.99) HSL_color_adjust_temp = 0.99;
                _RGB_Led.setSaturation(HSL_color_adjust_temp);

                // Send Saturation value feedback.
                send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
            break;
            case E_RE_CCW:
                // Rotary Encoder Counter Clockwise turn the LED Down
                HSL_color_adjust_temp = _RGB_Led.getSaturation() - HSL_adjust_value;

                // Out of bounds?
                if (HSL_color_adjust_temp < 0.01) HSL_color_adjust_temp = 0.01;
                _RGB_Led.setSaturation(HSL_color_adjust_temp);

                // Send Saturation value feedback.
                send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
            break;
            case E_RE_PRESSED:
                // Set adjust value to small
                HSL_adjust_value = HSL_SMALL_ADJUST_VALUE;
            break;
            case E_RE_RELEASED:
                // Set adjust value to large
                HSL_adjust_value = HSL_LARGE_ADJUST_VALUE;
            break;
            case E_ONLY_RED:
                {
                    // Set color to 50% red.
                    RgbColor _color_temp(128,_RGB_Led.getGreen(),_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send Saturation value feedback
                    send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
                }
            break;
            case E_ONLY_GREEN:
                {
                    // Set color to 50% green.
                    RgbColor _color_temp(_RGB_Led.getRed(),128,_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send Saturation value feedback.
                    send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
                }
            break;
            case E_ONLY_BLUE:
                {
                    // Set color to 50% blue.
                    RgbColor _color_temp(_RGB_Led.getRed(),_RGB_Led.getGreen(),128);
                    _RGB_Led.set(_color_temp);

                    // Send Saturation value feedback.
                    send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
                }
            break;
            case E_ALL_OFF:
                {
                    // Set color to 0(Zero) Intensity
                    _RGB_Led.setIntensity(0);

                    // Send Saturation value feedback.
                    send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
                }
            break;
            case E_ALL_HALF:
                {
                    // Set color to 50% Intensity
                    _RGB_Led.setIntensity(0.5);

                    // Send Saturation value feedback.
                    send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
                }
            break;
            case E_ALL_ON:
                {
                    // Set color to 100% Intensity
                    _RGB_Led.setIntensity(0.99);

                    // Send Saturation value feedback.
                    send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
                }
            break;
            case E_SELECT:
                Blink(A.get_current_data());
            break;
            case E_FORCE_FEEDBACK:
                // Send Saturation value feedback.
                send_feedback(A.get_current_data(), E_LED_SATURATION_PWM, _RGB_Led.getSaturation()*255);
            break;
            case E_ENABLE_STATUS_LED:
                // Enable the node's status LED.
                mcu_sleep_class::getInstance()->EnableStatusLED();
            break;
            case E_DISABLE_STATUS_LED:
                // Disable the node's status LED.
                mcu_sleep_class::getInstance()->DisableStatusLED();
            break;
            default:
            break;
            }
        break;
        default:
        break;
        }
    }


    void STATE_ADJ_MODE_INTENSITY(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        // Is a msg we should process?
        if (!(act_on_this_msg(A.get_current_data()))) return;
        
        // process the msg
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Send Intensity value feedback.
                send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
            }
        case E_RGB_CONTROLLER:
            switch(A.get_current_event())
            {
            case E_SET_RED:
                // Red State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_RED);
            break;
            case E_SET_GREEN:
                // Green State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_GREEN);
            break;
            case E_SET_BLUE:
                // Blue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_BLUE);
            break;
            case E_SET_HUE:
                // Hue State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_HUE);
            break;
            case E_SET_SATURATION:
                // Saturation State requested ... transition
                TRAN((STATE)&rgb_node_state_machine::STATE_ADJ_MODE_SATURATION);
            break;
            case E_SET_INTENSITY:
                // Already in Intensity State. Do nothing.
                send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
            break;
            case E_RE_CW:
                // Rotary Encoder Counter Clockwise turn the LED Up
                HSL_color_adjust_temp = _RGB_Led.getIntensity() + HSL_adjust_value;

                // Out of bounds?
                if (HSL_color_adjust_temp > 0.99) HSL_color_adjust_temp = 0.99;
                _RGB_Led.setIntensity(HSL_color_adjust_temp);

                // Send Intensity value feedback.
                send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
            break;
            case E_RE_CCW:
                // Rotary Encoder Counter Clockwise turn the LED Down
                HSL_color_adjust_temp = _RGB_Led.getIntensity() - HSL_adjust_value;

                // Out of bounds?
                if (HSL_color_adjust_temp < 0.0) HSL_color_adjust_temp = 0.0;
                _RGB_Led.setIntensity(HSL_color_adjust_temp);

                // Send Intensity value feedback.
                send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
            break;
            case E_RE_PRESSED:
                // Set adjust value to small
                HSL_adjust_value = HSL_SMALL_ADJUST_VALUE;
            break;
            case E_RE_RELEASED:
                // Set adjust value to large
                HSL_adjust_value = HSL_LARGE_ADJUST_VALUE;
            break;
            case E_ONLY_RED:
                {
                    // Set color to 50% red.
                    RgbColor _color_temp(128,_RGB_Led.getGreen(),_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send Intensity value feedback.
                    send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
                }
            break;
            case E_ONLY_GREEN:
                {
                    // Set color to 50% green.
                    RgbColor _color_temp(_RGB_Led.getRed(),128,_RGB_Led.getBlue());
                    _RGB_Led.set(_color_temp);

                    // Send Intensity value feedback.
                    send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
                }
            break;
            case E_ONLY_BLUE:
                {
                    // Set color to 50% blue.
                    RgbColor _color_temp(_RGB_Led.getRed(),_RGB_Led.getGreen(),128);
                    _RGB_Led.set(_color_temp);

                    // Send Intensity value feedback.
                    send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
                }
            break;
            case E_ALL_OFF:
                {
                    // Set color to 0(Zero) Intensity
                    _RGB_Led.setIntensity(0);

                    // Send Intensity value feedback.
                    send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, 0);
                }
            break;
            case E_ALL_HALF:
                {
                    // Set color to 50% Intensity
                    _RGB_Led.setIntensity(0.5);

                    // Send Intensity value feedback.
                    send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, 128);
                }
            break;
            case E_ALL_ON:
                {
                    // Set color to 100% Intensity
                    _RGB_Led.setIntensity(0.99);

                    // Send Intensity value feedback.
                    send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, 255);
                }
            break;
            case E_SELECT:
                Blink(A.get_current_data());
            break;
            case E_FORCE_FEEDBACK:
                // Send Intensity value feedback.
                send_feedback(A.get_current_data(), E_LED_INTENSITY_PWM, _RGB_Led.getIntensity()*255);
            break;
            case E_ENABLE_STATUS_LED:
                // Enable the node's status LED.
                mcu_sleep_class::getInstance()->EnableStatusLED();
            break;
            case E_DISABLE_STATUS_LED:
                // Disable the node's status LED.
                mcu_sleep_class::getInstance()->DisableStatusLED();
            break;
            default:
            break;
            }
        break;
        default:
        break;
        }
    }


    void send_feedback(uint8_t const &address,E_InputEvent const &event, uint8_t const &pwm_value)
    {
        event_element_class _temp;

        // If msg address is ZERO and _NODE_ADDRESS is ONE
        // OR IF msg address is NOT ZERO and msg address == _NODE_ADDRESS 
        // THEN send a deedback msg
        if (((address == 0) && (_NODE_ADDRESS == 1)) || 
            ((address != 0) && (address == _NODE_ADDRESS)))
        {
            // Assemble the msg
            _temp.set(E_RGB_NODE,event,pwm_value);
            // Send via comm
            _Comm.encode(_temp);
        }
    }

    void Blink(uint8_t const &address)
    {
        // If msg address is ZERO and _NODE_ADDRESS is ONE
        // OR IF msg address is NOT ZERO and msg address == _NODE_ADDRESS 
        // THEN go ahead and blink.
        if (((address == 0) && (_NODE_ADDRESS == 1)) ||
            ((address != 0) && (address == _NODE_ADDRESS)))
        {
            _RGB_Led.Blink();
        }
    }

    bool act_on_this_msg(uint8_t const &address)
    {
        // IF this msg address is ZERO 
        // OR IF msg address equals the NODE ADDRESS
        // THEN process it.
        if ((address == 0) || (address == _NODE_ADDRESS))
        {
            return true;
        }
        return false;
    }

    // Comm Class
    comm_class _Comm;

    // RGB LED class
    rgb_led_class _RGB_Led;

    // Node address is the address read from the DIP switches
    uint8_t _NODE_ADDRESS;

    // Adjust value is the amount to adjust the LED value
    uint8_t RGB_adjust_value;
    static const uint8_t RGB_LARGE_ADJUST_VALUE = 10;
    static const uint8_t RGB_SMALL_ADJUST_VALUE = 1;
    int16_t RGB_color_adjust_temp;

    double HSL_adjust_value;
    static constexpr double HSL_LARGE_ADJUST_VALUE = 0.04;
    static constexpr double HSL_SMALL_ADJUST_VALUE = 0.004;
    double HSL_color_adjust_temp;

    EventQueue *_event_queue;

};


#endif

