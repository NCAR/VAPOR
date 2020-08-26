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

    virtual bool   GetDefaultLightingEnabled() const override { return true; }
    virtual double GetDefaultPhongAmbient() const override { return 0.2; }
    virtual string GetDefaultAlgorithmName() const override;

    bool HasIsoValues() const override { return true; }

protected:
    virtual bool GetUseSingleColorDefault() const override { return true; }

private:
    void _init();
};

};    // namespace VAPoR
