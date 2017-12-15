//************************************************************************
//		     Copyright (C)  2004
//     University Corporation for Atmospheric Research
//		     All Rights Reserved
//************************************************************************/
//
//	File:		TFInterpolator.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2004
//
//	Description:	Implements the TFInterpolator class:
//		A class to interpolate transfer function values
//		Currently only supports linear interpolation
//
#include <cmath>
#include <cassert>
#include <iostream>
#include <vapor/TFInterpolator.h>
using namespace std;
using namespace VAPoR;

#ifdef WIN32
    #ifndef NAN
static const unsigned long __nan[2] = {0xffffffff, 0x7fffffff};
        #define NAN (*(const float *)__nan)
    #endif
#endif

float TFInterpolator::Xn = 95.047;
float TFInterpolator::Yn = 100.0;
float TFInterpolator::Zn = 108.883;

float TFInterpolator::XYZtransferMatrix[9] = {0.4124564, 0.2126729, 0.0193339, 0.3575761, 0.7151522, 0.1191920, 0.1804375, 0.0721750, 0.9503041};

float TFInterpolator::XYZinverseMatrix[9] = {3.2404548360214087,   -0.9692663898756537, 0.05564341960421365, -1.5371388501025751, 1.876010928842491,
                                             -0.20402585426769812, -0.4985315468684809, 0.04155608234667354, 1.0572251624579287};

float *TFInterpolator::colorMap = NULL;

TFInterpolator::~TFInterpolator()
{
    if (colorMap) delete colorMap;
}

// Determine the interpolated value at intermediate value 0<=r<=1
// where the value at left and right endpoint is known
// This method is just a stand-in until we get more sophistication
//
float TFInterpolator::interpolate(TFInterpolator::type t, float leftVal, float rightVal, float r)
{
    if (t == TFInterpolator::discrete) {
        if (r < 0.5)
            return leftVal;
        else
            return rightVal;
    }
    float val = (float)(leftVal * (1. - r) + r * rightVal);
    // if (val < 0.f || val > 1.f){
    // assert(val <= 1.f && val >= 0.f);
    //}
    return val;
}

float TFInterpolator::interpCirc(type t, float leftVal, float rightVal, float r)
{
    if (t == TFInterpolator::discrete) {
        if (r < 0.5)
            return leftVal;
        else
            return rightVal;
    }
    if (fabs(rightVal - leftVal) <= 0.5f) return interpolate(t, leftVal, rightVal, r);
    // replace smaller by 1+smaller, interpolate, then fit back into interval
    //
    float interpVal;
    if (leftVal <= rightVal) {
        interpVal = interpolate(t, leftVal + 1.f, rightVal, r);
    } else
        interpVal = interpolate(t, leftVal, rightVal + 1.f, r);

    if (interpVal >= 1.f) interpVal -= 1.f;
    if (interpVal < 0.f || interpVal > 1.f) { assert(interpVal <= 1.f && interpVal >= 0.f); }
    return interpVal;
}

float *TFInterpolator::genDivergentMap(float rgb1[3], float rgb2[3], int numColors)
{
    float interp;
    float color[3];

    float *colorMap1 = new float[numColors * 3];

    for (float i = 0; i < (float)numColors; i++) {
        interp = (float)i / (float)numColors;
        divergentInterpolation(rgb1, rgb2, color, interp);
        colorMap1[(int)i * 3] = color[0];
        colorMap1[(int)i * 3 + 1] = color[1];
        colorMap1[(int)i * 3 + 2] = color[2];
    }

    return colorMap1;
}

void TFInterpolator::correctiveDivergentInterpolation(float rgb1[3], float rgb2[3], float output[3], float interp) { divergentInterpolation(rgb1, rgb2, output, interp, true); }

