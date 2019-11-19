#pragma once

#include <vapor/VolumeParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API VolumeIsoParams : public VolumeParams {
public:
    VolumeIsoParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
    VolumeIsoParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~VolumeIsoParams();

    static string GetClassType() { return ("VolumeIsoParams"); }

    virtual bool   GetDefaultLightingEnabled() const { return true; }
    virtual double GetDefaultPhongAmbient() const { return 0.2; }
    virtual string GetDefaultAlgorithmName() const;

    bool HasIsoValues() const { return true; }

protected:
    virtual bool GetUseSingleColorDefault() const { return true; }

private:
    void _init();
};

};    // namespace VAPoR
