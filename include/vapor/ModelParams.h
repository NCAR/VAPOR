#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API ModelParams : public RenderParams {
public:
    static const std::string FileTag;

    ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
    ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType);
    ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~ModelParams();

    static string GetClassType() { return ("ModelParams"); }

private:
    void _init();
};

};    // namespace VAPoR