int TFInterpolator::divergentInterpolation(float rgb1[3], float rgb2[3], float output[3], float interp, bool corrective)
{
    interp = 1 - interp;
    static float msh1[3], msh2[3], mshMid[3];
    float        m1, s1, h1;
    float        m2, s2, h2;

    srgb2msh(rgb1, msh1);
    m1 = msh1[0];
    s1 = msh1[1];
    h1 = msh1[2];

    srgb2msh(rgb2, msh2);
    m2 = msh2[0];
    s2 = msh2[1];
    h2 = msh2[2];

    // If pohsv[1] are saturated and distinct, place white in the middle
    if (corrective) {
        if ((s1 > 0.05) && (s2 > .05) && (fabs(h1 - h2) > (3.1415 / 3.0))) {
            float Mmid;
            if ((m1 > m2) && (m1 > 88.f))
                Mmid = m1;
            else if ((m2 > m1) && (m2 > 88.f))
                Mmid = m2;
            else
                Mmid = 88.f;
            if (interp < 0.5) {
                m2 = Mmid;
                s2 = 0.0;
                h2 = 0.0;
                interp = 2 * interp;
            } else {
                m1 = Mmid;
                s1 = 0.0;
                h1 = 0.0;
                interp = 2 * interp - 1;
            }
        }
    }

    // Adjust hue of unsaturated colors
    if ((s1 < 0.05) && (s2 > 0.05))
        h1 = adjustHue(m2, s2, h2, m1);
    else if ((s2 < 0.05) && (s1 > 0.05))
        h2 = adjustHue(m1, s1, h1, m2);

    mshMid[0] = (1 - interp) * m1 + interp * m2;
    mshMid[1] = (1 - interp) * s1 + interp * s2;
    mshMid[2] = (1 - interp) * h1 + interp * h2;

    msh2srgb(mshMid, output);

    // Clip colors that are out of bounds
    float maxVal = output[0];
    if (maxVal < output[1]) maxVal = output[1];
    if (maxVal < output[2]) maxVal = output[2];
    if (maxVal > 255.0) {
        output[0] = output[0] * 255.0 / maxVal;
        output[1] = output[1] * 255.0 / maxVal;
        output[2] = output[2] * 255.0 / maxVal;
    }
    if (output[0] < 0) output[0] = 0.0;
    if (output[1] < 0) output[1] = 0.0;
    if (output[2] < 0) output[2] = 0.0;

    return 0;
}

float TFInterpolator::adjustHue(float m, float s, float h, float mUnsat)
{
    if (m >= mUnsat - .1)
        return h;    // mUnsat;
    else {
        float hSpin;
        hSpin = s * sqrt(pow(mUnsat, 2.f) - pow(m, 2.f)) / (m * sin(s));
        if (h > (-3.14159 / 3))
            return h + hSpin;
        else
            return h - hSpin;
    }
}

int TFInterpolator::msh2srgb(float msh[3], float rgb[3])
{
    static float lab[3];
    msh2lab(msh, lab);
    lab2srgb(lab, rgb);
    return 0;
}

int TFInterpolator::msh2lab(float msh[3], float lab[3])
{
    float l, a, b;
    float m = msh[0];
    float s = msh[1];
    float h = msh[2];

    l = m * cos(s);
    a = m * sin(s) * cos(h);
    b = m * sin(s) * sin(h);
    lab[0] = l;
    lab[1] = a;
    lab[2] = b;
    return 0;
}

float lab2srgbHelper(float val)
{
    float xlim = 0.008856;
    float a = 7.787;
    float b = 16.f / 116.f;
    float ylim = a * xlim + b;
    if (val > ylim) {
        return pow(val, 3);
    } else {
        return (val - b) / a;
    }
}

void LabToXYZ(double L, double a, double b, double *x, double *y, double *z)
{
    // LAB to XYZ
    double var_Y = (L + 16) / 116;
    double var_X = a / 500 + var_Y;
    double var_Z = var_Y - b / 200;

    if (pow(var_Y, 3) > 0.008856)
        var_Y = pow(var_Y, 3);
    else
        var_Y = (var_Y - 16.0 / 116.0) / 7.787;

    if (pow(var_X, 3) > 0.008856)
        var_X = pow(var_X, 3);
    else
        var_X = (var_X - 16.0 / 116.0) / 7.787;

    if (pow(var_Z, 3) > 0.008856)
        var_Z = pow(var_Z, 3);
    else
        var_Z = (var_Z - 16.0 / 116.0) / 7.787;
    const double ref_X = 0.9505;
    const double ref_Y = 1.000;
    const double ref_Z = 1.089;
    *x = ref_X * var_X;    // ref_X = 0.9505  Observer= 2 deg Illuminant= D65
    *y = ref_Y * var_Y;    // ref_Y = 1.000
    *z = ref_Z * var_Z;    // ref_Z = 1.089
}

