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
    struct MouseMode {
        string             name;
        const char *const *icon;
    };

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
        auto itr = _modes.cbegin();
        for (; itr != _modes.cend(); ++itr)
            if (itr->name == name) break;
        assert(itr != _modes.end());
        return itr->icon;
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
    int GetNumMouseModes() { return _modes.size(); }

    //! Return a vector of all registered mouse mode names
    //
    vector<string> GetAllMouseModes()
    {
        vector<string> v;
        for (auto itr = _modes.cbegin(); itr != _modes.cend(); ++itr) { v.push_back(itr->name); }
        return (v);
    }

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("MouseModeParamsTag"); }

    static string GetNavigateModeName() { return ("Navigate"); }

    static string GetRegionModeName() { return ("Region"); }

    static string GetGeoRefModeName() { return ("Geo Referenced"); }

private:
    static const string _currentMouseModeTag;

    vector<MouseMode> _modes;

    void _init();
    void _setUpDefault();
};

#endif    // MOUSEMODEPARAMS_H
