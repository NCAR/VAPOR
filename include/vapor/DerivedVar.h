#include <iostream>
#include <vapor/DC.h>
#include <vapor/MyBase.h>
#include <vapor/Proj4API.h>

#ifndef _DERIVEDVAR_H_
    #define _DERIVEDVAR_H_

namespace VAPoR {

class NetCDFCollection;

//!
//! \class DerivedVar
//!
//! \brief Derived variable abstract class
//!
//! \author John Clyne
//! \date    January, 2017
//!
//!
class VDF_API DerivedVar : public Wasp::MyBase {
public:
    DerivedVar(string varName) { _derivedVarName = varName; };

    virtual ~DerivedVar() {}

    virtual int Initialize() = 0;

    string GetName() const { return (_derivedVarName); }

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const = 0;

    virtual bool GetAtt(string attname, std::vector<double> &values) const
    {
        values.clear();
        return (false);
    }

    virtual bool GetAtt(string attname, std::vector<long> &values) const
    {
        values.clear();
        return (false);
    }

    virtual bool GetAtt(string attname, string &values) const
    {
        values.clear();
        return (false);
    }

    virtual std::vector<string> GetAttNames() const { return (std::vector<string>()); }

    virtual DC::XType GetAttType(string attname) const { return (DC::INVALID); }

    virtual std::vector<string> GetInputs() const = 0;

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const = 0;

    virtual size_t GetNumRefLevels() const { return (1); }

    virtual std::vector<size_t> GetCRatios() const { return (std::vector<size_t>(1, 1)); }

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0) = 0;

    virtual int CloseVariable(int fd) = 0;

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) = 0;

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) = 0;

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const = 0;

protected:
    string        _derivedVarName;
    DC::FileTable _fileTable;

    int _getVar(DC *dc, size_t ts, string varname, int level, int lod, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) const;

    int _getVarBlock(DC *dc, size_t ts, string varname, int level, int lod, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) const;
};

//!
//! \class DerivedCoordVar
//!
//! \brief Derived coordinate variable abstract class
//!
//! \author John Clyne
//! \date   Februrary, 2018
//!
//!
class VDF_API DerivedCoordVar : public DerivedVar {
public:
    DerivedCoordVar(string varName) : DerivedVar(varName) {}
    virtual ~DerivedCoordVar() {}

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const = 0;
};

//!
//! \class DerivedCFVertCoordVar
//!
//! \brief Derived coordinate variable abstract class
//!
//! \author John Clyne
//! \date   July, 2018
//!
//!
class VDF_API DerivedCFVertCoordVar : public DerivedCoordVar {
public:
    DerivedCFVertCoordVar(string varName, DC *dc, string mesh, string formula) : DerivedCoordVar(varName), _dc(dc), _mesh(mesh), _formula(formula) {}

    virtual ~DerivedCFVertCoordVar() {}

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const = 0;

protected:
    DC *   _dc;
    string _mesh;
    string _formula;
};

//!
//! \class DerivedDataVar
//!
//! \brief Derived data variable abstract class
//!
//! \author John Clyne
//! \date   Februrary, 2018
//!
//!
class VDF_API DerivedDataVar : public DerivedVar {
public:
    DerivedDataVar(string varName) : DerivedVar(varName) {}
    virtual ~DerivedDataVar() {}

    virtual bool GetDataVarInfo(DC::DataVar &cvar) const = 0;
};

//!
//! \class DerivedCoordVar_PCSFromLatLon
//!
//! \brief Derived PCS coordinate variable from lat-lon coordinate pairs
//!
//! \author John Clyne
//! \date   Februrary, 2018
//!
//!
class VDF_API DerivedCoordVar_PCSFromLatLon : public DerivedCoordVar {
public:
    DerivedCoordVar_PCSFromLatLon(string derivedVarName, DC *dc, std::vector<string> inNames, string proj4String, bool uGridFlag, bool lonFlag);
    virtual ~DerivedCoordVar_PCSFromLatLon() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>{_lonName, _latName}); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

private:
    DC *                _dc;
    string              _proj4String;
    string              _lonName;
    string              _latName;
    string              _xCoordName;
    string              _yCoordName;
    bool                _make2DFlag;
    bool                _uGridFlag;
    bool                _lonFlag;
    std::vector<size_t> _dimLens;
    Proj4API            _proj4API;
    DC::CoordVar        _coordVarInfo;

    int _setupVar();

    int _readRegionHelperCylindrical(DC::FileTable::FileObject *f, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);
    int _readRegionHelper1D(DC::FileTable::FileObject *f, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);
    int _readRegionHelper2D(DC::FileTable::FileObject *f, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);
};

//!
//! \class DerivedCoordVar_CF1D
//!
//! \brief Derived 1D CF conventions coordinate variable using
//! grid coordinates
//!
//! \author John Clyne
//! \date   Februrary, 2018
//!
//!
class VDF_API DerivedCoordVar_CF1D : public DerivedCoordVar {
public:
    DerivedCoordVar_CF1D(string derivedVarName, DC *dc, string dimName, int axis, string units);
    virtual ~DerivedCoordVar_CF1D() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>()); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

