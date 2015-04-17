#ifndef RGBConverter_h
#define RGBConverter_h

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

    File:   RGBConverter.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    RGBConverter.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    It is used in the RGB Node to calculate colors from HSL to RGB and
     back to RGB again.

    - Found on Github (see header below) and ported from Auduino.
    - Wrapped into a CPP class.
    
    James Stokebrand  - 2015 Mar 12
    jamesstokebrand AT gmail DOT com
 
*****************************************************/

#include <avr/io.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define SUPPORT_HSV_COLOR 0
#define SUPPORT_HSL_COLOR 1

class RgbColor
{
public:
    RgbColor() 
    {
        clear();
    }

    RgbColor(uint8_t const &R, uint8_t const &G, uint8_t const &B)
    : r(R)
    , g(G)
    , b(B)
    { }

    virtual ~RgbColor() {}

    void clear()
    {
        r=0;
        g=0;
        b=0;
    }

    void set(uint8_t const &R, uint8_t const &G, uint8_t const &B)
    { 
        r=R;
        g=G;
        b=B;
    }

    RgbColor& operator=(const RgbColor &rhs) {
        // Check for self-assignment!
        if (this == &rhs)      // Same object?
            return *this;        // Yes, so skip assignment, and just return *this.

        this->r = rhs.r;
        this->g = rhs.g;
        this->b = rhs.b;

        return *this;
    }

    uint8_t r;
    uint8_t g;
    uint8_t b;
};

#if SUPPORT_HSV_COLOR
class HsvColor
{
public:
    HsvColor()
    {
        clear();
    }

    HsvColor(uint8_t const &H, uint8_t const &S, uint8_t const &V)
    : h(H)
    , s(S)
    , v(V)
    { }

    virtual ~HsvColor() {}

    void clear()
    {
        h=0;
        s=0;
        v=0;
    }

    void set(uint8_t const &H, uint8_t const &S, uint8_t const &V)
    {
        h=H;
        s=S;
        v=V;
    }

    HsvColor& operator=(const HsvColor &rhs) {
        // Check for self-assignment!
        if (this == &rhs)      // Same object?
            return *this;        // Yes, so skip assignment, and just return *this.

        this->h = rhs.h;
        this->s = rhs.s;
        this->v = rhs.v;

        return *this;
    }

    uint8_t h;
    uint8_t s;
    uint8_t v;
}; 
#endif

#if SUPPORT_HSL_COLOR
class HslColor
{
public:
    HslColor()
    {
        clear();
    }

    HslColor(double const &H, double const &S, double const &L)
    : h(H)
    , s(S)
    , l(L)
    { }

    void set(double const &H, double const &S, double const &L)
    {
        h=H;
        s=S;
        l=L;
    }

    virtual ~HslColor() {}

    void clear()
    {
        h=0;
        s=0;
        l=0;
    }


    HslColor& operator=(const HslColor &rhs) {
        // Check for self-assignment!
        if (this == &rhs)      // Same object?
            return *this;        // Yes, so skip assignment, and just return *this.

        this->h = rhs.h;
        this->s = rhs.s;
        this->l = rhs.l;

        return *this;
    }

    double h;
    double s;
    double l;
};
#endif


class RGBConverter {

public:

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
    static void rgbToHsl(RgbColor const &A, HslColor &B);
    
    /**
     * Converts an HSL color value to RGB. Conversion formula
     * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
     * Assumes HslColor (h, s, and l) are contained in the set [0, 1] and
     * returns RgbColor (r, g, and b) in the set [0, 255].
     *
     * @param   HslColor const &A   The constant HSL color value
     * @param   RgbColor &B         The RGB color value
     */
    static void hslToRgb(HslColor const &A, RgbColor &B);
#endif

#if SUPPORT_HSV_COLOR
    /**
     * Converts an RGB color value to HSV. Conversion formula
     * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
     * Assumes RgbColor (r, g, and b) are contained in the set [0, 255] and
     * returns HsvColor (h, s, and v) in the set [0, 255].
     *
     * @param   RgbColor const &A   The constant RGB color value
     * @return  HsvColor &B         The HSV return value
     */
    static void rgbToHsv(RgbColor const &A, HsvColor &B);
    
    /**
     * Converts an HSV color value to RGB. Conversion formula
     * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
     * Assumes HsvColor (h, s, and v) are contained in the set [0, 255] and
     * returns RgbColor (r, g, and b) in the set [0, 255].
     *
     * @param   HsvColor const &A   The constant HSV representation
     * @return  RgbColor &B         The RGB color return value
     */
    static void hsvToRgb(HsvColor const &A, RgbColor &B);
#endif

private:
    static double threeway_max(double a, double b, double c);
    static double threeway_min(double a, double b, double c);
    static double hue2rgb(double p, double q, double t);
};

#endif

