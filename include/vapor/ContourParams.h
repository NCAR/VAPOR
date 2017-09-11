
#ifndef CONTOURPARAMS_H
#define CONTOURPARAMS_H

#include <string>
#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//! \class ContourParams
//! \brief Class that supports drawing Contours based on 2D or 3D vector field
//! \author Scott Pearse
//! \version 3.0
//! \date June 2017
class PARAMS_API ContourParams : public RenderParams {
  public:
    ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~ContourParams();

    //! \copydoc RenderParams::IsOpaque()
    virtual bool IsOpaque() const;

    //!
    //! \copydoc RenderParams::usingVariable()
    virtual bool usingVariable(const std::string &varname);

    //! Set the variable type being used by the barbs
    //!
    void SetVariables3D(bool val) {
        if (val)
            SetValueString(_varsAre3dTag, "Set variable dimensionality", "true");
        else
            SetValueString(_varsAre3dTag, "Set variable dimensionality", "false");
    }

    //! Find out whether the barbs are using 2D or 3D variables
    //!
    bool VariablesAre3D() const {
        if (GetValueString(_varsAre3dTag, "true") == "true") {
            return true;
        }
        return false;
    }

    int GetNumContours() const {
        return (int)GetValueDouble(_numContoursTag, 3.0);
    }

    void SetNumContours(int num) {
        cout << "Internal params SetNumContours" << num << endl;
        SetValueDouble(_numContoursTag, "Number of contours", (double)num);
    }

    //! Determine line thickness in voxels
    //! \retval double line thickness
    double GetLineThickness() const {
        return (GetValueDouble(_thicknessScaleTag, 1.0));
    }

    void SetLineThickness(double val) {
        SetValueDouble(_thicknessScaleTag, "Contour thickness", val);
    }

    double GetContourMin() const {
        return (GetValueDouble(_contourMinTag, 0.f));
    }

    void SetContourMin(double val) {
        SetValueDouble(_contourMinTag, "Contour minimum value", val);
    }

    double GetContourSpacing() const {
        return (GetValueDouble(_contourSpacingTag, 1.f));
    }

    void SetContourSpacing(double val) {
        SetValueDouble(_contourSpacingTag, "Spacing between contours", val);
    }

    void GetLineColor(int lineNum, float color[3]) {

        string varName = GetVariableName();
        TransferFunction *tf = 0;
        tf = (TransferFunction *)GetMapperFunc(varName);
        assert(tf);

        vector<double> vals = GetValueDoubleVec(_contoursTag);
        double val = vals[lineNum];

        tf->rgbValue(val, color);

        //	vector<double> defaultVec;
        //	defaultVec.push_back(1.f);
        //	defaultVec.push_back(0.f);
        //	defaultVec.push_back(0.f);
        //	GetValueDoubleVec(_lineColorTag, defaultVec);
        //	color[0] = 255.f;//defaultVec[0];
        //	color[1] = defaultVec[1];
        //	color[2] = defaultVec[2];
    }

    void SetLineColor(vector<double> vec) {
        SetValueDoubleVec(_lineColorTag, "Line color", vec);
    }

    void SetLockToTF(bool lock) {
        string l = "false";
        if (lock) {
            l = "true";
        }
        SetValueString(_lockToTFTag, "Lock settings to TF", l);
    }

    bool GetLockToTF() {
        if (GetValueString(_lockToTFTag, "true") == "true") {
            return true;
        } else {
            return false;
        }
    }

    vector<double> GetIsovalues() const {
        vector<double> vals;
        vals = GetValueDoubleVec(_contoursTag);
        return vals;
    }

    void SetIsovalues(vector<double> vals) {
        SetValueDoubleVec(_contoursTag, "Isovalue list", vals);
    }

    // Get static string identifier for this params class
    //
    static string GetClassType() {
        return ("ContourParams");
    }

    int GetNumDigits() const {
        double val = GetValueDouble(_numDigitsTag, 1.0);
        return (int)val;
    }

    void SetNumDigits(int digits) {
        SetValueDouble(_numDigitsTag, "Number of digits in isovalue annotation",
                       digits);
    }

    int GetTextDensity() const {
        double val = GetValueDouble(_textDensityTag, 1.0);
        return (int)val;
    }

    void SetTextDensity(int density) {
        SetValueDouble(_textDensityTag, "Density of isovalue annotations",
                       density);
    }

    bool GetTextEnabled() const {
        if (GetValueString(_textEnabledTag, "false") == "false") {
            return false;
        } else {
            return true;
        }
    }

    void SetTFLock(bool lock) {
        string l = "false";
        if (lock)
            l = "true";
        SetValueString(_lockToTFTag, "Lock contours to transfer function"
                                     " bounds",
                       l);
    }

    bool GetTFLock() {
        string l = GetValueString(_lockToTFTag, "true");
        if (l == "false")
            return false;
        return true;
    }

  private:
    void _init();
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

}; //End of Class ContourParams
}; // namespace VAPoR

#endif
