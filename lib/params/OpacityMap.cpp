//--OpacityMap.cpp -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// Various types of mappings from opacity to data value.
//
//----------------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <algorithm>

#include <vapor/OpacityMap.h>

#ifndef MAX
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef M_PI
    #define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_E
    #define M_E 2.71828182845904523536
#endif

using namespace std;
using namespace VAPoR;
using namespace Wasp;

//----------------------------------------------------------------------------
// Static member initalization
//----------------------------------------------------------------------------
const string OpacityMap::_relMinTag = "RelMinValue";
const string OpacityMap::_relMaxTag = "RelMaxValue";
const string OpacityMap::_enabledTag = "Enabled";
const string OpacityMap::_meanTag = "Mean";
const string OpacityMap::_ssqTag = "SSq";
const string OpacityMap::_freqTag = "Freq";
const string OpacityMap::_phaseTag = "Phase";
const string OpacityMap::_typeTag = "Type";
const string OpacityMap::_controlPointsTag = "OpacityMapControlPoints";
const string OpacityMap::_interpTypeTag = "OpacityInterpolation";
const string OpacityMap::_opacityMapIndexTag = "OpacityMapIndex";
const string OpacityMap::_dataBoundsTag = "DataBounds";

//
// Register class with object factory!!!
//
static ParamsRegistrar<OpacityMap> registrar(OpacityMap::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

OpacityMap::OpacityMap(ParamsBase::StateSave *ssave) : ParamsBase(ssave, OpacityMap::GetClassType()), _minSSq(0.0001), _maxSSq(1.0), _minFreq(1.0), _maxFreq(30.0), _minPhase(0.0), _maxPhase(2 * M_PI)
{
    SetInterpType(TFInterpolator::linear);
    SetType(CONTROL_POINT);
    SetEnabled(true);
    SetMean(0.5);
    SetSSQ(0.1);
    SetFreq(5.0);
    SetPhase(2. * M_PI);

    vector<double> bounds(2, 0.0);

    SetDataBounds(bounds);

    setMinValue(bounds[0]);
    setMaxValue(bounds[1]);
    vector<double> cps;
    for (int i = 0; i < 4; i++) {
        cps.push_back(1.f);
        cps.push_back((double)i / 3.);
        //	  cps.push_back((double)i/3.);
        //	  cps.push_back((double)i/3.);
    }
    SetControlPoints(cps);
}

OpacityMap::OpacityMap(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node), _minSSq(0.0001), _maxSSq(1.0), _minFreq(1.0), _maxFreq(30.0), _minPhase(0.0), _maxPhase(2 * M_PI) {}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
OpacityMap::~OpacityMap() {}

//----------------------------------------------------------------------------
// Clear  the control points
//----------------------------------------------------------------------------
void OpacityMap::clear()
{
    vector<double> cps;
    SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Add a new control point to the opacity map.
//----------------------------------------------------------------------------
void OpacityMap::addNormControlPoint(float normVal, float opacity)
{
    vector<double> cps = GetControlPoints();
    // cps.insert position is before where new element goes, so must add 2
    int index = leftControlIndex(normVal) * 2 + 2;
    cps.insert(cps.begin() + index++, opacity);
    cps.insert(cps.begin() + index, normVal);
    SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Add a new control point to the opacity map.
//----------------------------------------------------------------------------
void OpacityMap::addControlPoint(float value, float opacity)
{
    float normVal = (value - minValue()) / (maxValue() - minValue());
    addNormControlPoint(normVal, opacity);
}

//----------------------------------------------------------------------------
// Delete the control point.
//----------------------------------------------------------------------------
void OpacityMap::deleteControlPoint(int index)
{
    vector<double> cps = GetControlPoints();
    if (index >= 0 && index < cps.size() / 2 - 1 && cps.size() > 2) {
        cps.erase(cps.begin() + 2 * index, cps.begin() + 2 * index + 1);
        SetControlPoints(cps);
    }
}

//----------------------------------------------------------------------------
// Move the control point.
//----------------------------------------------------------------------------
void OpacityMap::moveControlPoint(int index, float dx, float dy)
{
    vector<double> cps = GetControlPoints();
    if (index >= 0 && index <= cps.size() / 2 - 1) {
        float minVal = 0.0;
        float maxVal = 1.0;
        float ndx = dx / (maxValue() - minValue());

        if (index != 0) { minVal = cps[2 * index - 1]; }

        if (index < cps.size() / 2 - 1) { maxVal = cps[2 * index + 3]; }

        float value = cps[2 * index + 1] + ndx;

        if (value < minVal) {
            value = minVal;
        } else if (value > maxVal) {
            value = maxVal;
        }

        cps[2 * index + 1] = value;

        float opacity = cps[2 * index] + dy;

        if (opacity < 0.0) {
            opacity = 0.0;
        } else if (opacity > 1.0) {
            opacity = 1.0;
        }

        cps[2 * index] = opacity;
    }
    SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Return the control point's opacity.
//----------------------------------------------------------------------------
float OpacityMap::controlPointOpacity(int index) const
{
    if (index < 0) { return 0.0; }
    vector<double> cps = GetControlPoints();
    if (index > cps.size() / 2 - 1) { return 1.0; }

    return cps[2 * index];
}

//----------------------------------------------------------------------------
// Set the control point's opacity.
//----------------------------------------------------------------------------
void OpacityMap::controlPointOpacity(int index, float opacity)
{
    vector<double> cps = GetControlPoints();
    if (index < 0 || 2 * index >= cps.size()) return;

    if (opacity < 0.0)
        opacity = 0.;
    else if (opacity > 1.0)
        opacity = 1.;

    cps[index * 2] = opacity;
    SetControlPoints(cps);
}

//----------------------------------------------------------------------------
// Return the control point's value (in data coordinates).
//----------------------------------------------------------------------------
float OpacityMap::controlPointValue(int index) const
{
    if (index < 0) { return minValue(); }
    vector<double> cps = GetControlPoints();
    if (2 * index + 1 >= cps.size()) { return maxValue(); }

    float norm = cps[2 * index + 1];

    return (norm * (maxValue() - minValue()) + minValue());
}

//----------------------------------------------------------------------------
// Set the control point's value (in data coordinates).
//----------------------------------------------------------------------------
void OpacityMap::controlPointValue(int index, float value)
{
    vector<double> cps = GetControlPoints();
    if (index < 0 || index * 2 >= cps.size()) { return; }

    float nv = (value - minValue()) / (maxValue() - minValue());

    float minVal = 0.0;
    float maxVal = 1.0;

    if (index > 0) { minVal = cps[2 * index - 1]; }

    if (index * 2 + 3 < cps.size()) { maxVal = cps[2 * index + 3]; }

    if (nv < minVal) {
        nv = minVal;
    } else if (nv > maxVal) {
        nv = maxVal;
    }

    cps[2 * index + 1] = nv;
}

//----------------------------------------------------------------------------
// Set the mean value (normalized coordinates) for the gaussian function
//----------------------------------------------------------------------------
void OpacityMap::SetMean(double mean) { SetValueDouble(_meanTag, "Set opacity mean", mean); }

//----------------------------------------------------------------------------
// Set the sigma squared value (normalized coordinates) for the gaussian
// function
//----------------------------------------------------------------------------
void OpacityMap::SetSSQ(double ssq) { SetValueDouble(_ssqTag, "Set Opac SSQ", ssq); }

//----------------------------------------------------------------------------
// Set the frequency (normalized coordinates) of the sine function
//----------------------------------------------------------------------------
void OpacityMap::SetFreq(double freq) { SetValueDouble(_freqTag, "Set Opac Freq", freq); }

//----------------------------------------------------------------------------
// Set the phase (normalized coordinates) of the sine function
//----------------------------------------------------------------------------
void OpacityMap::SetPhase(double p) { SetValueDouble(_phaseTag, "Set Opac Phase", denormSinePhase(p)); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
float OpacityMap::opacityData(float value) const
{
    float          nv = (value - minValue()) / (maxValue() - minValue());
    vector<double> cps = GetControlPoints();
    if (value < minValue()) { nv = 0; }

    if (value > maxValue()) { nv = 1.0; }
    OpacityMap::Type _type = GetType();
    switch (_type) {
    case CONTROL_POINT: {
        //
        // Find the bounding control points
        //
        int    index = leftControlIndex(nv);
        double val0 = cps[2 * index + 1];
        double val1 = cps[2 * index + 3];

        float ratio = (nv - val0) / (val1 - val0);

        if (ratio > 0. && ratio < 1.) {
            float o = TFInterpolator::interpolate(GetInterpType(), cps[2 * index], cps[2 * index + 2], ratio);
            return o;
        }

        if (ratio >= 1.0) { return cps[2 * index + 2]; }

        return cps[2 * index];
    }

    case GAUSSIAN: {
        return pow(M_E, -((nv - GetMean()) * (nv - GetMean())) / (2.0 * GetSSQ()));
    }

    case INVERTED_GAUSSIAN: {
        return 1.0 - pow(M_E, -((nv - GetMean()) * (nv - GetMean())) / (2.0 * GetSSQ()));
    }

    case SINE: {
        return (0.5 + sin(GetFreq() * M_PI * nv + GetPhase()) / 2);
    }
    }

    return 0.0;
}

//----------------------------------------------------------------------------
// Determine if the opacity map bounds the value.
//----------------------------------------------------------------------------
bool OpacityMap::inDataBounds(float value) const { return (value >= minValue() && value <= maxValue()); }

void OpacityMap::SetDataBounds(const vector<double> &bounds)
{
    assert(bounds.size() == 2);

    SetValueDoubleVec(_dataBoundsTag, "Set min max map value", bounds);
}

vector<double> OpacityMap::GetDataBounds() const
{
    vector<double> defaultv(2, 0.0);

    vector<double> bounds = GetValueDoubleVec(_dataBoundsTag, defaultv);

    if (bounds.size() != 2) bounds = defaultv;

    return (bounds);
}

//----------------------------------------------------------------------------
// binary search , find the index of the largest control point <= val
// Requires that control points are increasing.
//
// Developed by Alan Norton.
//----------------------------------------------------------------------------
int OpacityMap::leftControlIndex(float normval) const
{
    vector<double> cps = GetControlPoints();
    int            left = 0;
    int            right = cps.size() / 2 - 1;

    //
    // Iterate, keeping left to the left of ctrl point
    //
    while (right - left > 1) {
        int mid = left + (right - left) / 2;
        if (cps[mid * 2 + 1] > normval) {
            right = mid;
        } else {
            left = mid;
        }
    }

    return left;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double OpacityMap::normSSq(double ssq) { return (ssq - _minSSq) / (_maxSSq - _minSSq); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double OpacityMap::denormSSq(double ssq) { return _minSSq + (ssq * (_maxSSq - _minSSq)); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double OpacityMap::normSineFreq(double freq) { return (freq - _minFreq) / (_maxFreq - _minFreq); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double OpacityMap::denormSineFreq(double freq) { return _minFreq + (freq * (_maxFreq - _minFreq)); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double OpacityMap::normSinePhase(double phase) { return (phase - _minPhase) / (_maxPhase - _minPhase); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double OpacityMap::denormSinePhase(double phase) { return _minPhase + (phase * (_maxPhase - _minPhase)); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void OpacityMap::setOpaque()
{
    vector<double> cps = GetControlPoints();
    for (int i = 0; i < cps.size() / 2; i++) { cps[2 * i] = 1.; }
    SetControlPoints(cps);
}
bool OpacityMap::isOpaque() const
{
    vector<double> cps = GetControlPoints();
    for (int i = 0; i < cps.size(); i += 2) {
        if (cps[i] < 1.0) return false;
    }
    return true;
}

void OpacityMap::SetEnabled(bool val) { SetValueLong(_enabledTag, "Change opacity map enabled", (long)val); }

vector<double> OpacityMap::GetControlPoints() const
{
    vector<double> cps = GetValueDoubleVec(_controlPointsTag);

    while (cps.size() % 2) cps.push_back(0.0);

    return (cps);
}

void OpacityMap::SetControlPoints(const vector<double> &controlPoints)
{
    vector<double> mycp = controlPoints;

    while (mycp.size() % 2) mycp.push_back(0.0);

    SetValueDoubleVec(_controlPointsTag, "Set opacity control points", mycp);
}

void OpacityMap::SetInterpType(TFInterpolator::type t) { SetValueLong(_interpTypeTag, "Set Opacity Interpolation", (long)t); }

void OpacityMap::SetType(OpacityMap::Type t) { SetValueLong(_typeTag, "Set Opacity Map Type", (long)t); }

//----------------------------------------------------------------------------
// Set the minimum value of the opacity map (in data coordinates).
//
// The minimum value is stored as normalized coordinates in the parameter
// space. Therefore, the opacity map will change relative to any changes in
// the parameter space.
//----------------------------------------------------------------------------
void OpacityMap::setMinValue(double val)
{
    vector<double> minmaxmap = GetDataBounds();
    if (val > minmaxmap[1]) val = minmaxmap[1];
    if (val < minmaxmap[0]) val = minmaxmap[0];

    // change to relative max
    if (minmaxmap[1] > minmaxmap[0])
        val = (val - minmaxmap[0]) / (minmaxmap[1] - minmaxmap[0]);
    else
        val = 0.;

    SetValueDouble(_relMinTag, "Set Opacity min", val);
}
//----------------------------------------------------------------------------
// Set the minimum value of the opacity map (in data coordinates).
//
// The minimum value is stored as normalized coordinates in the parameter
// space. Therefore, the opacity map will change relative to any changes in
// the parameter space.
//----------------------------------------------------------------------------
void OpacityMap::setMaxValue(double val)
{
    vector<double> minmaxmap = GetDataBounds();
    if (val < minmaxmap[0]) val = minmaxmap[0];
    if (val > minmaxmap[1]) val = minmaxmap[1];

    // change to relative max
    if (minmaxmap[1] > minmaxmap[0])
        val = (val - minmaxmap[0]) / (minmaxmap[1] - minmaxmap[0]);
    else
        val = 1.;

    SetValueDouble(_relMaxTag, "Set Opacity max", val);
}

double OpacityMap::minValue() const
{
    vector<double> minmaxmap = GetDataBounds();
    double         relmin = GetValueDouble(_relMinTag, 0.0);
    return (minmaxmap[0] + relmin * (minmaxmap[1] - minmaxmap[0]));
}

double OpacityMap::maxValue() const
{
    vector<double> minmaxmap = GetDataBounds();
    double         relmax = GetValueDouble(_relMaxTag, 1.0);
    return (minmaxmap[0] + relmax * (minmaxmap[1] - minmaxmap[0]));
}
