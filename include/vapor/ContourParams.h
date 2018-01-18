
#ifndef CONTOURPARAMS_H
#define CONTOURPARAMS_H

#include <string>
#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//class ContourParams::Contours;

//! \class ContourParams
//! \brief Class that supports drawing Contours based on 2D or 3D vector field
//! \author Scott Pearse
//! \version 3.0
//! \date June 2017
class PARAMS_API ContourParams : public RenderParams {
  public:
    class Contours;

    ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    ContourParams(const ContourParams &rhs);

    ContourParams &operator=(const ContourParams &rhs);

    virtual ~ContourParams();

    Contours *GetCurrentContours();

    void MakeNewContours(string varName);

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

    //! Determine line thickness in voxels
    //! \retval double line thickness
    double GetLineThickness() const {
        return (GetValueDouble(_thicknessScaleTag, 1.0));
    }

    void SetLineThickness(double val) {
        SetValueDouble(_thicknessScaleTag, "Contour thickness", val);
    }

    int GetContourCount();

    double GetContourMin();

    double GetContourSpacing();

    double GetContourMax();

    void SetContourSpacing(double val);

    void GetLineColor(int lineNum, float color[3]);

    void SetLineColor(vector<double> vec) {
        SetValueDoubleVec(_lineColorTag, "Line color", vec);
    }

    void SetLockToTF(bool lock);

    bool GetLockToTF() const;

    vector<double> GetContourValues(string varName);

    void SetContourValues(string varName, vector<double> vals);

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
        SetValueDouble(_numDigitsTag, "Number of digits in contour annotation",
                       digits);
    }

    int GetTextDensity() const {
        double val = GetValueDouble(_textDensityTag, 1.0);
        return (int)val;
    }

    void SetTextDensity(int density) {
        SetValueDouble(_textDensityTag, "Density of contour annotations",
                       density);
    }

    bool GetTextEnabled() const;
    void SetTFLock(bool lock);
    bool GetTFLock();

  private:
    void _init();
    static const string _thicknessScaleTag;
    static const string _varsAre3dTag;
    static const string _lineColorTag;
    static const string _contoursTag;
    static const string _numDigitsTag;
    static const string _textDensityTag;
    static const string _textEnabledTag;
    static const string _lockToTFTag;
    ParamsContainer *_contours;

  public:
    class PARAMS_API Contours : public ParamsBase {
      public:
        Contours(ParamsBase::StateSave *ssave);

        Contours(ParamsBase::StateSave *ssave, XmlNode *node);

        virtual ~Contours();

        vector<double> GetContourValues() const {
            vector<double> defaultv(7, 0.);
            if (!_node->HasElementDouble(_valuesTag))
                return defaultv;

            vector<double> val = GetValueDoubleVec(_valuesTag);
            return val;
        }

        void SetContourValues(vector<double> vals) {
            SetValueDoubleVec(_valuesTag, "Set contour values", vals);
        }

        double GetMin() const;
        int GetCount() const;      // {
        double GetSpacing() const; //{
        static string GetClassType() {
            return ("Contours");
        }

      private:
        static const string _valuesTag;
    };

}; //End of Class ContourParams

}; // namespace VAPoR

#endif
