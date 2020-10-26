
#ifndef WIREFRAMEDATAPARAMS_H
#define WIREFRAMEDATAPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//! \class WireFrameParams
//! \brief Class that supports drawing Barbs based on 2D or 3D vector field
//! \author John Clyne
//! \version 3.0
class PARAMS_API WireFrameParams : public RenderParams {
public:
    WireFrameParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    WireFrameParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~WireFrameParams();

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("WireFrameParams"); }

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override { return (_dataMgr->GetVarTopologyDim(GetVariableName())); }

private:
    void _init();

};    // End of Class WireFrameParams
};    // namespace VAPoR

#endif