int TFInterpolator::lab2srgb(float lab[3], float rgb[3])
{
    static float xyz[3];
    float        x, y, z;
    float        l = lab[0];
    float        a = lab[1];
    float        b = lab[2];

    x = Xn * lab2srgbHelper((a / 500.f) + (l + 16.f) / (116.f));
    y = Yn * lab2srgbHelper((l + 16.f) / 116.f);
    z = Zn * lab2srgbHelper((l + 16.f) / 116.f - (b / 200.f));
    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = z;

    TFInterpolator::xyz2srgb(xyz, rgb);
    return 0;
}

int TFInterpolator::srgb2msh(float rgb[3], float msh[3])
{
    static float lab[3];
    srgb2lab(rgb, lab);
    lab2msh(lab, msh);
    return 0;
}

int TFInterpolator::lab2msh(float lab[3], float msh[3])
{
    float m, s, h;
    float l = lab[0];
    float a = lab[1];
    float b = lab[2];

    m = sqrt(pow(l, 2) + pow(a, 2) + pow(b, 2));
    s = acos(l / m);
    h = atan2(b, a);
    msh[0] = m;
    msh[1] = s;
    msh[2] = h;
    return 0;
}

float srgb2labHelper(float val)
{
    float limit = 0.008856;
    if (val > limit)
        return pow(val, (float)(1.0 / 3.0));
    else
        return 7.787 * val + 16.0 / 116.0;
}

int TFInterpolator::srgb2lab(float rgb[3], float lab[3])
{
    float        l, a, b;
    static float xyz[3];
    static float srgb[3];

    // Newly added!
    rgb2srgb(rgb, srgb);

    srgb2xyz(rgb, xyz);
    // srgb2xyz(rgb,xyz);
    float x = xyz[0];
    float y = xyz[1];
    float z = xyz[2];

    l = 116.0 * (srgb2labHelper(y / Yn) - 16.0 / 116.0);
    a = 500.0 * (srgb2labHelper(x / Xn) - srgb2labHelper(y / Yn));
    b = 200.0 * (srgb2labHelper(y / Yn) - srgb2labHelper(z / Zn));
    lab[0] = l;
    lab[1] = a;
    lab[2] = b;
    return 0;
}

int TFInterpolator::xyz2srgb(float xyz[3], float srgb[3])
{
    float r, g, b;
    float rgb[3];
    float x = xyz[0];
    float y = xyz[1];
    float z = xyz[2];

    r = x * XYZinverseMatrix[0] + y * XYZinverseMatrix[3] + z * XYZinverseMatrix[6];
    g = x * XYZinverseMatrix[1] + y * XYZinverseMatrix[4] + z * XYZinverseMatrix[7];
    b = x * XYZinverseMatrix[2] + y * XYZinverseMatrix[5] + z * XYZinverseMatrix[8];
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;

    rgb2srgb(rgb, srgb);

    r = srgb[0];
    g = srgb[1];
    b = srgb[2];

    return 0;
}

int TFInterpolator::rgb2srgb(float rgb[3], float srgb[3])
{
    float val;
    for (int i = 0; i < 3; i++) {
        val = rgb[i] / 100.0;
        if (val > 0.00313080495356037152)
            val = (1.055 * pow(val, (float)(1.0 / 2.4)) - .055);
        else
            val = val * 12.92;
        val = val * 255.0;
        if ((val - floor(val)) > .5)
            val = ceil(val);
        else
            val = floor(val);
        srgb[i] = val;
    }

    /* Clip colors that are out of bounds
        float maxVal = srgb[0];
        if (maxVal < srgb[1]) maxVal = srgb[1];
        if (maxVal < srgb[2]) maxVal = srgb[2];
        cout << maxVal << endl;
        if (maxVal > 255.0) {
                cout << "Correcting by " << maxVal << endl;
                srgb[0] /= maxVal;
                srgb[1] /= maxVal;
                srgb[2] /= maxVal;
        }
        if (srgb[0]<0) srgb[0]=0;
        if (srgb[1]<0) srgb[1]=0;
        if (srgb[2]<0) srgb[2]=0;*/

    return 0;
}

int TFInterpolator::srgb2rgb(float srgb[3], float rgb[3])
{
    float val;

    for (int i = 0; i < 3; i++) {
        val = srgb[i] / 255.0;
        if (val > 0.04045) {
            val = pow(((val + 0.055) / 1.055), 2.4);
        } else {
            val = val / 12.92;
        }
        rgb[i] = val * 100;
    }

    /* Clip colors that are out of bounds
    double maxVal = rgb[0];
    if (maxVal < rgb[1]) maxVal = rgb[1];
    if (maxVal < rgb[2]) maxVal = rgb[2];
    cout << maxVal << endl;
    if (maxVal > 100.0) {
        cout << "Correcting by " << maxVal << endl;
        rgb[0] /= maxVal;
        rgb[1] /= maxVal;
        rgb[2] /= maxVal;
    }
    if (rgb[0]<0) rgb[0]=0;
    if (rgb[1]<0) rgb[1]=0;
    if (rgb[2]<0) rgb[2]=0;*/

    return 0;
}

