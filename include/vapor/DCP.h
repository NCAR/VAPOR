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

#ifndef _DCP_H_
    #define _DCP_H_

//! This enables a beta feature which automatically generates particle density fields
//! when importing particle data. In it's current state, if DCP_ENABLE_PARTICLE_DENSITY
//! is set to 1, it will work but the volume will have a fixed grid size and it uses a sub-optimal sampling algorithm.
    #define DCP_ENABLE_PARTICLE_DENSITY 0

namespace VAPoR {


//! DCP is a simple daa format for particle data. The full specification can be found at
//! vapor/share/doc/DCP_Format.md

class VDF_API DCP : public VAPoR::DC {
public:
    DCP();
    virtual ~DCP();

protected:
    //! Initialize the DCP class
    //!
    //! Prepare a CF data set for reading. This method prepares
    //! the DCP class for reading the files indicated by
    //! \p paths.
    //! The method should be called immediately after the constructor,
    //! before any other class methods. This method
    //! exists only because C++ constructors can not return error codes.
    //!
    //! \param[in] path A list of CF NetCDF files comprising the output of
    //! a single CF model run.
    //!
    //! \retval status A negative int is returned on failure
    //!
    //! \sa EndDefine();
    //
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
        // Projections not supported yet :-(
        //
        return (_proj4String);
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
    virtual int openVariableRead(size_t ts, string varname, int, int) { return (DCP::openVariableRead(ts, varname)); }

    virtual int openVariableRead(size_t ts, string varname);


    //! \copydoc DC::CloseVariable()
    //!
    virtual int closeVariable(int fd);

    //! \copydoc DC::ReadRegion()
    //
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionTemplate(fd, min, max, region)); }
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }

    //! \copydoc DC::ReadRegionBlock()
    //!
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionTemplate(fd, min, max, region)); };
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }

    //! \copydoc DC::VariableExists()
    //!
    virtual bool variableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const;

private:
    NetCDFCollection *_ncdfc;
    VAPoR::UDUnits    _udunits;

    string                          _proj4String;
    std::map<string, DC::Dimension> _dimsMap;
    std::map<string, DC::CoordVar>  _coordVarsMap;
    std::map<string, DC::AuxVar>    _auxVarsMap;
    std::map<string, DC::Mesh>      _meshMap;
    std::map<string, DC::DataVar>   _dataVarsMap;
    std::map<string, string>        _coordVarKeys;
    std::map<string, string>        _sanitizedToOriginalMap;

    const string          _nodeFaceVar = "cellsOnVertex";
    const string          _faceNodeVar = "verticesOnCell";
    const string          _fakeEmptyVar = "empty";
    vector<string>        _fakeVars = {_nodeFaceVar, _faceNodeVar};
    int                   _fakeVarsFileCounterStart = 10000;
    int                   _fakeVarsFileCounter = _fakeVarsFileCounterStart;
    std::map<int, string> _fdMap;


    string sanitizeVarName(const string &name);
    string getOriginalVarName(const string &name) const;
    bool   isCoordVar(const string &var) const;
    int    getAxis(const string &var) const;
    string getUnits(const string &var) const;
    string getTimeCoordVar(const string &var) const;

    template<class T> int _readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region);

    template<class T> bool _getAttTemplate(string varname, string attname, T &values) const;
};
};    // namespace VAPoR

#endif
