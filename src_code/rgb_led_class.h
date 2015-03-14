#ifndef _RGB_LED_CLASS_H_
#define _RGB_LED_CLASS_H_

/****************************************************
    RGB LED Class

    File:   rgb_led_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com
	
    rgb_led_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements an interface for the common anode RGB LED devices
     that will be similar to a normal LED (IE On/Off/Toggle and more)
    - Three common anode LEDs wrapped by PWM classes.
    - Supports RGB and HSL

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
    2014 Oct 15  James Stokebrand   Initial creation.

*****************************************************/

#ifndef _PWM_CLASS_H_
#include "pwm_class.h"
#endif

#ifndef RGBConverter_h
#include "RGBConverter.h"
#endif

class rgb_led_class
: public RGBConverter
{
public:
    rgb_led_class(
              IOPinDefines::E_PinDef const &R
            , IOPinDefines::E_PinDef const &G
            , IOPinDefines::E_PinDef const &B
            , bool const &CommonCathode = true)
    : red_led(R,CommonCathode,0)
    , green_led(G,CommonCathode,0)
    , blue_led(B,CommonCathode,0)
    , HSL_color_valid(false)
    { 
        // Scale values for adjusting RGB LED color
        red_scale_value = 1.0;
        green_scale_value = 0.3;
        blue_scale_value = 1.0;

        // Clear the current color values.
        RGB_currentColor.clear();
        HSL_currentColor.clear();
    }

    void set(RgbColor const &A);
    void set(HsvColor const &A);
    void set(HslColor const &A);

    void get(RgbColor &A);
    void get(HsvColor &A);
    void get(HslColor &A);

    void RGB_On() 
    {
        // Set the current color to max
        RGB_currentColor.r = RGB_currentColor.g = RGB_currentColor.b = 255;

        // Use SET instead of On() for each LED 
        //  in case of the use of color scaling.
        set(RGB_currentColor);
    }

    void RGB_Off()
    {
        // Remember the current color
        RGB_currentColor.clear();

        // Use SET instead of On() for each LED 
        //  in case of the use of color scaling.
        set(RGB_currentColor);

        
    }

    void HSL_On()
    {
        if (!HSL_color_valid)
        {
            // Read current HSL color
            get(HSL_currentColor);
            HSL_color_valid = true;
        }

        // Set Intensity(luminosity) to ZERO
        HSL_currentColor.l = 0;
        set(HSL_currentColor);
    }

    void HSL_Off()
    {
        if (!HSL_color_valid)
        {
            // Read current HSL color
            get(HSL_currentColor);
            HSL_color_valid = true;
        }

        // Set Intensity(luminosity) to ZERO
        HSL_currentColor.l = 0.99;
        set(HSL_currentColor);
    }

    void Toggle()
    {
        red_led.Toggle();
        green_led.Toggle();
        blue_led.Toggle();

        RGB_currentColor.r = red_led.getValue();
        RGB_currentColor.g = green_led.getValue();
        RGB_currentColor.b = blue_led.getValue();
    }

    inline uint8_t getRed()
    {
        return RGB_currentColor.r;
    }

    void setRed(uint8_t const &A)
    {
        RGB_currentColor.r = A;
        set(RGB_currentColor);
    }

    inline uint8_t getGreen()
    {
        return RGB_currentColor.g;
    }

    void setGreen(uint8_t const &A)
    {
        RGB_currentColor.g = A;
        set(RGB_currentColor);
    }
    
    inline uint8_t getBlue()
    {
        return RGB_currentColor.b;
    }

    void setBlue(uint8_t const &A)
    {
        RGB_currentColor.b = A;
        set(RGB_currentColor);
    }

    double getHue()
    {
        if (!HSL_color_valid)
        {
            // Read current HSL color
            get(HSL_currentColor);
            HSL_color_valid = true;
        }
        return HSL_currentColor.h;
    }

    void setHue(double const &A)
    {
        if (!HSL_color_valid)
        {
            // Read current HSL color
            get(HSL_currentColor);
            HSL_color_valid = true;
        }

        // Set Hue
        HSL_currentColor.h = A;
        set(HSL_currentColor);
    }

    double getSaturation()
    {
        if (!HSL_color_valid) {
            // Read current HSL color
            get(HSL_currentColor);
            HSL_color_valid = true;
        }
        return HSL_currentColor.s;
    }

    void setSaturation(double const &A)
    {   
        if (!HSL_color_valid) {
            // Read current HSL color
            get(HSL_currentColor);
            HSL_color_valid = true;
        }

        // Set Saturation
        HSL_currentColor.s = A;
        set(HSL_currentColor);
    }

    double getIntensity()
    {
        HslColor A;
        get(A);
        return A.l;
    }   

    void setIntensity(double const &A)
    {
        if (!HSL_color_valid) {
            // Read current HSL color
            get(HSL_currentColor);
            HSL_color_valid = true;
        }

        // Set Intensity
        HSL_currentColor.l = A;
        set(HSL_currentColor);
    }

    void Blink()
    {
        for(uint8_t A=0; A<255; A+=51)
        {
            red_led.setValue(A);
            green_led.setValue(A);
            blue_led.setValue(A);
        }

        for(uint8_t B=255; B>0; B-=51)
        {
            red_led.setValue(B);
            green_led.setValue(B);
            blue_led.setValue(B);
        }

        red_led.setValue(RGB_currentColor.r);
        green_led.setValue(RGB_currentColor.g);
        blue_led.setValue(RGB_currentColor.b);
    }


private:
    pwm_class red_led;
    pwm_class green_led;
    pwm_class blue_led;

    // Use these values to adjust the RGB values
    //  for color correctness.
    float red_scale_value;
    float green_scale_value;
    float blue_scale_value;

    RgbColor RGB_currentColor;

    bool HSL_color_valid;
    HslColor HSL_currentColor;
};

#endif
