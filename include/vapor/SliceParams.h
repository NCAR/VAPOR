
#ifndef SLICEPARAMS_H
#define SLICEPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//! \class SliceParams
//! \brief Class that supports drawing Barbs based on 2D or 3D vector field
//! \author Alan Norton
//! \version 3.0
class PARAMS_API SliceParams : public RenderParams {
public:
    SliceParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    SliceParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~SliceParams();

    virtual int Initialize() override;

    virtual void SetRefinementLevel(int level) override;

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("SliceParams"); }

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override { return (3); }

    int GetSampleRate() const;

    void SetSampleRate(int rate);

    int GetDefaultSampleRate() const;

    void SetCachedValues(std::vector<double> values);

    std::vector<double> GetCachedValues() const;

    static const string _sampleRateTag;
    static const string SampleLocationTag;

private:
    bool _initialized = false;

    void                _init();
    std::vector<double> _cachedValues;

};    // End of Class SliceParams
};    // namespace VAPoR

#endif
