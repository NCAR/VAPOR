
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
        vector<long> dims(3);
        dims[0] = GetValueLong(_xBarbsCountTag, 10);
        dims[1] = GetValueLong(_yBarbsCountTag, 10);
        dims[2] = GetValueLong(_zBarbsCountTag, 1);
        return dims;
    }

    //! Specify the size of a discrete grid
    //! E.g. the grid on which barbs are placed.
    //! \param[in] int array of size 3 - Specifies the barb distribution on the X, Y, and Z axes.
    void SetGrid(const int grid[3])
    {
        SetValueLong(_xBarbsCountTag, "", grid[0]);
        SetValueLong(_yBarbsCountTag, "", grid[1]);
        SetValueLong(_zBarbsCountTag, "", grid[2]);
    }

    //! Query line thickness as a multiplier that's applied to the default value.
    //! \retval double line thickness.
    double GetLineThickness() const { return GetValueDouble(_thicknessScaleTag, 1.f); }

    //! Set line thickness as a multiplier that's applied to the default value.
    //! \param[in] double - Line thickness.
    void SetLineThickness(double val) { SetValueDouble(_thicknessScaleTag, "Barb thickness", val); }

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("BarbParams"); }

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override
    {
        for (const auto &p : GetFieldVariableNames()) {
            if (!p.empty()) return _dataMgr->GetVarTopologyDim(p);
        }
        return GetBox()->IsPlanar() ? 2 : 3;
    }

    //! \copydoc RenderParams::GetActualColorMapVariableName()
    virtual string GetActualColorMapVariableName() const override
    {
        if (UseSingleColor())
            return "";
        else
            return GetColorMapVariableName();
    }

protected:
    virtual bool GetUseSingleColorDefault() const override { return true; }

private:
    void _init();

public:
    static const string _needToRecalculateScalesTag;
    static const string _lengthScaleTag;
    static const string _thicknessScaleTag;
    //! Number of barbs displayed on X axis
    static const string _xBarbsCountTag;
    //! Number of barbs displayed on Y axis
    static const string _yBarbsCountTag;
    //! Number of barbs displayed on Z axis
    static const string _zBarbsCountTag;
    static const string _alignGridTag;
    static const string _alignGridStridesTag;
    static const string _varsAre3dTag;

};    // End of Class BarbParams
};    // namespace VAPoR

#endif
