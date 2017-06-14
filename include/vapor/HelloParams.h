
#ifndef HELLOPARAMS_H
#define HELLOPARAMS_H

#include <vapor/RenderParams.h>

namespace VAPoR {

//! \class HelloParams
//! \brief Class that supports drawing a line connecting two points.
//! \author Alan Norton
//! \version 3.0
//! \date June 2015
class PARAMS_API HelloParams : public RenderParams {
  public:
    HelloParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    HelloParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~HelloParams();

    //! \copydoc RenderParams::IsOpaque()
    virtual bool IsOpaque() const { return true; }

    //! \copydoc RenderParams::usingVariable()
    virtual bool usingVariable(const std::string &varname) {
        return (GetVariableName() == varname);
    }

    //! Determine line thickness in voxels
    //! \retval double line thickness
    double GetLineThickness() {
        return (GetValueDouble(m_lineThicknessTag, 1.0));
    }

    //! Set the line thickness
    //! \param[in] double thickness
    //! \retval int 0 if success
    void SetLineThickness(double val) {
        SetValueDouble(m_lineThicknessTag, "Set line thickness", val);
    }

    //! Obtain the first endpoint in user coordinates.
    const vector<double> GetPoint1() {
        vector<double> defaultv(3, 0.0);
        return GetValueDoubleVec(m_point1Tag, defaultv);
    }

    //! Obtain the second endpoint in user coordinates.
    const vector<double> GetPoint2() {
        vector<double> defaultv(3, 1.0);
        return GetValueDoubleVec(m_point2Tag, defaultv);
    }

    //! Set the first endpoint
    void SetPoint1(vector<double> pt) {
        assert(pt.size() == 3);
        SetValueDoubleVec(m_point1Tag, "Set First Endpoint", pt);
    }

    //! Set the second endpoint
    void SetPoint2(vector<double> pt) {
        assert(pt.size() == 3);
        SetValueDoubleVec(m_point2Tag, "Set Second Endpoint", pt);
    }

    // Get static string identifier for this params class
    //
    static string GetClassType() {
        return ("HelloParams");
    }

  private:
    static const string m_lineThicknessTag;
    static const string m_point1Tag;
    static const string m_point2Tag;

    void _init();

};     //End of Class HelloParams
};     // namespace VAPoR
#endif //HELLOPARAMS_H
