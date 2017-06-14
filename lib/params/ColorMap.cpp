//--ColorMap.cpp ------------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// A map from data value to/from color.
// 
//----------------------------------------------------------------------------


#include <iostream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <vapor/ColorMap.h>

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

using namespace std;
using namespace VAPoR;
using namespace Wasp;

//----------------------------------------------------------------------------
// Static member initalization
//----------------------------------------------------------------------------
const string ColorMap::_controlPointsTag = "ColorMapControlPoints";
const string ColorMap::_interpTypeTag = "ColorInterpolationType";
const string ColorMap::_dataBoundsTag = "DataBounds";

//
// Register class with object factory!!!
//
static ParamsRegistrar<ColorMap> registrar(ColorMap::GetClassType());



//============================================================================
// Class ColorMap::Color
//============================================================================

//----------------------------------------------------------------------------
// Default constructor (white)
//----------------------------------------------------------------------------
ColorMap::Color::Color() :
  _hue(0.0),
  _sat(0.0),
  _val(1.0)
{
}

//----------------------------------------------------------------------------
// Constructor 
//----------------------------------------------------------------------------
ColorMap::Color::Color(float h, float s, float v) :
  _hue(h),
  _sat(s),
  _val(v)
{
}
//----------------------------------------------------------------------------
// Constructor 
//----------------------------------------------------------------------------
ColorMap::Color::Color(double h, double s, double v) :
  _hue((float)h),
  _sat((float)s),
  _val((float)v)
{
}

//----------------------------------------------------------------------------
// Copy Constructor 
//----------------------------------------------------------------------------
ColorMap::Color::Color(const Color &c) :
  _hue(c._hue),
  _sat(c._sat),
  _val(c._val)
{
}


//----------------------------------------------------------------------------
// Return the rgb components of the color (0.0 ... 1.0)
//----------------------------------------------------------------------------
void ColorMap::Color::toRGB(float *rgb)
{

  /*	
   *  hsv-rgb Conversion function.  inputs and outputs	between 0 and 1
   *	copied (with corrections) from Hearn/Baker
   */
  if (_sat == 0.f)  //grey
  {
    rgb[0] = rgb[1] = rgb[2] = _val;
    return;
  }

  int sector    = (int)(_hue*6.f);   
  float sectCrd = _hue*6.f - (float) sector;

  if (sector == 6)
  { 
    sector = 0;
  }

  float a = _val*(1.f - _sat);
  float b = _val*(1.f - sectCrd*_sat);
  float c = _val*(1.f - (_sat*(1.f - sectCrd)));

  switch (sector)
  {
	case (0): //red to green, r>g
      rgb[0] = _val;
      rgb[1] = c;
      rgb[2] = a;
      break;
    case (1): // red to green, g>r
      rgb[1] = _val;
      rgb[2] = a;
      rgb[0] = b;
      break;
	case (2): //green to blue, gr>bl
      rgb[0] = a;
      rgb[1] = _val;
      rgb[2] = c;
      break;
	case (3): //green to blue, gr<bl
      rgb[0] = a;
      rgb[2] = _val;
      rgb[1] = b;
      break;
	case (4): //blue to red, bl>red
      rgb[1] = a;
      rgb[2] = _val;
      rgb[0] = c;
      break;
	case (5): //blue to red, bl<red
      rgb[1] = a;
      rgb[0] = _val;
      rgb[2] = b;
      break;
    default: 
      assert(0);
  }

}



//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
 
ColorMap::ColorMap(
	ParamsBase::StateSave *ssave
) : ParamsBase(ssave, ColorMap::GetClassType()) {

	SetInterpType(TFInterpolator::diverging);
 
	// Create a default color map with 4 control points:
	//
	// Ken Moreland's "Smooth Cool Warm" color map
	//
	vector<double> cps;
	cps.push_back(232.0/360.0);
	cps.push_back(.695);
	cps.push_back(.757);
	cps.push_back(0.);
	cps.push_back(348.0/360.0);
	cps.push_back(.977);
	cps.push_back(.706);
	cps.push_back(.99);

	SetControlPoints(cps);

}

ColorMap::ColorMap(
	ParamsBase::StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node) {
}


//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
ColorMap::~ColorMap()
{
}

//----------------------------------------------------------------------------
// Clear (& deallocated) the control points
//----------------------------------------------------------------------------
void ColorMap::clear()
{
  vector<double> cps;
  SetControlPoints(cps);
}



//----------------------------------------------------------------------------
// Return the control point's color
//---------------------------------------------------------------------------- 
ColorMap::Color ColorMap::controlPointColor(int index) const
{
	vector<double> cps = GetControlPoints();
	if (index+4 > cps.size()*4) return Color();

	return Color(cps[4*index],cps[4*index+1],cps[4*index+2]);
}


