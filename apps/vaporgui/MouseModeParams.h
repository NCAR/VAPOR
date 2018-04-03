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
class MouseModeParams : public VAPoR::ParamsBase {
public:
    //! Create a MouseModeParams object from scratch
    //
    MouseModeParams(VAPoR::ParamsBase::StateSave *ssave);

    //! Create a MouseModeParams object from an existing XmlNode tree
    //
    MouseModeParams(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node);

    virtual ~MouseModeParams();

    //! method identifies pixmap icon for each mode
    const char *const *GetIcon(string name) const
    {
        map<string, const char *const *>::const_iterator itr;
        itr = _iconMap.find(name);
        assert(itr != _iconMap.end());
        return itr->second;
    }

    //! Method to register a mouse mode.  Called during startup.
    //! \param[in] name name of the mode, identifying it during selection
    //! \param[in] modeType An integer type identify associated with \p name
    //! \param[in] xpmIcon (as an xpm) used when displaying mode in GUI.
    //!
    void RegisterMouseMode(string name, int modeType, const char *const xpmIcon[]);

    //! method that indicates the manipulator type that is associated with a
    //! mouse mode.
    //! \param[in] name A valid mode name
    //! \retval int manipulator type
    //
    int GetModeManipType(string name) const
    {
        map<string, int>::const_iterator itr;
        itr = _typeMap.find(name);
        assert(itr != _typeMap.end());
        return itr->second;
    }

    //! method indicates the current mouse mode
    //! \retval current mouse mode
    //
    string GetCurrentMouseMode() const { return GetValueString(_currentMouseModeTag, GetNavigateModeName()); }

    //! method sets the current mouse mode
    //!
    //! \param[in] name  current mouse mode
    //
    void SetCurrentMouseMode(string name);

    //! method indicates how many mouse modes are available.
    int GetNumMouseModes() { return _typeMap.size(); }

    //! Return a vector of all registered mouse mode names
    //
    vector<string> GetAllMouseModes()
    {
        vector<string>                   v;
        map<string, int>::const_iterator itr;
        for (itr = _typeMap.begin(); itr != _typeMap.end(); ++itr) { v.push_back(itr->first); }
        return (v);
    }

    //! Get current camera position
    //! \param[out] position 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraPos(double position[3]) const
    {
        double def[3] = {0.0, 0.0, 5.0};
        _get3(_positionVectorTag, def, position);
    }

    void SetCameraPos(const double position[3]) { _set3(_positionVectorTag, "Set camera position", position); }

    //! Get current camera normalized view direction
    //! \param[out] viewDir 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraViewDir(double viewDir[3]) const
    {
        double def[3] = {0.0, 0.0, -1.0};
        _get3(_directionVectorTag, def, viewDir);
    }

    void SetCameraViewDir(const double viewDir[3]) { _set3(_directionVectorTag, "Set camera view direction", viewDir); }

    //! Get current camera normalized up direction
    //! \param[out] upVec 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraUpVec(double upVec[3]) const
    {
        double def[3] = {0.0, 1.0, 0.0};
        _get3(_upVectorTag, def, upVec);
    }

    void SetCameraUpVec(const double upVec[3]) { _set3(_upVectorTag, "Set camera up vector", upVec); }

    //! Obtain rotation center in local coordinates
    //! \param[out] center 3-element vector with center of rotation
    //
    void GetRotationCenter(double center[3]) const
    {
        double def[3] = {0.0, 0.0, 0.0};
        _get3(_rotCenterTag, def, center);
    }

    //! Specify rotation center in local coordinates
    //! \param[in] vector<double>& rotation center in local coordinates
    //! \retval int 0 if successful
    void SetRotationCenter(const double center[3]) { _set3(_rotCenterTag, "Set rotation center", center); }

    //! Get home camera position
    //! \param[out] position 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraPosHome(double position[3]) const
    {
        double def[3] = {0.0, 0.0, 5.0};
        _get3(_positionVectorHomeTag, def, position);
    }

    void SetCameraPosHome(const double position[3]) { _set3(_positionVectorHomeTag, "Set camera position home", position); }

    //! Get home camera normalized view direction
    //! \param[out] viewDir 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraViewDirHome(double viewDir[3]) const
    {
        double def[3] = {0.0, 0.0, -1.0};
        _get3(_directionVectorHomeTag, def, viewDir);
    }

    void SetCameraViewDirHome(const double viewDir[3]) { _set3(_directionVectorHomeTag, "Set camera view direction home", viewDir); }

    //! Get home camera normalized up direction
    //! \param[out] upVec 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraUpVecHome(double upVec[3]) const
    {
        double def[3] = {0.0, 1.0, 0.0};
        _get3(_upVectorHomeTag, def, upVec);
    }

    void SetCameraUpVecHome(const double upVec[3]) { _set3(_upVectorHomeTag, "Set camera up vector home", upVec); }

    //! Obtain home rotation center in local coordinates
    //! \param[out] center 3-element vector with center of rotation
    //
    void GetRotationCenterHome(double center[3]) const
    {
        double def[3] = {0.0, 0.0, 0.0};
        _get3(_rotCenterHomeTag, def, center);
    }

    //! Specify home rotation center in local coordinates
    //! \param[in] vector<double>& rotation center in local coordinates
    //! \retval int 0 if successful
    void SetRotationCenterHome(const double center[3]) { _set3(_rotCenterHomeTag, "Set rotation center home", center); }

    void SetCameraToHome();
    void SetHomeToCamera();

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("MouseModeParamsTag"); }

    static string GetNavigateModeName() { return ("Navigate"); }

    static string GetRegionModeName() { return ("Region"); }

private:
    static const string _currentMouseModeTag;
    static const string _rotCenterTag;
    static const string _positionVectorTag;
    static const string _upVectorTag;
    static const string _directionVectorTag;
    static const string _rotCenterHomeTag;
    static const string _positionVectorHomeTag;
    static const string _upVectorHomeTag;
    static const string _directionVectorHomeTag;

    map<string, int>                 _typeMap;
    map<string, const char *const *> _iconMap;

    void _init();
    void _setUpDefault();

    void _get3(string tag, const double def[3], double v[3]) const
    {
        vector<double> defaultv = {def[0], def[1], def[2]};
        vector<double> vec = GetValueDoubleVec(tag, defaultv);
        assert(vec.size() == 3);
        v[0] = vec[0];
        v[1] = vec[1];
        v[2] = vec[2];
    }

    void _set3(string tag, string msg, const double v[3])
    {
        vector<double> vec;
        vec.push_back(v[0]);
        vec.push_back(v[1]);
        vec.push_back(v[2]);
        SetValueDoubleVec(tag, msg, vec);
    }
};

#endif    // MOUSEMODEPARAMS_H
