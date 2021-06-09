#pragma once

#include <vapor/DerivedVar.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class DerivedParticleDensity : public DerivedDataVar {
    DC *     _dc;
    string   _meshName;
    DataMgr *_dataMgr;

public:
    DerivedParticleDensity(string varName, DC *dc, string meshName, DataMgr *dataMgr);
    virtual int                 Initialize() override;
    virtual int                 OpenVariableRead(size_t ts, int level = 0, int lod = 0) override;
    virtual int                 CloseVariable(int fd) override;
    virtual int                 ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) override;
    virtual int                 ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) override;
    virtual int                 GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const override;
    virtual bool                VariableExists(size_t ts, int reflevel, int lod) const override;
    virtual bool                GetBaseVarInfo(DC::BaseVar &var) const override;
    virtual bool                GetDataVarInfo(DC::DataVar &cvar) const override;
    virtual std::vector<string> GetInputs() const override;

protected:
    string _dataVar;

    virtual void compute(Grid *inGrid, float *output, int xd, int yd, int zd, size_t realNP) const;
};


class DerivedParticleAverage : public DerivedParticleDensity {
public:
    DerivedParticleAverage(string varName, DC *dc, string meshName, DataMgr *dataMgr, string inputVar);
    virtual int Initialize() override { return 0; }

protected:
    virtual void compute(Grid *inGrid, float *output, int xd, int yd, int zd, size_t realNP) const override;
};


class DerivedCoordVar1DSpan : public DerivedCoordVar_CF1D {
    DC *   _dc;
    float  _minExt, _maxExt;
    string _inputCoordVar;

public:
    DerivedCoordVar1DSpan(string derivedVarName, DC *dc, string dimName, int axis, string units, float minExt, float maxExt);
    DerivedCoordVar1DSpan(string derivedVarName, DC *dc, string dimName, int axis, string units, string inputCoordVar);
    virtual ~DerivedCoordVar1DSpan() {}
    virtual int                 ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) override;
    virtual std::vector<string> GetInputs() const override;
};

}    // namespace VAPoR
