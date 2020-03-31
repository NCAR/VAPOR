
#ifndef BARBPARAMS_H
#define BARBPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//! \class BarbParams
//! \brief Class that supports drawing Barbs based on 2D or 3D vector field
//! \author Scott Pearse
//! \version 3.0
//! \date June 2017
class PARAMS_API BarbParams : public RenderParams {
public:
    BarbParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    BarbParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~BarbParams();

    bool GetNeedToRecalculateScales() const { return (bool)GetValueDouble(_needToRecalculateScalesTag, 0.0); }

    void SetNeedToRecalculateScales(bool val);

    //! Get the length scaling factor
    //! \retval double scale factor
    //
    double GetLengthScale() const { return GetValueDouble(_lengthScaleTag, 1.f); }

    void SetLengthScale(double val) { SetValueDouble(_lengthScaleTag, "Barb length", val); }

    //! Determine the size of the discrete grid
    //! E.g. the grid on which barbs are placed.
    //! \retval vector<long> grid
    const vector<long> GetGrid() const
    {
        vector<long> defaultGrid(2, 10);
        defaultGrid.push_back(1.0);
        return (GetValueLongVec(_gridTag, defaultGrid));
    }

    void SetGrid(const int grid[3])
    {
        vector<long> griddims;
        for (int i = 0; i < 3; i++) { griddims.push_back((long)grid[i]); }
        SetValueLongVec(_gridTag, "Set grid", griddims);
    }

    //! Determine line thickness in voxels
    //! \retval double line thickness
    double GetLineThickness() const { return GetValueDouble(_thicknessScaleTag, 1.f); }

    void SetLineThickness(double val) { SetValueDouble(_thicknessScaleTag, "Barb thickness", val); }

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("BarbParams"); }

protected:
    virtual bool GetUseSingleColorDefault() const { return true; }

private:
    void                _init();
    static const string _needToRecalculateScalesTag;
    static const string _lengthScaleTag;
    static const string _thicknessScaleTag;
    static const string _gridTag;
    static const string _alignGridTag;
    static const string _alignGridStridesTag;
    static const string _varsAre3dTag;

};    // End of Class BarbParams
};    // namespace VAPoR

#endif
