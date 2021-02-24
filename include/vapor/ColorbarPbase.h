//************************************************************************
//																*
//		     Copyright (C)  2016								*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved								*
//																*
//************************************************************************/
//
//	File:		ColorbarPbase.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016
//
//	Description:	Defines the ColorbarPbase class, derived from ParamsBase.
//		This supports parameters controlling the display of color bars.
//
#ifndef COLORBARPBASE_H
#define COLORBARPBASE_H

#include <vapor/ParamsBase.h>

namespace VAPoR {

//! \class ColorbarPbase
//! \ingroup Public_Params
//! \brief Settings for color bar displayed in scene
//! Intended to be used in any Params class
//! \author Alan Norton
//! \version 3.0
//! \date   February 2016

//! The ColorbarPbase class is a ParamsBase class that manages the settings associated with a color bar.
//! There is a corresponding ColorbarFrame class in the GUI that manages display of these settings.
//!
//!
//!
class PARAMS_API ColorbarPbase : public ParamsBase {
public:
    //! Create a ColorbarPbase object from scratch
    //
    ColorbarPbase(ParamsBase::StateSave *ssave);

    //! Create a ColorbarPbase object from an existing XmlNode tree
    //
    ColorbarPbase(ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~ColorbarPbase();

    //! Get the X,Y corner (upper left) coordinates
    //! Relative to [0,1]
    //! \retval pair of x,y coordinates
    vector<double> GetCornerPosition() const;

    //! Set the X,Y corner (upper left) coordinates
    //! Relative to [0,1]
    //! \param[in] posn = x,y coordinates
    void SetCornerPosition(vector<double> posn);

    //! Get the X,Y size
    //! Relative to [0,1]
    //! \retval pair of x,y sizes
    vector<double> GetSize() const;

    //! Set the X,Y sizes
    //! Relative to [0,1]
    //! \param[in] posn = x,y sizes
    void SetSize(vector<double> sz);

    //! Get the title text
    //! (displayed after variable name)
    //! \retval title
    string GetTitle() const { return GetValueString(_colorbarTitleTag, ""); }

    //! Set the title text
    //! \param[in] text to display
    void SetTitle(string text) { SetValueString(_colorbarTitleTag, "Set colorbar title", text); }

    //! Determine if colorbar is enabled
    //! \return true if enabled
    bool IsEnabled() const { return (GetValueLong(_colorbarEnabledTag, (long)false) != 0); }

    //! Enable or disable colorbar
    //! \param[in] bool true if enabled
    void SetEnabled(bool val) { SetValueLong(_colorbarEnabledTag, "enable/disable colorbar", val); }

    //! Determine colorbar text size
    //! \return pointsize
    long GetFontSize() const;

    //! Set colorbar text size
    //! \param[in] val text point size
    void SetFontSize(long val);

    //! Determine colorbar num tics
    //! \return number of tics
    long GetNumTicks() const;

    //! Set colorbar number of tic marks
    //! \param[in] val number of tics
    void SetNumTicks(long val);

    //! Determine colorbar num digits to display
    //! \return number of digits
    long GetNumDigits() const;

    //! Set colorbar number of digits
    //! \param[in] val number of digits
    void SetNumDigits(long val);

    //! Get the background color
    //! as an rgb triple
    //! \retval rgb color
    vector<double> GetBackgroundColor() const;

    //! Set the background color as an rgb triple
    //! \param[in] color = (r,g,b)
    void SetBackgroundColor(vector<double> color);

    //! Copy the settings (except enablement, title, and position) to another ColobarPbase.
    //! \param[in] target ColorbarPbase that is target of the copy.
    void copyTo(ColorbarPbase *target);

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("ColorBarSettingParams"); }

public:
    static const string _colorbarBackColorTag;
    static const string _colorbarSizeXTag;
    static const string _colorbarSizeYTag;
    static const string _colorbarPositionXTag;
    static const string _colorbarPositionYTag;
    static const string _colorbarFontSizeTag;
    static const string _colorbarNumDigitsTag;
    static const string _colorbarTitleTag;
    static const string _colorbarNumTicksTag;
    static const string _colorbarEnabledTag;

    static const string UseScientificNotationTag;
};
};        // namespace VAPoR
#endif    // COLORBARPBASE_H
