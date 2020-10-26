#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/NetCDFCollection.h>
#include <vapor/DerivedVar.h>
#include <vapor/DerivedVarMgr.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/utils.h>
#include <vapor/DC.h>

#ifndef _DCMPAS_H_
    #define _DCMPAS_H_

namespace VAPoR {

class DerivedCoordVar_WRFTime;

//!
//! \class DCMPAS
//! \ingroup Public_VDCMPAS
//!
//! \brief Class for reading a NetCDF Climate Forecast (MPAS)  data set
//! stored as a series
//! of NetCDF files.
//!
//! \author John Clyne
//! \date    March, 2015
//!
class VDF_API DCMPAS : public VAPoR::DC {
public:
    //! Class constuctor
    //!
    //!
    DCMPAS();
    virtual ~DCMPAS();

protected:
    //! Initialize the DCMPAS class
    //!
    //! Prepare a MPAS data set for reading. This method prepares
    //! the DCMPAS class for reading the files indicated by
    //! \p paths.
    //! The method should be called immediately after the constructor,
    //! before any other class methods. This method
    //! exists only because C++ constructors can not return error codes.
    //!
    //! \param[in] path A list of MPAS NetCDF files comprising the output of
    //! a single MPAS model run.
    //!
    //! \retval status A negative int is returned on failure
    //!
    //! \sa EndDefine();
    //
    virtual int initialize(const vector<string> &paths, const std::vector<string> &options);

    //! \copydoc DC::GetDimension()
    //!
    virtual bool getDimension(string dimname, DC::Dimension &dimension) const;

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
    //
    virtual bool getAuxVarInfo(string varname, DC::AuxVar &var) const;

    //! \copydoc DC::GetBaseVarInfo()
    //
    virtual bool getBaseVarInfo(string varname, DC::BaseVar &var) const;

    //! \copydoc DC::GetDataVarNames()
    //!
    virtual std::vector<string> getDataVarNames() const;

    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual std::vector<string> getCoordVarNames() const;

    virtual std::vector<string> getAuxVarNames() const;

    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual size_t getNumRefLevels(string varname) const { return (1); }

    //! \copydoc DC::GetMapProjection()
    //!
    virtual string getMapProjection() const { return ("+proj=eqc +ellps=WGS84 +lon_0=0.0 +lat_0=0.0"); }

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

    virtual int openVariableRead(size_t ts, string varname, int, int);

    //! \copydoc DC::CloseVariable()
    //!
    virtual int closeVariable(int fd);

    //! \copydoc DC::ReadRegion()
    //
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionTemplate(fd, min, max, region)); }
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }

    //! \copydoc DC::ReadRegionBlock()
    //!
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionTemplate(fd, min, max, region)); }

    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }

    //! \copydoc DC::VariableExists()
    //!
    virtual bool variableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const;

