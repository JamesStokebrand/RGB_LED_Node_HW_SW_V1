/*
 * RGBConverter.h - Arduino library for converting between RGB, HSV and HSL
 * 
 * Ported from the Javascript at http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript
 * The hard work was Michael's, all the bugs are mine.
 *
 * Robert Atkins, December 2010 (ratkins_at_fastmail_dot_fm).
 *
 * https://github.com/ratkins/RGBConverter
 *
 */

/****************************************************

    RGB Converter Class

    File:   RGBConverter.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    RGBConverter.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    It is used in the RGB Node to calculate colors from HSL to RGB and
     back to RGB again.

    - Found on Github (see header below) and ported from Auduino.
    - Wrapped into a CPP class.
    
    James Stokebrand  - 2015 Mar 12
    jamesstokebrand AT gmail DOT com
 
*****************************************************/

#include "RGBConverter.h"

#if SUPPORT_HSL_COLOR
/**
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes RgbColor (r, g, and b) are contained in the set [0, 255] and
 * returns HslColor (h, s, and l) in the set [0, 1].
 *
 * @param   RgbColor const &A   The constant RGB color value
 * @param   HslColor &B         The HSL color value
 */
void RGBConverter::rgbToHsl(RgbColor const &A, HslColor &B) {
    double rd = (double) A.r/255;
    double gd = (double) A.g/255;
    double bd = (double) A.b/255;
    double max = threeway_max(rd, gd, bd);
    double min = threeway_min(rd, gd, bd);
    double h=0, s=0, l = (max + min) / 2;

    if (max == min) {
        h = s = 0; // achromatic
    } else {
        double d = max - min;
        s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
        if (max == rd) {
            h = (gd - bd) / d + (gd < bd ? 6 : 0);
        } else if (max == gd) {
            h = (bd - rd) / d + 2;
        } else if (max == bd) {
            h = (rd - gd) / d + 4;
        }
        h /= 6;
    }
    B.h = h;
    B.s = s;
    B.l = l;
}

double RGBConverter::hue2rgb(double p, double q, double t) {

    if (t < 0) t += 1;
    if (t > 1.0) t -= 1;
    if (t < 1.0 / 6.0)
    {
        return p + (q - p) * 6.0 * t;
    }
    if (t < 1.0 / 2.0)
    {
        return q;
    }
    if (t < 2.0 / 3.0)
    {
        return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
    }
    return p;
}

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes RgbColor (r, g, and b) are contained in the set [0, 255] and
 * returns HslColor (h, s, and l) in the set [0, 1].
 *
 * @param   HslColor const &B   The constant HSL color value
 * @param   RgbColor &A   The RGB color return value
 */
void RGBConverter::hslToRgb(HslColor const &A, RgbColor &B) {
    double r, g, b;
    double H = A.h;

  if (A.s == 0) {
    r = g = b = A.l; // achromatic
  } else {
    double q = A.l < 0.5 ? A.l * (1.0 + A.s) : A.l + A.s - A.l * A.s;
    double p = 2 * A.l - q;
    r = hue2rgb(p, q, (H + 1.0 / 3.0));
    g = hue2rgb(p, q, H);
    b = hue2rgb(p, q, (H - 1.0 / 3.0));
  }

  B.r = r * 255;
  B.g = g * 255;
  B.b = b * 255;
}
#endif

#if SUPPORT_HSV_COLOR
/**
 * Converts an RGB color value to HSV. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes RgbColor (r, g, and b) are contained in the set [0, 255] and
 * returns HsvColor (h, s, and v) in the set [0, 255]
 *
 * @param   RgbColor const &A   The constant RGB color value
 * @return  HsvColor &B         The HSV representation
 */
void RGBConverter::rgbToHsv(RgbColor const &A, HsvColor &B) {
    double rd = (double) A.r/255;
    double gd = (double) A.g/255;
    double bd = (double) A.b/255;
    double max = threeway_max(rd, gd, bd), min = threeway_min(rd, gd, bd);
    double h=0, s=0, v = max;

    double d = max - min;
    s = max == 0 ? 0 : d / max;

    if (max == min) { 
        h = 0; // achromatic
    } else {
        if (max == rd) {
            h = (gd - bd) / d + (gd < bd ? 6 : 0);
        } else if (max == gd) {
            h = (bd - rd) / d + 2;
        } else if (max == bd) {
            h = (rd - gd) / d + 4;
        }
        h /= 6;
    }

    B.h = h * 255;
    B.s = s * 255;
    B.v = v * 255;
}

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes HsvColor (h, s, and v) are contained in the set [0, 255] and
 * returns RgbColor (r, g, and b) in the set [0, 255].
 *
 * @param   HsvColor const &A   The constant HSV representation
 * @return  RgbColor &B         The RGB color return value
 */
void RGBConverter::hsvToRgb(HsvColor const &A, RgbColor &B) {
    double r=0, g=0, b=0;
    double H=0, S=0, V=0;

    H = ((double)A.h)*255.0f;
    S = ((double)A.s)*255.0f;
    V = ((double)A.v)*255.0f;


    uint8_t i = uint8_t(H * 6);
    double f = H * 6 - i;
    double p = V * (1 - S);
    double q = V * (1 - f * S);
    double t = V * (1 - (1 - f) * S);

    switch(i % 6){
        case 0: r = V, g = t, b = p; break;
        case 1: r = q, g = V, b = p; break;
        case 2: r = p, g = V, b = t; break;
        case 3: r = p, g = q, b = V; break;
        case 4: r = t, g = p, b = V; break;
        case 5: r = V, g = p, b = q; break;
    }

    B.r = r * 255;
    B.g = g * 255;
    B.b = b * 255;
}
#endif
 
double RGBConverter::threeway_max(double a, double b, double c) {
    return max(a, max(b, c));
}

double RGBConverter::threeway_min(double a, double b, double c) {
    return min(a, min(b, c));
}

