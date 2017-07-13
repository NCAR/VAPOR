//************************************************************************
//									*
//		     Copyright (C)  1024				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MouseModeParams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		April 2014
//
//	Description:	Defines the MouseModeParams class.
//		This Params class is global.  Specifies the current MouseMode.
//
#ifndef MOUSEMODEPARAMS_H
#define MOUSEMODEPARAMS_H


#include <vector>
#include <map>

#include <vapor/ParamsBase.h>



//! \class MouseModeParams
//! \ingroup Public_Params
//! \brief A class for describing mouse modes in use in VAPOR
//! \author Alan Norton
//! \version 3.0
//! \date    April 2014
//!
class PARAMS_API MouseModeParams : public VAPoR::ParamsBase {
	
public: 

 //! Create a MouseModeParams object from scratch
 //     
 MouseModeParams(
    VAPoR::ParamsBase::StateSave *ssave
 );

 //! Create a MouseModeParams object from an existing XmlNode tree
 //
 MouseModeParams(
    VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node
 );


 virtual ~MouseModeParams();
	
 //! method identifies pixmap icon for each mode
 const char* const * GetIcon(string name) const {
 	map <string, const char *const  *>::const_iterator itr;
	itr = _iconMap.find(name);
	assert(itr != _iconMap.end());
	return itr->second;
 }


 //! Method to register a mouse mode.  Called during startup.
 //! \param[in] name name of the mode, identifying it during selection
 //! \param[in] modeType An integer type identify associated with \p name
 //! \param[in] xpmIcon (as an xpm) used when displaying mode in GUI.
 //!
 void RegisterMouseMode(
	string name, int modeType, const char* const xpmIcon[]
 );


 //! method that indicates the manipulator type that is associated with a 
 //! mouse mode.
 //! \param[in] name A valid mode name
 //! \retval int manipulator type 
 //
 int GetModeManipType(string name) const {
	map <string, int>::const_iterator itr;
	itr = _typeMap.find(name);
	assert(itr != _typeMap.end());
	return itr->second;
 }




 //! method indicates the current mouse mode
 //! \retval current mouse mode
 //
 string GetCurrentMouseMode() const {
	return GetValueString(_currentMouseModeTag, GetNavigateModeName());
 }

 //! method sets the current mouse mode
 //!
 //! \param[in] name  current mouse mode
 //
 void SetCurrentMouseMode(string name);

 //! method indicates how many mouse modes are available.
 int GetNumMouseModes() {
	return _typeMap.size();
 }

 //! Return a vector of all registered mouse mode names
 //
 vector <string> GetAllMouseModes() {
	vector <string> v;
	map <string, int>::const_iterator itr;
	for (itr = _typeMap.begin(); itr != _typeMap.end(); ++itr) {
		v.push_back(itr->first);
	}
	return(v);
 }


 // Get static string identifier for this params class
 //
 static string GetClassType() {
	return("MouseModeParamsTag");
 }

 static string GetNavigateModeName() {
	return("Navigate");
 }

private:

 static const string _currentMouseModeTag;

 map <string, int> _typeMap;
 map <string, const char * const  *> _iconMap;

 void _init();
 void _setUpDefault();


};

#endif //MOUSEMODEPARAMS_H 
