//--OpacityMap.h ---------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// Various types of mappings from opacity to data value.
//
//----------------------------------------------------------------------------

#ifndef OpacityMap_H
#define OpacityMap_H

#include <iostream>
#include <vapor/ParamsBase.h>
#include <vapor/TFInterpolator.h>

#ifdef WIN32
    #include <vapor/glutil.h>
#endif

namespace VAPoR {

class PARAMS_API OpacityMap : public ParamsBase {
public:
    enum Type { CONTROL_POINT, GAUSSIAN, INVERTED_GAUSSIAN, SINE };

    //! Create a OpacityMap object from scratch
    //
    OpacityMap(ParamsBase::StateSave *ssave);

    //! Create a OpacityMap object from an existing XmlNode tree
    //
    OpacityMap(ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~OpacityMap();
    void clear();

    float opacityData(float value) const;
    bool  inDataBounds(float value) const;

    void SetType(OpacityMap::Type type);

    OpacityMap::Type GetType() const { return (OpacityMap::Type)GetValueLong(_typeTag, CONTROL_POINT); }

    void           SetDataBounds(const vector<double> &bounds);
    vector<double> GetDataBounds() const;

    double minValue() const;
    double maxValue() const;

    void setMinValue(double val);
    void setMaxValue(double val);

    bool   IsEnabled() { return (GetValueLong(_enabledTag, 0) != 0 ? true : false); }
    void   SetEnabled(bool enabled);
    double GetMean() const { return GetValueDouble(_meanTag, 0.5); }
    void   SetMean(double mean);
    double GetSSQ() const { return GetValueDouble(_ssqTag, 0.1); }
    void   SetSSQ(double ssq);
    double GetFreq() const { return GetValueDouble(_freqTag, 5.0); }
    void   SetFreq(double freq);
    double GetPhase() const { return GetValueDouble(_phaseTag, 2 * M_PI); }
    void   SetPhase(double phase);

    int numControlPoints() { return (int)GetControlPoints().size() / 2; }

    void addNormControlPoint(float normv, float opacity);    // Normalized Coords
    void addControlPoint(float value, float opacity);        // Data Coordinates
    void deleteControlPoint(int index);
    void moveControlPoint(int index, float dx, float dy);    // Data Coordinates

    float controlPointOpacity(int index) const;
    void  controlPointOpacity(int index, float opacity);

    float controlPointValue(int index) const;           // Data Coordinates
    void  controlPointValue(int index, float value);    // Data Coordinates

    void setOpaque();
    bool isOpaque() const;

    void SetInterpType(TFInterpolator::type t);

    TFInterpolator::type GetInterpType() const { return (TFInterpolator::type)GetValueLong(_interpTypeTag, TFInterpolator::linear); }

    vector<double> GetControlPoints() const;
    void           SetControlPoints(const vector<double> &opacityControlPoints);

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("OpacityMapParams"); }

private:
    int leftControlIndex(float val) const;

    double normSSq(double ssq);
    double denormSSq(double ssq);

    double normSineFreq(double freq);
    double denormSineFreq(double freq);

    double normSinePhase(double phase);
    double denormSinePhase(double phase);

    const double _minSSq;
    const double _maxSSq;
    const double _minFreq;
    const double _maxFreq;
    const double _minPhase;
    const double _maxPhase;

    static const string _relMinTag;
    static const string _relMaxTag;
    static const string _enabledTag;
    static const string _meanTag;
    static const string _ssqTag;
    static const string _freqTag;
    static const string _phaseTag;
    static const string _typeTag;
    static const string _controlPointsTag;
    static const string _interpTypeTag;
    static const string _opacityMapIndexTag;
    static const string _dataBoundsTag;
};
};    // namespace VAPoR

#endif    // OpacityMap_H