private:
    NetCDFCollection *_ncdfc;
    VAPoR::UDUnits    _udunits;
    DerivedVarMgr     _dvm;
    bool              _hasVertical;    // has 3D data (i.e. vertical data)

    class MPASFileObject : public DC::FileTable::FileObject {
    public:
        MPASFileObject(size_t ts, string varname, int level, int lod, int fd, bool derivedFlag) : FileObject(ts, varname, level, lod, fd), _derivedFlag(derivedFlag) {}

        bool GetDerivedFlag() const { return (_derivedFlag); }

    private:
        bool _derivedFlag;
    };

    std::map<string, DC::Dimension> _dimsMap;
    std::map<string, DC::CoordVar>  _coordVarsMap;
    std::map<string, DC::Mesh>      _meshMap;
    std::map<string, DC::DataVar>   _dataVarsMap;
    std::map<string, DC::AuxVar>    _auxVarsMap;
    std::vector<string>             _cellVars;
    std::vector<string>             _pointVars;
    std::vector<string>             _edgeVars;
    Wasp::SmartBuf                  _nEdgesOnCellBuf;
    Wasp::SmartBuf                  _lonCellSmartBuf;
    Wasp::SmartBuf                  _lonVertexSmartBuf;

    int _InitDerivedVars(NetCDFCollection *ncdfc);
    int _InitCoordvars(NetCDFCollection *ncdfc);

    int _InitVerticalCoordinatesDerivedAtmosphere(NetCDFCollection *ncdfc);
    int _InitVerticalCoordinatesDerivedOcean(NetCDFCollection *ncdfc);

    int  _CheckRequiredFields(NetCDFCollection *ncdfc) const;
    bool _HasVertical(NetCDFCollection *ncdfc) const;

    int _InitDimensions(NetCDFCollection *ncdfc);

    int _GetVarCoordinates(NetCDFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar);

    int _InitMeshes(NetCDFCollection *ncdfc);
    int _InitAuxVars(NetCDFCollection *ncdfc);
    int _InitDataVars(NetCDFCollection *ncdfc);

    vector<string> _GetSpatialDimNames(NetCDFCollection *ncdfc, string varname) const;

    bool _isAtmosphere(NetCDFCollection *ncdfc) const;
    bool _isOcean(NetCDFCollection *ncdfc) const;

    bool _isCoordVar(string varname) const;
    bool _isDataVar(string varname) const;

    int  _read_nEdgesOnCell(size_t ts);
    void _addMissingFlag(int *data) const;
    int  _readVarToSmartBuf(size_t ts, string varname, Wasp::SmartBuf &smartBuf);
    int  _readCoordinates(size_t ts);

    void _splitOnBoundary(string varname, int *connData) const;

    int _readRegionTransposed(MPASFileObject *w, const vector<size_t> &min, const vector<size_t> &max, float *region);

    int _readRegionEdgeVariable(MPASFileObject *w, const vector<size_t> &min, const vector<size_t> &max, float *region);

    template<class T> int _readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region);

    template<class T> bool _getAttTemplate(string varname, string attname, T &values) const;

    // Derive vertical coordinate variable for dual mesh from primary mesh
    //
    class DerivedCoordVertFromCell : public DerivedCoordVar {
    public:
        DerivedCoordVertFromCell(string derivedVarName, string derivedDimName, DC *dc, string inName, string cellsOnVertexName);

        int Initialize();

        bool GetBaseVarInfo(DC::BaseVar &var) const;

        bool GetCoordVarInfo(DC::CoordVar &cvar) const;

        virtual std::vector<string> GetInputs() const { return (std::vector<string>{_inName}); }

        int GetDimLensAtLevel(int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

        int OpenVariableRead(size_t ts, int, int);

        int CloseVariable(int fd);

        int ReadRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

        int ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region);

        bool VariableExists(size_t ts, int, int) const;

    private:
        string       _derivedDimName;
        DC *         _dc;
        string       _inName;
        string       _cellsOnVertexName;
        DC::CoordVar _coordVarInfo;

        float *_getCellData();

        int *_getCellsOnVertex(size_t i0, size_t i1, int &vertexDegree);
    };

    // Derive Uzonal and Umeridional data variable
    //
    class DerivedZonalMeridonal : public DerivedDataVar {
    public:
        DerivedZonalMeridonal(string derivedVarName, DC *dc, NetCDFCollection *ncdfc, string normalVarName, string tangentialVarName, bool zonalFlag);

        int Initialize();

        bool GetBaseVarInfo(DC::BaseVar &var) const;

        bool GetDataVarInfo(DC::DataVar &cvar) const;

        virtual std::vector<string> GetInputs() const { return (std::vector<string>{_normalVarName, _tangentialVarName}); }

        int GetDimLensAtLevel(int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

        int OpenVariableRead(size_t ts, int, int);

        int CloseVariable(int fd);

        int ReadRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

        int ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region);

        bool VariableExists(size_t ts, int, int) const;

    private:
        DC *              _dc;
        NetCDFCollection *_ncdfc;
        string            _normalVarName;
        string            _tangentialVarName;
        bool              _zonalFlag;
        DC::DataVar       _dataVarInfo;
    };
};
};    // namespace VAPoR

#endif
