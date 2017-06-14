//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		TFInterpolator.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2004
//
//	Description:	Defines the TFInterpolator class:
//		A class to interpolate transfer function values
//		Currently only supports linear interpolation
//
#ifndef TFINTERPOLATOR_H
#define TFINTERPOLATOR_H
#include <iostream>
#include <cmath>

#include <vapor/common.h>

namespace VAPoR {
class PARAMS_API TFInterpolator {
  public:
    ~TFInterpolator();

    //Default is linear
    enum type {
        linear,
        discrete,
        logarithm,
        exponential,
        diverging
    };
    //Determine the interpolated value at intermediate value 0<=r<=1
    //where the value at left and right endpoint is known
    //This method is just a stand-in until we get more sophistication
    //
    static float interpolate(type, float leftVal, float rightVal, float r);

    //Linear interpolation for circular (hue) fcn.  values in [0,1).
    //If it's closer to go around 1, then do so
    //
    static float interpCirc(type t, float leftVal, float rightVal, float r);

    //! Generate the complete diverging color map using the Moreland
    //! technique from RGB1 to RGB2, placing "white" in the middle.
    //! The number of points given by the "numColors" variable
    //! controls the resolution of the color map.
    //!
    //! \param[in] RGB1 Start point color for diverging color map.
    //! \param[in] RGB2 End point color for diverging color map.
    //! \param[in] Number of colors to generate in the new color map.
    //! \retval Return status, 1 = success
    static float *genDivergentMap(float rgb1[3], float rgb2[3], int numColors);

    //private:

    //! Interpolation algorithm to generate a single interpolated
    //! color in a diverging color map.
    //!
    //! \param[in] RGB1 Start point color for diverging color map
    //! \param[in] RGB2 End point color for diverging color map.
    //! \param[in] The output interpolated RGB value in the divergent map
    //! \param[in] The interpolation weight of the current color in the color map
    //! \retval Return status, 1 = success
    static int divergentInterpolation(float rgb1[3], float rgb2[3], float output[3], float interp);

    //! A function to provide an adjusted hue when interpolating
    //! to an unsaturated color in Msh space
    //! \param[in] Input M value to adjust hue to
    //! \param[in] Input s value to adjust hue to
    //! \param[in] Input h value to adjust hue to
    //! \param[in] Second input M value to adjust hue to
    //! \param[out] Adjusted hue value
    static float adjustHue(float m1, float s1, float h1, float m2);

    //! Conversion of Msh to RGB
    //! \param[in] A 3-tuple color in Msg space
    //! \param[in] The resultant 3-tuple color in rgb space
    //! \retval Return status, 1 = success
    static int msh2srgb(float msh[3], float rgb[3]);

    //! Conversion of Msh to Lab
    //! \param[in] A 3-tuple color in Msh space
    //! \param[in] The resultant 3-tuple color in Lab space
    //! \retval Return status, 1 = success
    static int msh2lab(float msh[3], float lab[3]);

    //! Conversion of Lab to RGB
    //! \param[in] A 3-tuple color in Lab space
    //! \param[in] The resultant 3-tuple color in rgb space
    //! \retval Return status, 1 = success
    static int lab2srgb(float lab[3], float rgb[3]);

    //! Conversion from RGB to Msh
    //! \param[in] A 3-tuple color in rgb space
    //! \param[in] The resultant 3-tuple color in Msh space
    //! \retval Return status, 1 = success
    static int srgb2msh(float rgb[3], float msh[3]);

    //! Conversion from RGB to Lab
    //! \param[in] A 3-tuple color in rgb space
    //! \param[in] The resultant 3-tuple color in Lab space
    //! \retval Return status, 1 = success
    static int srgb2lab(float rgb[3], float lab[3]);

    //! Conversion from Lab to Msh
    //! \param[in] A 3-tuple color in rgb space
    //! \param[in] The resultant 3-tuple color in Msh space
    //! \retval Return status, 1 = success
    static int lab2msh(float lab[3], float msh[3]);

    //! Conversion from Lab to Msh
    //! \param[in] A 3-tuple color in rgb space
    //! \param[in] The resultant 3-tuple color in xyz space
    //! \retval Return status, 1 = success
    static int srgb2xyz(float rgb[3], float xyz[3]);

    //! Conversion from xyz to rgb
    //! \param[in] A 3-tuple color in xyz space
    //! \param[in] The resultant 3-tuple color in rgb space
    //! \retval Return status, 1 = success
    static int xyz2srgb(float xyz[3], float rgb[3]);

    //! Convert an RGB tuple to sRGB
    //! \param[in] A 3-tuple color in rgb space
    //! \param[in] The resultant 3-tuple color in sRGB space
    //! \retval Return status, 1 = success
    static int rgb2srgb(float rgb[3], float srgb[3]);

    //! Convert an sRGB tuple to RGB
    //! \param[in] A 3-tuple color in sRGB space
    //! \param[in] The resultant 3-tuple color in RGB space
    //! \retval Return status, 1 = success
    static int srgb2rgb(float srgb[3], float rgb[3]);

    static int rgb2hsv(float rgb[3], float hsv[3]);
    static int hsv2rgb(float hsv[3], float rgb[3]);

    static float Xn;
    static float Yn;
    static float Zn;
    static float XYZtransferMatrix[9];
    static float XYZinverseMatrix[9];
    static float *colorMap;
};

}; // namespace VAPoR

#endif //TFINTERPOLATOR_H
