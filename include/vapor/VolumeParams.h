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

    std::string                           GetAlgorithm() const;
    void                                  SetAlgorithm(std::string algorithm);
    static const std::vector<std::string> GetAlgorithmNames();

private:
    void _init();

    static const std::string _algorithmTag;
};

};    // namespace VAPoR