//----------------------------------------------------------------------------
// Set the control point's color.
//---------------------------------------------------------------------------- 
void ColorMap::controlPointColor(int index, Color color)
{
	vector<double> cps = GetControlPoints();
	if (index+4 > cps.size()*4) return;	// no-op

	cps[4*index] = color.hue();
	cps[4*index+1] = color.sat();
	cps[4*index+2] = color.val();
	SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Return the control point's value (in data coordinates).
//---------------------------------------------------------------------------- 
float ColorMap::controlPointValue(int index) const
{
	vector<double> cps = GetControlPoints();
	if (index+4 > cps.size()*4) return(0.0);	// no-op

	float norm = (float)cps[4*index+3];

	return (norm * (maxValue() - minValue()) + minValue());
}

//----------------------------------------------------------------------------
// Set the control point's value (in data coordinates).
//---------------------------------------------------------------------------- 
void  ColorMap::controlPointValue(int index, float value)
{
 
  vector<double> cps = GetControlPoints();
  if (index+4 > cps.size()*4) return;	// no-op

  float nv = (value - minValue()) / (maxValue() - minValue());

  float minVal = 0.0;
  float maxVal = 1.0;

  if (index > 0)
  {
    minVal = (cps[3]);
  }
  if (index < cps.size()/4 -1)
	maxVal = cps[cps.size()-1];
  
  if (nv < minVal)
  {
    nv = minVal;
  }
  else if (nv > maxVal)
  {
    nv = maxVal;
  }
  cps[index*4+3] = nv;
  SetControlPoints(cps);
}



//----------------------------------------------------------------------------
// Add a new control point to the colormap.
//----------------------------------------------------------------------------
void ColorMap::addControlPointAt(float value)
{
  Color c = color(value);

  float nv = (value - minValue()) / (maxValue() - minValue());

  vector<double> cps = GetControlPoints();
  //Find the insertion point:
  int indx = leftIndex(nv)*4+4;
  cps.insert(cps.begin()+indx++, c.hue());
  cps.insert(cps.begin()+indx++, c.sat());
  cps.insert(cps.begin()+indx++, c.val());
  cps.insert(cps.begin()+indx, nv);
  
  SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Add a new control point to the colormap.
//----------------------------------------------------------------------------
void ColorMap::addControlPointAt(float value, Color color)
{
  
  vector<double> cps = GetControlPoints();
  //Find the insertion point:
  int indx = leftIndex(value)*4;
  cps.insert(cps.begin()+indx++, color.hue());
  cps.insert(cps.begin()+indx++, color.sat());
  cps.insert(cps.begin()+indx++, color.val());
  cps.insert(cps.begin()+indx, value);
  
  SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Add a new control point to the colormap.
//----------------------------------------------------------------------------
void ColorMap::addNormControlPoint(float normValue, Color color)
{
   
  vector<double> cps = GetControlPoints();
  //Find the insertion point:
  int indx = leftIndex(normValue)*4;
  cps.insert(cps.begin()+indx++, color.hue());
  cps.insert(cps.begin()+indx++, color.sat());
  cps.insert(cps.begin()+indx++, color.val());
  cps.insert(cps.begin()+indx, normValue);
  
  SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Delete the control point.
//----------------------------------------------------------------------------
void ColorMap::deleteControlPoint(int index)
{
  vector<double> cps = GetControlPoints();
  if (index >= 0 && index < cps.size()/4 && cps.size() > 4)
  {
	cps.erase(cps.begin()+4*index, cps.begin()+4*index+4);
   
	SetControlPoints(cps);
  }
}

//----------------------------------------------------------------------------
// Move the control point, but not past adjacent control points
//---------------------------------------------------------------------------- 
void ColorMap::move(int index, float delta)
{
  vector<double> cps = GetControlPoints();
  if (index > 0 && index < cps.size()/4 -1) //don't move first or last control point!
  {
    
    float ndx = delta / (maxValue() - minValue());

	float minVal = cps[index*4 -1]; //value to the left
   
    float maxVal = cps[index*4 + 7]; //value to the right
  
    float value = cps[index*4+3] + ndx;

	if (value < 0.005) 
		value = 0.005;
	
    if (value <= minVal)
    {
      value = minVal;
    }
    else if (value >= maxVal)
    {
      value = maxVal;
    }
    
    cps[index*4+3] = value;
	SetControlPoints(cps);
  }
}


//----------------------------------------------------------------------------
// Interpolate a color at the value (data coordinates)
//
// Developed by Alan Norton. 
//----------------------------------------------------------------------------
ColorMap::Color ColorMap::color(float value) const
{
  //
  // normalize the value
  // 
  float nv = (value - minValue()) / (maxValue() - minValue());
  vector<double> cps = GetControlPoints();
  //
  // Find the bounding control points
  //
  int index = leftIndex(nv);

  assert(index>=0 && index*4+7< cps.size());
  double leftVal = cps[4*index +3];
  double rightVal = cps[4*index + 7];

  float ratio = (nv - leftVal)/(rightVal - leftVal);

  if (ratio > 0.f && ratio < 1.f)
  {
	TFInterpolator::type itype = GetInterpType();
	if (itype == TFInterpolator::diverging){
		float hsv1[3] = {(float)cps[4*index],(float)cps[4*index+1],(float)cps[4*index+2]};
		float hsv2[3] = {(float)cps[4*index+4],(float)cps[4*index+5],(float)cps[4*index+6]};
		float rgb1[3], rgb2[3];
		float rgbOutput[3], hsvOutput[3];
		
		TFInterpolator::hsv2rgb(hsv1,rgb1);
		rgb1[0] = rgb1[0]*255.0;
		rgb1[1] = rgb1[1]*255.0;
		rgb1[2] = rgb1[2]*255.0;
		
		TFInterpolator::hsv2rgb(hsv2,rgb2);
		rgb2[0] = rgb2[0]*255.0;
		rgb2[1] = rgb2[1]*255.0;
		rgb2[2] = rgb2[2]*255.0;

		TFInterpolator::divergentInterpolation(rgb2, rgb1, rgbOutput, ratio);
		TFInterpolator::rgb2hsv(rgbOutput, hsvOutput);
		
		//Normalize HSV output
		hsvOutput[0] = hsvOutput[0]/360.0;
		hsvOutput[2] = (float)hsvOutput[2]/255.0;

		return Color(hsvOutput[0], hsvOutput[1], hsvOutput[2]);
	}
	else if (itype == TFInterpolator::linear){
		float h = TFInterpolator::interpCirc(itype, 
                                         cps[4*index], //hue
                                         cps[4*index+4], 
                                         ratio);

		float s = TFInterpolator::interpolate(itype, 
                                          cps[4*index+1], //sat
                                          cps[4*index+5], 
                                          ratio);

		float v = TFInterpolator::interpolate(itype, 
                                          cps[4*index+2],//val
                                          cps[4*index+6], 
                                          ratio);

		return Color(h, s, v);
	}
	else if (itype == TFInterpolator::discrete){
		if (ratio < .5){
			float h = cps[4*index];
			float s = cps[4*index+1];
			float v = cps[4*index+2];
			return Color(h,s,v);
		}
		else {
			float h = cps[4*index+4];
			float s = cps[4*index+5];
			float v = cps[4*index+6];
			return Color(h,s,v);
		}
	}
  }

  if (ratio >= 1.0)
  {
    return Color(cps[4*index+4],cps[4*index+5],cps[4*index+6]);
  }

  return Color(cps[4*index],cps[4*index+1],cps[4*index+2]);
}
 

//----------------------------------------------------------------------------
// binary search , find the index of the largest control point <= val
// Requires that control points are increasing. 
//
// Developed by Alan Norton. 
//----------------------------------------------------------------------------
int ColorMap::leftIndex(float val) const
{
  vector<double> cps = GetControlPoints();
  int left = 0;
  int right = cps.size()/4 -1;

  //
  // Iterate, keeping left to the left of ctrl point
  //
  while (right-left > 1)
  {
    int mid = left+ (right-left)/2;
	if (cps[mid*4+3] > val)
    {
      right = mid;
    }
    else 
    {
      left = mid;
    }
  }
  return left;
}


vector<double> ColorMap::GetControlPoints() const {

	vector <double> cps = GetValueDoubleVec(_controlPointsTag);

	while (cps.size() % 4) cps.push_back(0.0);

	return(cps);
}

void ColorMap::SetControlPoints(const vector<double> &controlPoints) {
	vector <double> mycp = controlPoints;

	while (mycp.size() % 4) mycp.push_back(0.0);

	SetValueDoubleVec(
		_controlPointsTag, "Set color control points", mycp
	);
}

void ColorMap::SetInterpType(TFInterpolator::type t){
	SetValueLong(
		_interpTypeTag, "Set Color Interpolation", (long)t
	);
}

void ColorMap::SetDataBounds(const vector <double> &bounds) {
	assert(bounds.size() == 2);

	SetValueDoubleVec(_dataBoundsTag, "Set min max map value", bounds);
}

vector <double> ColorMap::GetDataBounds() const {
	vector <double> defaultv(2, 0.0);

	vector <double> bounds = GetValueDoubleVec(_dataBoundsTag, defaultv);

	if (bounds.size() != 2) bounds = defaultv;

	return(bounds);
}