private:
    DC *         _dc;
    string       _dimName;
    size_t       _dimLen;
    DC::CoordVar _coordVarInfo;
};

//!
//! \class DerivedCoordVar_CF2D
//!
//! \brief Derived 2D CF conventions coordinate variable . Coordinates
//! are provided by \p data, whose length must be dimlens[0] * dimLens[1]
//!
//! \author John Clyne
//! \date   Februrary, 2018
//!
//!
class VDF_API DerivedCoordVar_CF2D : public DerivedCoordVar {
public:
    DerivedCoordVar_CF2D(string derivedVarName, std::vector<string> dimNames, std::vector<size_t> dimLens, int axis, string units, const vector<float> &data);
    virtual ~DerivedCoordVar_CF2D() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>()); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

private:
    std::vector<string> _dimNames;
    std::vector<size_t> _dimLens;
    std::vector<float>  _data;
    DC::CoordVar        _coordVarInfo;
};

//!
//! \class DerivedCoordVar_WRFTime
//!
//! \brief Derived WRF Time coordinate variable
//!
//! \author John Clyne
//! \date   Februrary, 2018
//!
//!
class VDF_API DerivedCoordVar_WRFTime : public DerivedCoordVar {
public:
    DerivedCoordVar_WRFTime(string derivedVarName, NetCDFCollection *ncdfc, string wrfTimeVar, string dimName, float p2si = 1.0);
    virtual ~DerivedCoordVar_WRFTime() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>{_wrfTimeVar}); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

    size_t TimeLookup(size_t ts) const { return (ts < _timePerm.size() ? _timePerm[ts] : 0); }

private:
    NetCDFCollection * _ncdfc;
    std::vector<float> _times;
    std::vector<int>   _timePerm;
    string             _wrfTimeVar;
    float              _p2si;
    size_t             _ovr_ts;
    DC::CoordVar       _coordVarInfo;
};

//!
//! \class DerivedCoordVar_TimeInSeconds
//!
//! \brief Derived time coordinate variable
//!
//! \author John Clyne
//! \date   Februrary, 2018
//!
//!
class VDF_API DerivedCoordVar_TimeInSeconds : public DerivedCoordVar {
public:
    DerivedCoordVar_TimeInSeconds(string derivedVarName, DC *dc, string nativeTimeVar, string dimName);
    virtual ~DerivedCoordVar_TimeInSeconds() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>{_nativeTimeVar}); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

    const vector<double> &GetTimes() const { return (_times); }

private:
    DC *                _dc;
    std::vector<double> _times;
    string              _nativeTimeVar;
    DC::CoordVar        _coordVarInfo;
};

class VDF_API DerivedCoordVar_Staggered : public DerivedCoordVar {
public:
    DerivedCoordVar_Staggered(string derivedVarName, string stagDimName, DC *dc, string inName, string dimName);
    virtual ~DerivedCoordVar_Staggered() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>{_inName}); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

private:
    string       _inName;
    string       _stagDimName;
    string       _dimName;
    DC *         _dc;
    DC::CoordVar _coordVarInfo;
    int          _stagDim;
};

class VDF_API DerivedCoordVar_UnStaggered : public DerivedCoordVar {
public:
    DerivedCoordVar_UnStaggered(string derivedVarName, string unstagDimName, DC *dc, string inName, string dimName);
    virtual ~DerivedCoordVar_UnStaggered() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>{_inName}); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

private:
    string       _inName;
    string       _unstagDimName;
    string       _dimName;
    DC *         _dc;
    DC::CoordVar _coordVarInfo;
    int          _stagDim;
};

class VDF_API DerivedCoordVarStandardWRF_Terrain : public DerivedCFVertCoordVar {
public:
    DerivedCoordVarStandardWRF_Terrain(DC *dc, string mesh, string formula);
    virtual ~DerivedCoordVarStandardWRF_Terrain() {}

    virtual int Initialize();

    virtual bool GetBaseVarInfo(DC::BaseVar &var) const;

    virtual bool GetCoordVarInfo(DC::CoordVar &cvar) const;

    virtual std::vector<string> GetInputs() const { return (std::vector<string>{"PH", "PHB"}); }

    virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    virtual size_t GetNumRefLevels() const { return (_dc->GetNumRefLevels(_PHVar)); }

    virtual std::vector<size_t> GetCRatios() const { return (_dc->GetCRatios(_PHVar)); }

    virtual int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

    virtual int CloseVariable(int fd);

    virtual int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

    virtual int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

    virtual bool VariableExists(size_t ts, int reflevel, int lod) const;

    static bool ValidFormula(string formula);

private:
    string       _PHVar;
    string       _PHBVar;
    float        _grav;
    DC::CoordVar _coordVarInfo;
};

};    // namespace VAPoR

#endif
