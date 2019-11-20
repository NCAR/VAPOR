//--ColorMap.h ---------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// A map from data value to/from color.
// 
//----------------------------------------------------------------------------

#ifndef ColorMap_H
#define ColorMap_H

#include <vapor/ParamsBase.h>
#include <vapor/TFInterpolator.h>

namespace VAPoR {


class PARAMS_API ColorMap : public ParamsBase
{

public:

 class PARAMS_API Color {
 public:

  Color();
  Color(float h, float s, float v);
  Color(double h, double s, double v);
  Color(const Color &color);

  void toRGB(float *rgb) const;

  void  hue(float h) { _hue = h; }
  float hue() const  { return _hue; }

  void  sat(float s) { _sat = s; }
  float sat() const  { return _sat; }

  void  val(float v) { _val = v; }
  float val() const  { return _val; }

 private:

  float _hue;
  float _sat;
  float _val;
 };

 
 //! Create a ColorMap object from scratch
 //
 ColorMap(ParamsBase::StateSave *ssave);

 //! Create a ColorMap object from an existing XmlNode tree
 //
 ColorMap(
    ParamsBase::StateSave *ssave, XmlNode *node
 );

 virtual ~ColorMap();

 void  clear();

 TFInterpolator::type GetInterpType() const {
	long defaultv = TFInterpolator::diverging;
	return (TFInterpolator::type) GetValueLong(_interpTypeTag, defaultv);
 }
 void SetInterpType(TFInterpolator::type t);
 void SetUseWhitespace(bool enabled);
 bool GetUseWhitespace() const;

 int numControlPoints() const {
	return (int)(GetControlPoints().size()/4); 
 }

 Color controlPointColor(int index) const;
 void  controlPointColor(int index, Color color);

 float controlPointValue(int index) const;               // Data Coordinates
 float controlPointValueNormalized(int index) const;
 void  controlPointValue(int index, float value);  // Data Coordinates
 void  controlPointValueNormalized(int index, float value);

 void addControlPointAt(float value);
 int addNormControlPointAt(float value);
 void addControlPointAt(float value, Color color);
 int addNormControlPoint(float normValue, Color color);
 void deleteControlPoint(int index);

 void move(int index, float delta);
 
 Color color(float value) const;
 Color colorNormalized(float nv) const;
 Color getDivergingColor(float ratio, float index) const;
 Color getCorrectiveDivergingColor(float ratio, float index) const;

 // Method to obtain the control points as a double vector, with 
 // 4 entries for each control point
 // in the order hue,sat,value, datavalue
 vector<double> GetControlPoints() const;

 void SetControlPoints(const vector<double> &controlPoints);


 //!
 //! The minimum value is stored as normalized coordinates in the parameter
 //! space. Therefore, the color map will change relative to any changes in
 //! the parameter space. True???
 //
 void SetDataBounds(const vector <double> &bounds); 
 vector <double> GetDataBounds() const;

 float minValue() const {
	return(GetDataBounds()[0]);
 }
 float maxValue() const {
	return(GetDataBounds()[1]);
 }
    
    void Reverse();


 // Get static string identifier for this params class
 //
 static string GetClassType() {
	return("ColorMapParams");
 }

public:
 static const string _controlPointsTag; 
 static const string _interpTypeTag;
 static const string _useWhitespaceTag;
 static const string _dataBoundsTag;
private:

 int leftIndex(float val) const;
 
};

class PARAMS_API ARGB{
public:
	ARGB(int r, int g, int b){
		_argbvalue = ((r&255)<<16) | ((g&255)<<8) | (b&255);
	}
private:
	unsigned int _argbvalue;

};
};

#endif // ColorMap_H
