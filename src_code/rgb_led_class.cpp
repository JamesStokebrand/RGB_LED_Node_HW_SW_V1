/****************************************************
    RGB LED Class

    File:   rgb_led_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com
	
    rgb_led_class.cpp file is part of the RGB LED Controller and Node 
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

#ifndef _RGB_LED_CLASS_H_
#include "rgb_led_class.h"
#endif


void rgb_led_class::set(RgbColor const &A)
{
    // Scale and set the RGB value
    red_led.setValue(A.r*red_scale_value);
    green_led.setValue(A.g*green_scale_value);
    blue_led.setValue(A.b*blue_scale_value);

    RGB_currentColor.r = A.r;
    RGB_currentColor.g = A.g;
    RGB_currentColor.b = A.b;

    HSL_color_valid = false;
}

#if SUPPORT_HSV_COLOR
void rgb_led_class::set(HsvColor const &A)
{
    RgbColor B;
    hsvToRgb(A,B);
    set(B);
}
#endif

void rgb_led_class::set(HslColor const &A)
{
    HSL_currentColor = A;
    RgbColor B;
    hslToRgb(A,B);
    set(B);

    // Must set HSL_color_valid to TRUE here.  
    //  The set(RgbColor) will set this to false 
    //  so we must reset it here after the set(RgbColor) 
    //  call.
    HSL_color_valid = true;
}

void rgb_led_class::get(RgbColor &A)
{
    A.r = RGB_currentColor.r;
    A.g = RGB_currentColor.g;
    A.b = RGB_currentColor.b;
}

#if SUPPORT_HSV_COLOR
void rgb_led_class::get(HsvColor &A)
{
    rgbToHsv(RGB_currentColor,A);
}
#endif

void rgb_led_class::get(HslColor &A)
{
    if (HSL_color_valid) {
        A = HSL_currentColor;
        return;
    }
    rgbToHsl(RGB_currentColor,A);
}



