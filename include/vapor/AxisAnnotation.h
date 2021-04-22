#ifndef AXISANNOTATION_H
#define AXISANNOTATION_H
/*
 * This class describes a viewpoint
 */
#include <vapor/ParamsBase.h>

namespace VAPoR {

//! \class AxisAnnotation
//! \ingroup Public_Params
//! \brief class that indicates location and direction of view
//! \author Scott Pearse
//! \version 3.0
//! \date January 2018

//! \par
//! This class contains all the parameters associated with axis annotations,
//! for a DataMgr or Renderer.

class PARAMS_API AxisAnnotation : public ParamsBase {
public:
    enum Flags {};

    AxisAnnotation(ParamsBase::StateSave *ssave);
    AxisAnnotation(ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~AxisAnnotation();

    void Initialize();

    void SetAxisAnnotationEnabled(bool val);
    bool GetAxisAnnotationEnabled() const;

    vector<double> GetAxisBackgroundColor() const;
    void           GetAxisBackgroundColor(float bgColor[]) const;
    void           SetAxisBackgroundColor(vector<double> color);

    vector<double> GetAxisColor() const;
    void           SetAxisColor(vector<double> color);

    void           SetNumTics(vector<double> ticnums);
    vector<double> GetNumTics() const;

    void           SetAxisOrigin(vector<double> orig);
    vector<double> GetAxisOrigin() const;

    void           SetMinTics(vector<double> ticmins);
    vector<double> GetMinTics() const;

    void           SetMaxTics(vector<double> ticmaxs);
    vector<double> GetMaxTics() const;

    void           SetTicSize(vector<double> ticsizes);
    vector<double> GetTicSize() const;

    void           SetXTicDir( double dir );
    int            GetXTicDir() const;

    void           SetYTicDir( double dir );
    int            GetYTicDir() const;

    void           SetZTicDir( double dir );
    int            GetZTicDir() const;

    void           SetTicDirs(vector<double> ticdirs);
    vector<double> GetTicDirs() const;

    double GetTicWidth() const;
    void   SetTicWidth(double val);

    long GetAxisTextHeight() const;
    void SetAxisTextHeight(long val);

    long GetAxisDigits() const;
    void SetAxisDigits(long val);

    void SetLatLonAxesEnabled(bool val);
    bool GetLatLonAxesEnabled() const;

    string GetDataMgrName() const;
    void   SetDataMgrName(string dataMgr);

    bool GetShowAxisArrows() const;
    void SetShowAxisArrows(bool val);

    void SetAxisFontSize(int size);
    int  GetAxisFontSize() const;

    bool GetAxisAnnotationInitialized() const;
    void SetAxisAnnotationInitialized(bool val);

    static string GetClassType() { return ("AxisAnnotation"); }

    static const string _colorTag;
    static const string _digitsTag;
    static const string _textHeightTag;
    static const string _fontSizeTag;
    static const string _ticWidthTag;
    static const string _ticDirsTag;
    static const string _ticSizeTag;
    static const string _minTicsTag;
    static const string _maxTicsTag;
    static const string _numTicsTag;
    static const string _originTag;
    static const string _backgroundColorTag;
    static const string _annotationEnabledTag;
    static const string _latLonAxesTag;
    static const string _dataMgrTag;
    static const string _initializedTag;
};
};    // namespace VAPoR

#endif    // AXISANNOTATION_H
