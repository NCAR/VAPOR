//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MiscParams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		January 2005
//
//	Description:	Defines the MiscParams class
//		This is derived from the Params class
//		It contains all the parameters required for animation

//
#ifndef MISCPARAMS_H
#define MISCPARAMS_H


#include <vapor/ParamsBase.h>



//! \class MiscParams
//! \ingroup Public_Params
//! \brief A class that specifies parameters used in animation 
//! \author Alan Norton
//! \version 3.0
//! \date    February 2014

//! When this class is local, it controls the time-steps in one visualizer.
//! The global (shared) MiscParams controls the animation in any number of visualizers.

class MiscParams : public VAPoR::ParamsBase {
	
public: 


 MiscParams(
    ParamsBase::StateSave *ssave
 );

 MiscParams(
     VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node
 );


 virtual ~MiscParams();

 //! Set the use of the numeric time-step banner in the visualizer
 //
 void SetTimeStep(bool val) {
	SetValueLong(_timeStepTag, "Set use of time step banner", (long)val);
 }

 //! Get value indicating whether numeric time-step banner is being used
 //! \retval bool indicating usage
 //
 bool GetTimeStep() const {
	return (bool)GetValueLong(_timeStepTag, 0);
 }
 
 //! Set the use of a formatted time-step banner in the visualizer
 //
 void SetTimeStamp(bool val) {
	SetValueLong(_timeStampTag, "Set use of time step string banner", (long)val);
 }

 //! Get value indicating whether formatted time string banner is being used
 //! \retval bool indicating usage
 //
 bool GetTimeStamp() const {
	return (bool)GetValueLong(_timeStampTag, 0);
 }

 void SetTimeAnnotLLX(float llx) {
	SetValueLong(_timeAnnotLLXTag, 
		"Set the lower-left coord of the time annotation", (long)llx
	);
 }

 float GetTimeAnnotLLX() const {
	return (float)GetValueLong(_timeAnnotLLXTag, 10);
 }

 void SetTimeAnnotLLY(float lly) {
	SetValueLong(_timeAnnotLLYTag, 
		"Set the lower-right coord of the time annotation", (long)lly
	);
 }

 float GetTimeAnnotLLY() const {
	return (float)GetValueLong(_timeAnnotLLYTag, 10);
 }

 void SetTimeAnnotSize(float size) {
	cout << "SetTimeAnnotSize() " << size << endl;
	SetValueLong(_timeAnnotSizeTag, 
		"Set the text size of the time annotation", (long)size
	);
 }

 int GetTimeAnnotSize() const {
	int val = (int)GetValueLong(_timeAnnotSizeTag, 20);
	cout << "GetTimeAnnotSize() " << val << endl;
	return val;
 }

 void SetTimeAnnotColor(const std::vector <double> &rgb) {
	assert(rgb.size() == 3);
	vector <double> v = rgb;
	for (int i=0; i<v.size(); i++) {
		if (v[i] < 0.0) v[i] = 0.0;
		if (v[i] > 1.0) v[i] = 1.0;
	}
	SetValueDoubleVec(_timeAnnotColorTag, "Specify time "
					"annotation color in RGB", v);
 }

 void GetTimeAnnotColor(std::vector <double> &rgb) const {
	rgb.clear();

	vector <double> defaultv(3,1.0);
	rgb =  GetValueDoubleVec(_timeAnnotColorTag, defaultv);
	for (int i=0; i<rgb.size(); i++) {
		if (rgb[i] < 0.0) rgb[i] = 0.0;
		if (rgb[i] > 1.0) rgb[i] = 1.0;
	}
 }

 void GetTimeAnnotColor(float rgb[3]) const {
    vector <double> rgbv;
	GetTimeAnnotColor(rgbv);
    for (int i=0; i<3; i++) rgb[i] = rgbv[i];
 }

 // Get static string identifier for this params class
 //
 static string GetClassType() {
	return("MiscParams");
 }

private:
 static const string _timeStepTag;
 static const string _timeStampTag;
 static const string _timeAnnotLLXTag;
 static const string _timeAnnotLLYTag;
 static const string _timeAnnotSizeTag;
 static const string _timeAnnotColorTag;

 //! Put a params instance into default state with no data.
 void _init();

};

#endif //MISCPARAMS_H 
