//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnnotationsParams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Defines the AnnotationsParams class.
//		This class supports parameters associted with the
//		vizfeature panel, describing the visual features in the visualizer
//
#ifndef ANNOTATIONSPARAMS_H
#define ANNOTATIONSPARAMS_H

#include <vector>

#include <vapor/common.h>
#include <vapor/ParamsBase.h>
#include <vapor/AxisAnnotation.h>

using namespace Wasp;

namespace VAPoR {

class XmlNode;

//! \class AnnotationsParams
//! \ingroup Public_Params
//! \brief A class for describing visual features displayed in the visualizer.
//! \author Alan Norton
//! \version 3.0
//! \date    June 2015

//! The AnnotationsParams class controls various features displayed in the visualizers
//! There is a global AnnotationsParams, that
//! is shared by all windows whose vizfeature is set to "global".  There is also
//! a local AnnotationsParams for each window, that users can select whenever there are multiple windows.
//! When local settings are used, they only affect one currently active visualizer.
//! The AnnotationsParams class also has several methods that are useful in setting up data requests from the DataMgr.
//!
class PARAMS_API AnnotationsParams : public ParamsBase {
public:
    //! Create a AnnotationsParams object from scratch
    //
    AnnotationsParams(ParamsBase::StateSave *ssave);

    //! Create a AnnotationsParams object from an existing XmlNode tree
    //
    AnnotationsParams(ParamsBase::StateSave *ssave, XmlNode *node);

    //! Copy from already existing instance
    //
    AnnotationsParams(const AnnotationsParams &rhs);

    virtual ~AnnotationsParams(){};

    //! Obtain domain frame color
    void GetDomainColor(double color[3]) const;
    void GetDomainColor(std::vector<double> &color) const { _getColor(color, _domainColorTag); }

    //! Set domain frame color
    void SetDomainColor(vector<double> color);

    bool GetUseDomainFrame() const { return (0 != GetValueLong(_domainFrameTag, (long)false)); }
    void SetUseDomainFrame(bool onOff) { SetValueLong(_domainFrameTag, "toggle domain frame", (long)onOff); }

    bool GetUseRegionFrame() const { return (0 != GetValueLong(_regionFrameTag, (long)false)); }
    void SetUseRegionFrame(bool onOff) { SetValueLong(_regionFrameTag, "toggle region frame", (long)onOff); }

    //! Obtain region frame color
    void GetRegionColor(double color[3]) const;
    void GetRegionColor(std::vector<double> &color) const { _getColor(color, _regionColorTag); }

    //! Set region frame color
    void SetRegionColor(vector<double> color);

    //! Obtain background color
    void GetBackgroundColor(double color[3]) const;
    void GetBackgroundColor(std::vector<double> &color) const { _getColor(color, _backgroundColorTag); }

    //! Set background color
    void SetBackgroundColor(std::vector<double> color);

    string GetCurrentAxisDataMgrName() const;
    void   SetCurrentAxisDataMgrName(string dataMgr);

    AxisAnnotation *GetAxisAnnotation(string dataMgr = "");

    void                SetAxisArrowCoords(std::vector<double> coords);
    std::vector<double> GetAxisArrowCoords() const;

    bool GetShowAxisArrows() const;
    void SetShowAxisArrows(bool val);

    void SetAxisFontSize(int size);
    int  GetAxisFontSize();

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("AnnotationsParams"); }

private:
#ifdef VAPOR3_0_0_ALPHA
    static void changeStretch(vector<double> prevStretch, vector<double> newStretch);
#endif

    ParamsContainer *_axisAnnotations;

    static const string _domainColorTag;
    static const string _domainFrameTag;
    static const string _regionFrameTag;
    static const string _regionColorTag;

    static const string _axisColorTag;
    static const string _axisDigitsTag;
    static const string _axisTextHeightTag;
    static const string _axisFontSizeTag;
    static const string _ticWidthTag;
    static const string _ticDirsTag;
    static const string _ticSizeTag;
    static const string _minTicsTag;
    static const string _maxTicsTag;
    static const string _numTicsTag;
    static const string _axisOriginTag;
    static const string _backgroundColorTag;
    static const string _axisAnnotationEnabledTag;
    static const string _axisAnnotationsTag;
    static const string _latLonAxesTag;

    static const string   _axisArrowCoordsTag;
    static const string   _showAxisArrowsTag;
    static const string   _currentAxisDataMgrTag;
    static vector<double> _previousStretch;

    void _init();

    void m_getColor(double color[3], string tag) const;
    void _getColor(vector<double> &color, string tag) const;
    void m_setColor(vector<double> color, string tag, string msg);
};

};        // namespace VAPoR
#endif    // ANNOTATIONSPARAMS_H
