//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		StartupParams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2015
//
//	Description:	Defines the StartupParams class.
//		This class supports parameters associted with the
//		Startup panel, describing the startup settings of the app.
//
#ifndef STARTUPPARAMS_H
#define STARTUPPARAMS_H


#include <vector>
#include <vapor/ParamsBase.h>

//! \class StartupParams
//! \ingroup Public_Params
//! \brief A class for describing settings at startup.
//! \author Alan Norton
//! \version 3.0
//! \date    November 2015

//! The StartupParams class controls various features set when the application starts.
//! There is only a global StartupParams, that 
//! is used throughout the application
//!
class StartupParams : public VAPoR::ParamsBase {
	
public: 

 StartupParams(
	VAPoR::ParamsBase::StateSave *ssave
 );


 StartupParams(
	VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node
 );

 //! Destructor
 ~StartupParams();


 //! Provide a short name suitable for use in the GUI tab
 //! \retval string name
 const std::string getShortName() {return _shortName;}

 //! Obtain the default lod fidelity and default refinement fidelity for 
 //! 2D variables.
 //! First value is LOD, second is Refinement
 //! \return vector of default fidelity for 2D 
 vector <long> GetFidelityDefault2D() const {
	vector <long> defaultv(2,2);
	return GetValueLongVec(_fidelityDefault2DTag, defaultv);
 }

 //! Obtain the default lod fidelity and default refinement fidelity 
 //! for 3D variables.
 //! First value is LOD, second is Refinement
 //! \return vector of default fidelity for 3D 
 vector <long> GetFidelityDefault3D() const {
	vector <long> defaultv(2,2);
	return GetValueLongVec(_fidelityDefault3DTag);
 }

 //! Set the 2d fidelity defaults
 //! \param[in] lodDef Default LOD factor for 2D
 //! \param[in] refDef Default Refinement factor for 2D
 void SetFidelityDefault2D(long lodDef, long refDef){
	vector <long> val;
	val.push_back(lodDef);
	val.push_back(refDef);
	SetValueLongVec(_fidelityDefault2DTag, "Set fidelity 2D default", val);
}

 //! Set the 3d fidelity defaults
 //! \param[in] lodDef Default LOD factor for 3d
 //! \param[in] refDef Default Refinement factor for 3D
 void SetFidelityDefault3D(long lodDef, long refDef){
	vector <long> val;
	val.push_back(lodDef);
	val.push_back(refDef);
	SetValueLongVec(_fidelityDefault3DTag, "Set fidelity 3D default", val);
 }

	bool GetAutoStretch() const {
		return (0!= GetValueLong(_autoStretchTag, (long) false));
	}

	void SetAutoStretch(bool val) {
		SetValueLong(_autoStretchTag,"Enable Auto Stretch", val);
	}

	long GetCacheMB() const {
		long val = GetValueLong(_cacheMBTag,1000);
		if (val < 0) val = 1000;
		return(val);
	}

	void SetCacheMB(long val){
		if (val < 0) val = 1000;
		SetValueLong(_cacheMBTag, "Set cache size", val);
	}

	long GetTextureSize() const {
		long val =  GetValueLong(_texSizeTag,0);
		if (val < 0) val = 0;
		return(val);
	}

	void SetTextureSize(long val){
		if (val < 0) val = 0;
		SetValueLong(_texSizeTag, "Set graphic tex size", val);
	}

	//! Enable or disable tex size setting
	void SetTexSizeEnable(bool val){
		SetValueLong(_texSizeEnableTag, "toggle enable texture size", (long)val);
	}

	//! query axis annotation enabled
	bool GetTexSizeEnable() const {
		return (0 != GetValueLong(_texSizeEnableTag,(long) 0));
	}

	//! Lock or unlock window size
	void SetWinSizeLock(bool val){
		SetValueLong(_winSizeLockTag, "toggle lock window size", (long)val);
	}

	//! query window size locking
	bool GetWinSizeLock() const {
		return (0 != GetValueLong(_winSizeLockTag, (long) false));
	}

	//! Set window size 
	void SetWinSize(size_t width, size_t height);

	//! Get window size 
	void GetWinSize(size_t &width, size_t &height) const;
	
	string GetSessionDir() const {
		return GetValueString(_sessionDirTag,string("."));
	}

	void SetSessionDir(string name){
		SetValueString(_sessionDirTag,"set session directory", name);
	}

	string GetMetadataDir() const {
		return GetValueString(_metadataDirTag,string("."));
	}

	void SetMetadataDir(string dir){
		SetValueString(_metadataDirTag,"set metadata directory", dir);
	}

	string GetImageDir() const {
		return GetValueString(_imageDirTag,string("."));
	}
 
	void SetImageDir(string dir){
		SetValueString(_imageDirTag,"set image directory", dir);
	}
 
	string GetTFDir() const {
		return GetValueString(_tfDirTag,string("."));
	}
 
	void SetTFDir(string dir){
		SetValueString(_tfDirTag,"set trans function directory", dir);
	}

	string GetFlowDir() const {
		return GetValueString(_flowDirTag,string("."));
	}

	void SetFlowDir(string dir){
		SetValueString(_flowDirTag,"set flow save directory", dir);
	}

	string GetPythonDir() const {
		return GetValueString(_pythonDirTag,string("."));
	}

	void SetPythonDir(string dir){
		SetValueString(_pythonDirTag,"set python directory", dir);
	}

	string GetCurrentPrefsPath() const {
		string defaultv = "/tmp/.vapor3_prefs";
		return GetValueString(_currentPrefsPathTag, defaultv);
	}

	void SetCurrentPrefsPath(string pth){
		// This is never captured in undo/redo
		SetValueString(_currentPrefsPathTag,"set current preference path", pth);
	}

	size_t GetNumExecutionThreads() const {
		long val = GetValueLong(_numExecutionThreads, 0);
		if (val < 0) val = 0;
		return((size_t) val);
	}

	void SetNumExecutionThreads(size_t val) {
		SetValueLong(_numExecutionThreads, "Number of execution threads", 0);
	}

#ifdef DEAD
	//! Copy all the paths in the startup settings to the current PathParams (so they will be in the current session).
	void CopyPathsToSession();
	
#endif

 // Get static string identifier for this params class
 //
 static string GetClassType() {
	return(m_classType);
 }
	
	
private:
	
	static const string m_classType;
	static const string _shortName;
	static const string _cacheMBTag;
	static const string _texSizeTag;
	static const string _texSizeEnableTag;
	static const string _winSizeTag;
	static const string _winSizeLockTag;
	static const string _currentPrefsPathTag;
	static const string _sessionDirTag;
	static const string _metadataDirTag;
	static const string _imageDirTag;
	static const string _tfDirTag;
	static const string _flowDirTag;
	static const string _pythonDirTag;
	static const string _fidelityDefault2DTag;
	static const string _fidelityDefault3DTag;
	static const string _autoStretchTag;
	static const string _numExecutionThreads;
	
	void _init();
};

#endif //STARTUPPARAMS_H 
