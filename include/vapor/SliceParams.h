
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

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("SliceParams"); }

    std::vector<int> GetSampleRates() const;

    void SetSampleRates(std::vector<int> rates);

    bool IsOpaque() const;

private:
    void _init();
    bool usingVariable(const std::string &varname);

    static const string _samplingRateTag;

};    // End of Class SliceParams
};    // namespace VAPoR

#endif
