//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//					
//	File:		MapperFunction.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2005
//
//	Description:	Defines the MapperFunction class  
//		This is the mathematical definition of a function
//		that can be used to map data to either colors or opacities
//		Subclasses can either implement color or transparency 
//		mapping (or both), and/or identify one or more isovalues.

#ifndef MAPPERFUNCTION_H
#define MAPPERFUNCTION_H

#include <vapor/OpacityMap.h>
#include <vapor/ColorMap.h>
#include <vapor/TFInterpolator.h>
#include <vapor/ParamsBase.h>

namespace VAPoR {

class XmlNode;
class ParamNode;

//! \class MapperFunction
//! \brief Parent class for TransferFunction and IsoControl,
//! supports positioning histogram over color/opacity maps as well as
//! a set of isovalues (as with Contours)
//! \author Alan Norton
//! \version 3.0
//! \date January 2016
class PARAMS_API MapperFunction : public ParamsBase 
{

public:
	
 MapperFunction(
	ParamsBase::StateSave *ssave, const string &classname
 );

 MapperFunction(
	 ParamsBase::StateSave *ssave, XmlNode *node
 );

 MapperFunction(
    const MapperFunction &rhs
 );

 MapperFunction &operator=( const MapperFunction& rhs );


 virtual ~MapperFunction();

 //! Determine the opacity value at a particular data value
 //! \param[in] point float data value
 float getOpacityValueData(float point) const;

 //! Determine the color value (in HSV) at a data point
 //! \param[in] point Data value
 //! \param[out] h Hue
 //! \param[out] sat Saturation
 //! \param[out] val Value
 void  hsvValue(float point, float* h, float* sat, float* val) const;

 //! Determine the color value (in RGB) at a data point
 //! \param[in] point Data value
 //! \param[out] rgb r,g,b floats
 void  rgbValue(float point, float rgb[3]) const {
	float hsv[3];
	hsvValue(point, hsv, hsv+1, hsv+2);
	hsvToRgb(hsv, rgb);
 }

 //! Make the opacity map completely opaque (opacity 1)
 void setOpaque();

 //! Determine if the map is totally opaque (opacity 1)
 //! \return true if opaque.
 bool isOpaque() const;

 //! Build a color/opacity lookup table.
 //! Caller must supply an array to be filled in.
 //! Each entry isa 4-tuple: r,g,b,opacity.
 //! \param[out] clut lookup table of size _numEntries*4
 void makeLut(float* clut) const;

 //! Obtain minimum mapping (histo) value
 //! \return Minimum mapping value
 float getMinMapValue() const {
	return(getMinMaxMapValue()[0]);
 };

 //! Obtain maximum mapping (histo) value
 //! \return Maximum mapping value
 float getMaxMapValue() const {
	return(getMinMaxMapValue()[1]);
 };

 //! Set both minimum and maximum mapping (histo) values
 //! \param[in] val1 minimum value
 //! \param[in] val2 maximum value
 void setMinMaxMapValue(float val1,float val2);

 void setMinMapValue(float val) {
	setMinMaxMapValue(val, getMaxMapValue());
 }

 void setMaxMapValue(float val) {
	setMinMaxMapValue(getMinMapValue(), val);
 }

 //! Obtain min and max mapping (histo) values
 //! \return minimum and maximum as a 2-vector of doubles
 vector<double> getMinMaxMapValue() const;

 //! Create an opacity map for this transfer function
 //! \param[in] type of opacity map
 virtual OpacityMap* createOpacityMap(
	OpacityMap::Type type=OpacityMap::CONTROL_POINT
 );

 //! Delete an opacity map from this transfer function
 //! \param[in] omap Pointer to map to delete
 //
 void DeleteOpacityMap(const OpacityMap *omap);


 //! Obtain the opacity map associated with an index.
 //! specified index identifies which of the opacity maps is requested.
 //! \param[in] index Opacity map index of desired map.
 virtual OpacityMap* GetOpacityMap(int index) const;


 //! Determine how many opacity maps are available
 //! \return number of opacity maps
 int         getNumOpacityMaps() const {
	return(m_opacityMaps->Size());
 }; 


//! Specify an opacity scale factor applied to all opacity maps
//! \param[in] val opacity scale factor
 void setOpacityScale(double val) { 
	SetValueDouble(_opacityScaleTag, "Set Opacity Scale", val);
 }

 //! Identify the current opacity scale factor
 //! \return current opacity scale factor
 double getOpacityScale() const {
	return GetValueDouble(_opacityScaleTag, 1.0);
 }

 //! Opacity composition types
 enum CompositionType {
	ADDITION = 0,
	MULTIPLICATION = 1
 };

 //! Specify the type of opacity composition (ADDITION or MULTIPLICATION)
 //! \param[in] t CompositionType
 void setOpacityComposition(CompositionType t) { 
	SetValueLong(
		_opacityCompositionTag, "Set Opacity Composition Type", (long) t
	);
}

//! Obtain the type of opacity composition (ADDITION or MULTIPLICATION)
//! \return CompositionType
 CompositionType getOpacityComposition() const {
	return (CompositionType) GetValueLong(_opacityCompositionTag, ADDITION); 
 }
    
 //! Utility method converts HSV to RGB
 //! \param[in] hsv (HSV as float[3] array)
 //! \param[out] rgb (RGB as float[3] array)
 static void hsvToRgb(float* hsv, float* rgb);

 //! Utility method converts RGB to HSV
 //! \param[in] rgb (RGB as float[3] array)
 //! \param[out] hsv (HSV as float[3] array)
 static void rgbToHsv(float* rgb, float* hsv);

