#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/NetCDFCFCollection.h>
#include <vapor/Proj4API.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/utils.h>
#include <vapor/DC.h>
#include <vapor/STLUtils.h>

#pragma once

namespace VAPoR {


//! \class DCRAM
//! \brief DCRAM is a virtual data collection used to allow data to be loaded from ram. Specifically, this is used by PythonDataMgr
//! \author Stas Jaroszynski

class VDF_API DCRAM : public VAPoR::DC {
public:
    DCRAM();
    virtual ~DCRAM();
    
    void Test();
    
    void AddDimension(const DC::Dimension &dim);
    void AddMesh(const DC::Mesh &mesh);
    void AddCoordVar(const DC::CoordVar &var, const float *buf);
    void AddDataVar(const DC::DataVar &var, const float *buf);

protected:
    map<string, float*> _dataMap;
    
    void copyVarData(const DC::BaseVar &var, const float *buf, const size_t size);
    
    //! \copydoc DC::Initialize()
    //!
    virtual int initialize(const vector<string> &paths, const std::vector<string> &options);


    //! \copydoc DC::getDimension()
    //!
    virtual bool getDimension(string dimname, DC::Dimension &dimension) const;
    virtual bool getDimension(string dimname, DC::Dimension &dimension, long ts) const;

    //! \copydoc DC::getDimensionNames()
    //!
    virtual std::vector<string> getDimensionNames() const;

    //! \copydoc DC::getMeshNames()
    //!
    std::vector<string> getMeshNames() const;

    //! \copydoc DC::getMesh()
    //!
    virtual bool getMesh(string mesh_name, DC::Mesh &mesh) const;

    //! \copydoc DC::GetCoordVarInfo()
    //!
    virtual bool getCoordVarInfo(string varname, DC::CoordVar &cvar) const;

    //! \copydoc DC::GetDataVarInfo()
    //!
    virtual bool getDataVarInfo(string varname, DC::DataVar &datavar) const;

    //! \copydoc DC::GetAuxVarInfo()
    //!
    virtual bool getAuxVarInfo(string varname, DC::AuxVar &var) const;


    //! \copydoc DC::GetBaseVarInfo()
    //
    virtual bool getBaseVarInfo(string varname, DC::BaseVar &var) const;



    //! \copydoc DC::GetDataVarNames()
    //!
    virtual std::vector<string> getDataVarNames() const;

    virtual std::vector<string> getAuxVarNames() const;


    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual std::vector<string> getCoordVarNames() const;

    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual size_t getNumRefLevels(string varname) const { return (1); }

    //! \copydoc DC::GetMapProjection()
    //!
    virtual string getMapProjection() const
    {
        return "";
    }

    //! \copydoc DC::GetAtt()
    //!
    virtual bool getAtt(string varname, string attname, vector<double> &values) const;
    virtual bool getAtt(string varname, string attname, vector<long> &values) const;
    virtual bool getAtt(string varname, string attname, string &values) const;

    //! \copydoc DC::GetAttNames()
    //!
    virtual std::vector<string> getAttNames(string varname) const;

    //! \copydoc DC::GetAttType()
    //!
    virtual XType getAttType(string varname, string attname) const;

    //! \copydoc DC::GetDimLensAtLevel()
    //!
    virtual int getDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;
    virtual int getDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level, long ts) const;


    //! \copydoc DC::OpenVariableRead()
    //!
    virtual int openVariableRead(size_t ts, string varname, int, int) { return (DCRAM::openVariableRead(ts, varname)); }

    virtual int openVariableRead(size_t ts, string varname);


    //! \copydoc DC::CloseVariable()
    //!
    virtual int closeVariable(int fd);

    //! \copydoc DC::ReadRegion()
    //
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionTemplate(fd, min, max, region)); }
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, double *region) { VAssert(!"DCRAM: double data not supported"); return -1; }

    //! \copydoc DC::ReadRegionBlock()
    //!
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionTemplate(fd, min, max, region)); };
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, double *region) { VAssert(!"DCRAM: double data not supported"); return -1; }

    //! \copydoc DC::VariableExists()
    //!
    virtual bool variableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const;

private:
    std::map<string, DC::Dimension> _dimsMap;
    std::map<string, DC::CoordVar>  _coordVarsMap;
    std::map<string, DC::AuxVar>    _auxVarsMap;
    std::map<string, DC::Mesh>      _meshMap;
    std::map<string, DC::DataVar>   _dataVarsMap;

    int                   _fakeVarsFileCounterStart = 10000;
    int                   _fakeVarsFileCounter = _fakeVarsFileCounterStart;
    std::map<int, string> _fdMap;


    template<class T> int _readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region);

    template<class T> bool _getAttTemplate(string varname, string attname, T &values) const;
};
};    // namespace VAPoR