int TFInterpolator::srgb2xyz(float rgb[3], float xyz[3])
{
    float linearRGB[3];
    float x, y, z, r, g, b;

    srgb2rgb(rgb, linearRGB);
    r = linearRGB[0];
    g = linearRGB[1];
    b = linearRGB[2];

    x = r * XYZtransferMatrix[0] + g * XYZtransferMatrix[3] + b * XYZtransferMatrix[6];
    y = r * XYZtransferMatrix[1] + g * XYZtransferMatrix[4] + b * XYZtransferMatrix[7];
    z = r * XYZtransferMatrix[2] + g * XYZtransferMatrix[5] + b * XYZtransferMatrix[8];
    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = z;
    return 0;
}

int TFInterpolator::rgb2hsv(float rgb[3], float hsv[3])
{
    // hsv         out;
    double min, max, delta;

    min = rgb[0] < rgb[1] ? rgb[0] : rgb[1];
    min = min < rgb[2] ? min : rgb[2];

    max = rgb[0] > rgb[1] ? rgb[0] : rgb[1];
    max = max > rgb[2] ? max : rgb[2];

    hsv[2] = max;    // 255.0;                                // v
    delta = max - min;
    if (delta < 0.00001) {
        hsv[1] = 0;
        hsv[0] = 0;    // undefined, maybe nan?
        return 0;
    }
    if (max > 0.0) {               // NOTE: if Max is == 0, this divide would cause a crash
        hsv[1] = (delta / max);    // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, v is undefined
        hsv[1] = 0.0;
        hsv[0] = NAN;    // its now undefined
        return 0;
    }
    if (rgb[0] >= max)                         // > is bogus, just keeps compilor happy
        hsv[0] = (rgb[1] - rgb[2]) / delta;    // between yellow & magenta
    else if (rgb[1] >= max)
        hsv[0] = 2.0 + (rgb[2] - rgb[0]) / delta;    // between cyan & yellow
    else
        hsv[0] = 4.0 + (rgb[0] - rgb[1]) / delta;    // between magenta & cyan

    hsv[0] *= 60.0;    // degrees

    if (hsv[0] < 0.0) hsv[0] += 360.0;

    // hsv[0] = hsv[0]/360.0;
    return 0;
}

int TFInterpolator::hsv2rgb(float hsv[3], float rgb[3])
{
    double hh, p, q, t, ff;
    long   i;
    // rgb         out;

    hsv[0] = hsv[0] * 360.0;    //*360.0;//2.0*3.14159;	// scale to unit circle

    if (hsv[1] <= 0.0) {    // < is bogus, just shuts up warnhsv[1]
        rgb[0] = hsv[2];
        rgb[1] = hsv[2];
        rgb[2] = hsv[2];
        return 0;
    }
    hh = hsv[0];
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = hsv[2] * (1.0 - hsv[1]);
    q = hsv[2] * (1.0 - (hsv[1] * ff));
    t = hsv[2] * (1.0 - (hsv[1] * (1.0 - ff)));

    switch (i) {
    case 0:
        rgb[0] = hsv[2];
        rgb[1] = t;
        rgb[2] = p;
        break;
    case 1:
        rgb[0] = q;
        rgb[1] = hsv[2];
        rgb[2] = p;
        break;
    case 2:
        rgb[0] = p;
        rgb[1] = hsv[2];
        rgb[2] = t;
        break;

    case 3:
        rgb[0] = p;
        rgb[1] = q;
        rgb[2] = hsv[2];
        break;
    case 4:
        rgb[0] = t;
        rgb[1] = p;
        rgb[2] = hsv[2];
        break;
    case 5:
    default:
        rgb[0] = hsv[2];
        rgb[1] = p;
        rgb[2] = q;
        break;
    }

    // rgb[0] = rgb[0]*255;
    // rgb[1] = rgb[1]*255;
    // rgb[2] = rgb[2]*255;

    return 0;
}