 //! Obtain the number of entries in the color/opacity map
 //! \return number of entries
 int getNumEntries() const { return _numEntries; }

  
 //! Map and quantize a real value to the corresponding table index
 //! i.e., quantize to current Mapper function domain
 //! \param[in] point value to be quantized
 //! \return quantized value in [0,_numEntries-1]
 int mapFloatToIndex(float point)  const {
	int indx = mapPosition(point, 
	getMinMapValue(), 
	getMaxMapValue(), _numEntries-1);
	if (indx < 0) indx = 0;
	if (indx > _numEntries-1) indx = _numEntries-1;
	return indx;
 }

 //! Determine float value associated with index
 //! \param[in] indx index (between 0 and _numEntries)
 //! \return corresponding float value
 float mapIndexToFloat(int indx) const {
	return (float)(getMinMapValue() + 
		((float)indx)*(float)(getMaxMapValue()- getMinMapValue())/
		(float)(_numEntries-1));
 }

 //! Obtain the color interpolation type
 //! \return TFInterpolator::type color interpolation type
 TFInterpolator::type getColorInterpType() {
	ColorMap* cmap = GetColorMap();
	if (cmap) return cmap->GetInterpType();
	return TFInterpolator::linear;
 }

 //! Specify the color interpolation type
 //! \param[in] t color interpolation type
 void setColorInterpType(TFInterpolator::type t){
	ColorMap* cmap = GetColorMap();
	if (cmap) cmap->SetInterpType(t);
 }

 //! Method to get the Color Map from the Mapper Function
 //! \return ColorMap pointer to the Color Map
 virtual ColorMap* GetColorMap() const {
	return(m_colorMap);
 }

 //! Method to get the state of the automatic histogram
 //! update setting.
 //! \return The state of the autoUpdateHisto checkbox
 //
 bool GetAutoUpdateHisto() {
	return ((bool) GetValueLong(_autoUpdateHistoTag, (int) false));
 }

 //! Method to set the state of the automatic histogram
 //! update setting.
 //! \param[in] State of the autoUpdateHisto setting
 //
 void SetAutoUpdateHisto(bool val) {
	SetValueLong(
		_autoUpdateHistoTag,"enable/disable auto update of histogram",val
	);
 }

 //! Method to get the state of whether the current mapper function
 //! applies color through a primary variable or secondary variable.
 //! For example, Barbs may have a "Color Mapped Variable" that colors
 //! the barbs according to a "Secondary variable", independent of
 //! the vector variables that define the Barbs.  Isosurfaces can have
 //! Secondary Variable colorings too.
 //!
 bool GetSecondaryVarMapper() {
	return ((bool) GetValueLong(_autoUpdateHistoTag, (int) false));
 }
 
 //! Method to set the state of whether the current mapper function
 //! applies color through a ColorMappedVariable, or a renderer's primary
 //! variable.
 //! \param[in] State of the Secondary Variable color setting
 //
 void SetSecondaryVarMapper(bool val) {
	SetValueLong(
		_secondaryVarMapperTag,"Apply color through a secondary color",val
	);
 }

private:

	
 //
 // XML tags
 //

 static const string _dataBoundsTag;
 static const string _opacityCompositionTag;
 static const string _opacityScaleTag;
 static const string _opacityMapsTag;
 static const string _opacityMapTag;
 static const string _autoUpdateHistoTag;
 static const string _secondaryVarMapperTag;

 //
 // Size of lookup table.  Always 1<<8 currently!
 //
 const int _numEntries;

 ParamsContainer *m_opacityMaps;
 ColorMap *m_colorMap;

 //!
 //! Map a point to the specified range, and quantize it.
 //! \param[in] x point value
 //! \param[in] minvalue minimum value
 //! \param[in] maxValue maximum value
 //!
 static int mapPosition(float x, float minValue, float maxValue, int hSize);

 //Construct name for new parent of opac map node, unique for this instance;
 string getNewOpacMapTag(int* tagIndex);

 //Construct a map tag for a specific index
 string getOpacMapTag(int index) const;

 //Extract the index from the opacity map tag
 int getOpacMapNum(string tag);

 string _make_omap_name(int index) const;

};

#ifdef	DEAD

//! \class MFContainer
//! \brief A simple container class for managing a collection
//! of MapperFunctions, typically each associated with a different
//! variable.
//!
//! \author John Clyne
//! \date February 2016
class PARAMS_API MFContainer : public MyBase  {
public: 

	MFContainer();

	//! Construct an empty container. 
	//!
	//! \param[in] p A pointer to the RenderParams that will
	//! store the subsequent collection.
	//! \param[in] tag Name to give the collection
	//!
	//! \sa ParamsBase::SetParamsBase()
	//
	MFContainer(RenderParams *p, string tag);

	~MFContainer();

	//! Insert a MapperFunction in the collection
	//!
	//! This method inserts the MapperFunction \p mf into the 
	//! collection and gives it the name specified by \p name
	//
	void Insert(string name, MapperFunction *mf);

	//! Return the named MapperFunction
	//!
	//! This method returns the MapperFunction named by \p name, if it 
	//! exists. Otherwise it returns NULL. No error is generated
	//!
	MapperFunction *GetMF(string name) const;

	//! Remove all elements from list
	//
	void Clear();

	//! Erase the named MapperFunction
	//
	void Erase(string name);

	//! Return a list of names of all of the MapperFunctions
	//
	vector <string> GetNames() const;


private:
	RenderParams *_params;
	string _tag;


};
#endif

};
#endif //MAPPERFUNCTION_H
