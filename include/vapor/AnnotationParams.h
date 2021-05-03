//************************************************************************
//									*
//			 Copyright (C)  2015				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnnotationParams.h
//
//	Author:	Scott Pearse
//			Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Defines the AnnotationParams class.
//		This class supports parameters associted with the
//		vizfeature panel, describing the visual features in the visualizer
//
#ifndef ANNOTATIONPARAMS_H
#define ANNOTATIONPARAMS_H

#include <vector>

#include <vapor/common.h>
#include <vapor/ParamsBase.h>
#include <vapor/AxisAnnotation.h>

using namespace Wasp;

namespace VAPoR {

class XmlNode;

//! \class AnnotationParams
//! \ingroup Public_Params
//! \brief A class for describing visual features displayed in the visualizer.
//! \author Alan Norton
//! \version 3.0
//! \date	June 2015

//! The AnnotationParams class controls various features displayed in the visualizers
//! There is a global AnnotationParams, that
//! is shared by all windows whose vizfeature is set to "global".  There is also
//! a local AnnotationParams for each window, that users can select whenever there are multiple windows.
//! When local settings are used, they only affect one currently active visualizer.
//! The AnnotationParams class also has several methods that are useful in setting up data requests from the DataMgr.
//!
class PARAMS_API AnnotationParams : public ParamsBase {
public:
    //! Create a AnnotationParams object from scratch
    //
    AnnotationParams(ParamsBase::StateSave *ssave);

    //! Create a AnnotationParams object from an existing XmlNode tree
    //
    AnnotationParams(ParamsBase::StateSave *ssave, XmlNode *node);

    //! Copy from already existing instance
    //
    AnnotationParams(const AnnotationParams &rhs);

    virtual ~AnnotationParams(){};

    void GetDomainColor(double color[3]) const;
    void GetDomainColor(std::vector<double> &color) const { _getColor(color, _domainColorTag); }

    void SetDomainColor(vector<double> color);

    bool GetUseDomainFrame() const { return (0 != GetValueLong(_domainFrameTag, (long)false)); }
    void SetUseDomainFrame(bool onOff) { SetValueLong(_domainFrameTag, "toggle domain frame", (long)onOff); }

    bool GetUseRegionFrame() const { return (0 != GetValueLong(_regionFrameTag, (long)false)); }
    void SetUseRegionFrame(bool onOff) { SetValueLong(_regionFrameTag, "toggle region frame", (long)onOff); }

    void GetRegionColor(double color[3]) const;
    void GetRegionColor(std::vector<double> &color) const { _getColor(color, _regionColorTag); }

    void SetRegionColor(vector<double> color);

    void GetBackgroundColor(double color[3]) const;
    void GetBackgroundColor(std::vector<double> &color) const { _getColor(color, _backgroundColorTag); }

    void SetBackgroundColor(std::vector<double> color);

    string GetCurrentAxisDataMgrName() const;
    void   SetCurrentAxisDataMgrName(string dataMgr = "default");

    AxisAnnotation *GetAxisAnnotation();

    void SetAxisFontSize(int size);
    int  GetAxisFontSize();

    double GetTimeLLX() const;
    void   SetTimeLLX(double llx);

    double GetTimeLLY() const;
    void   SetTimeLLY(double lly);

    std::vector<double>       GetTimeColor() const;
    template<typename T> void GetTimeColor(T color[]) const { m_getColor(color, _timeColorTag); };
    void                      SetTimeColor(std::vector<double> color);

    int  GetTimeType() const;
    void SetTimeType(int type);

    int  GetTimeSize() const;
    void SetTimeSize(int size);


    bool   GetAxisArrowEnabled() const;
    double GetAxisArrowSize() const;
    double GetAxisArrowXPos() const;
    double GetAxisArrowYPos() const;

    void SetAxisArrowEnabled(bool enabled);
    void SetAxisArrowSize(double pos);
    void SetAxisArrowXPos(double pos);
    void SetAxisArrowYPos(double pos);

    static string GetClassType() { return ("AnnotationParams"); }

    static const string AxisArrowSizeTag;
    static const string AxisArrowXPosTag;
    static const string AxisArrowYPosTag;
    static const string AxisArrowEnabledTag;

private:
    ParamsContainer *_axisAnnotations;

public:
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

    static const string   _currentAxisDataMgrTag;
    static vector<double> _previousStretch;

    static const string _timeLLXTag;
    static const string _timeLLYTag;
    static const string _timeColorTag;
    static const string _timeTypeTag;
    static const string _timeSizeTag;

    static const string _projStringTag;

private:
    void _init();

    template<typename T> void m_getColor(T color[3], string tag) const
    {
        vector<double> defaultv(3, 1.0);
        vector<double> val = GetValueDoubleVec(tag, defaultv);
        for (int i = 0; i < val.size(); i++) {
            color[i] = val[i];
            if (color[i] < 0.0) color[i] = 0.0;
            if (color[i] > 1.0) color[i] = 1.0;
        }
    }
    // void m_getColor(double color[3], string tag) const;
    void _getColor(vector<double> &color, string tag) const;
    void m_setColor(vector<double> color, string tag, string msg);
};

};        // namespace VAPoR
#endif    // ANNOTATIONPARAMS_H
