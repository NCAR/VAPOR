#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API VolumeParams : public RenderParams {
public:
    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~VolumeParams();

    virtual bool IsOpaque() const;

    virtual bool usingVariable(const std::string &varname);

    static string GetClassType() { return ("VolumeParams"); }

private:
    void _init();
};

};    // namespace VAPoR
