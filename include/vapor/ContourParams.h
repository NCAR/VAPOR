
#ifndef CONTOURPARAMS_H
#define CONTOURPARAMS_H

#include <string>
#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class Contours;

//! \class ContourParams
//! \brief Class that supports drawing Contours based on 2D or 3D vector field
//! \author Scott Pearse
//! \version 3.0
//! \date June 2017
class PARAMS_API ContourParams : public RenderParams {
public:
    ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    ContourParams(const ContourParams &rhs);

    ContourParams &operator=(const ContourParams &rhs);

    virtual ~ContourParams();

    Contours *GetContours();

    void MakeNewContours(string varName);

    //! \copydoc RenderParams::IsOpaque()
    virtual bool IsOpaque() const;

    //!
    //! \copydoc RenderParams::usingVariable()
    virtual bool usingVariable(const std::string &varname);

    //! Set the variable type being used by the barbs
    //!
    void SetVariables3D(bool val)
    {
        if (val)
            SetValueString(_varsAre3dTag, "Set variable dimensionality", "true");
        else
            SetValueString(_varsAre3dTag, "Set variable dimensionality", "false");
    }

    //! Find out whether the barbs are using 2D or 3D variables
    //!
    bool VariablesAre3D() const
    {
        if (GetValueString(_varsAre3dTag, "true") == "true") { return true; }
        return false;
    }

    int GetNumContours();

    void SetNumContours(int num);

    //! Determine line thickness in voxels
    //! \retval double line thickness
    double GetLineThickness() const { return (GetValueDouble(_thicknessScaleTag, 1.0)); }

    void SetLineThickness(double val) { SetValueDouble(_thicknessScaleTag, "Contour thickness", val); }

    double GetContourMin();

    void SetContourMin(double val);

    double GetContourSpacing();

    void SetContourSpacing(double val);

    void GetLineColor(int lineNum, float color[3])
    {
        GetConstantColor(color);
        string cmVar = GetColorMapVariableName();
        if ((cmVar == "") || (cmVar == "Default")) {
            string            varName = GetVariableName();
            TransferFunction *tf = 0;
            tf = (TransferFunction *)GetMapperFunc(varName);
            if (!tf) { tf = MakeTransferFunc(varName); }
            assert(tf);

            // vector<double> vals = GetValueDoubleVec(_contoursTag);
            vector<double> vals = GetIsovalues(varName);
            double         val = vals[lineNum];

            tf->rgbValue(val, color);
        } else {
            GetConstantColor(color);
        }
    }

    void SetLineColor(vector<double> vec) { SetValueDoubleVec(_lineColorTag, "Line color", vec); }

    void SetLockToTF(bool lock)
    {
        string l = "false";
        if (lock) { l = "true"; }
        SetValueString(_lockToTFTag, "Lock settings to TF", l);
    }

    bool GetLockToTF()
    {
        if (GetValueString(_lockToTFTag, "true") == "true") {
            return true;
        } else {
            return false;
        }
    }

    vector<double> GetIsovalues(string varName);

    void SetIsovalues(string varName, vector<double> vals);

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("ContourParams"); }

    int GetNumDigits() const
    {
        double val = GetValueDouble(_numDigitsTag, 1.0);
        return (int)val;
    }

    void SetNumDigits(int digits) { SetValueDouble(_numDigitsTag, "Number of digits in isovalue annotation", digits); }

    int GetTextDensity() const
    {
        double val = GetValueDouble(_textDensityTag, 1.0);
        return (int)val;
    }

    void SetTextDensity(int density) { SetValueDouble(_textDensityTag, "Density of isovalue annotations", density); }

    bool GetTextEnabled() const
    {
        if (GetValueString(_textEnabledTag, "false") == "false") {
            return false;
        } else {
            return true;
        }
    }

    void SetTFLock(bool lock)
    {
        string l = "false";
        if (lock) l = "true";
        SetValueString(_lockToTFTag,
                       "Lock contours to transfer function"
                       " bounds",
                       l);
    }

    bool GetTFLock()
    {
        string l = GetValueString(_lockToTFTag, "true");
        if (l == "false") return false;
        return true;
    }

private:
    void                _init();
    static const string _thicknessScaleTag;
    static const string _varsAre3dTag;
    static const string _numContoursTag;
    static const string _lineColorTag;
    static const string _contoursTag;
    static const string _numDigitsTag;
    static const string _textDensityTag;
    static const string _textEnabledTag;
    static const string _contourMinTag;
    static const string _contourSpacingTag;
    static const string _lockToTFTag;
    ParamsContainer *   _contours;

};    // End of Class ContourParams

class PARAMS_API Contours : public ParamsBase {
public:
    Contours(ParamsBase::StateSave *ssave);

    Contours(ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~Contours();

    vector<double> GetIsovalues() const;

    void SetIsovalues(vector<double> vals);

    double GetMin() const { return GetValueDouble(_minTag, 0.); }

    void SetMin(double min) { SetValueDouble(_minTag, "Set contour minimum", min); }

    int GetCount() const { return (int)GetValueDouble(_countTag, 7.); }

    void SetCount(int count) { SetValueDouble(_countTag, "Set contour count", (double)count); }

    double GetSpacing() const { return GetValueDouble(_spacingTag, 1.); }

    void SetSpacing(double spacing) { SetValueDouble(_spacingTag, "Set contour spacing", spacing); }

    static string GetClassType() { return ("Contours"); }

private:
    static const string _minTag;
    static const string _countTag;
    static const string _spacingTag;
};
};    // namespace VAPoR

#endif
