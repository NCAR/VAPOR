#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/NetCDFCollection.h>
#include <vapor/Proj4API.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/utils.h>
#include <vapor/DC.h>

#ifndef _DCMPAS_H_
    #define _DCMPAS_H_

namespace VAPoR {

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
    virtual int Initialize(const vector<string> &paths);

    //! \copydoc DC::GetDimension()
    //!
    virtual bool GetDimension(string dimname, DC::Dimension &dimension) const;

    //! \copydoc DC::GetDimensionNames()
    //!
    virtual std::vector<string> GetDimensionNames() const;

    std::vector<string> GetMeshNames() const;

    virtual bool GetMesh(string mesh_name, DC::Mesh &mesh) const;

    //! \copydoc DC::GetCoordVarInfo()
    //!
    virtual bool GetCoordVarInfo(string varname, DC::CoordVar &cvar) const;

    //! \copydoc DC::GetDataVarInfo()
    //!
    virtual bool GetDataVarInfo(string varname, DC::DataVar &datavar) const;

    //! \copydoc DC::GetBaseVarInfo()
    //
    virtual bool GetBaseVarInfo(string varname, DC::BaseVar &var) const;

    //! \copydoc DC::GetDataVarNames()
    //!
    virtual std::vector<string> GetDataVarNames() const;

    // override parent class!!
    virtual std::vector<string> GetDataVarNames(int ndim, bool spatial) const;

    virtual std::vector<string> GetAuxVarNames() const { return (vector<string>()); }

    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual std::vector<string> GetCoordVarNames() const;

    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual size_t GetNumRefLevels(string varname) const { return (1); }

    //! \copydoc DC::GetMapProjection(string)
    //!
    virtual string GetMapProjection(string varname) const { return (_proj4String); }

    //! \copydoc DC::GetMapProjection()
    //!
    virtual string GetMapProjection() const { return (_proj4String); }

    //! \copydoc DC::GetAtt()
    //!
    virtual bool GetAtt(string varname, string attname, vector<double> &values) const;
    virtual bool GetAtt(string varname, string attname, vector<long> &values) const;
    virtual bool GetAtt(string varname, string attname, string &values) const;

    //! \copydoc DC::GetAttNames()
    //!
    virtual std::vector<string> GetAttNames(string varname) const;

    //! \copydoc DC::GetAttType()
    //!
    virtual XType GetAttType(string varname, string attname) const;

    //! \copydoc DC::GetDimLensAtLevel()
    //!
    virtual int GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    //! \copydoc DC::OpenVariableRead()
    //!
    virtual int OpenVariableRead(size_t ts, string varname, int, int) { return (DCMPAS::OpenVariableRead(ts, varname)); }

    virtual int OpenVariableRead(size_t ts, string varname);

    //! \copydoc DC::CloseVariable()
    //!
    virtual int CloseVariable();

    //! \copydoc DC::Read()
    //!
    virtual int Read(float *data);
    virtual int Read(int *data) { return (_ncdfc->Read(data, _ovr_fd)); }

    //! \copydoc DC::ReadSlice()
    //!
    virtual int ReadSlice(float *slice);

    //! \copydoc DC::ReadRegion()
    //
    virtual int ReadRegion(const vector<size_t> &min, const vector<size_t> &max, float *region);

    //! \copydoc DC::ReadRegionBlock()
    //!
    virtual int ReadRegionBlock(const vector<size_t> &min, const vector<size_t> &max, float *region);
    virtual int ReadRegionBlock(const vector<size_t> &min, const vector<size_t> &max, int *region) { return (DCMPAS::Read(region)); }

    //! \copydoc DC::GetVar()
    //!
    virtual int GetVar(string varname, int, int, float *data) { return (DCMPAS::GetVar(varname, data)); }

    virtual int GetVar(string varname, int, int, int *data)
    {
        SetErrMsg("Not implemented");
        return (-1);
    }

    virtual int GetVar(string varname, float *data);

    //! \copydoc DC::GetVar()
    //!
    virtual int GetVar(size_t ts, string varname, int, int, float *data) { return (DCMPAS::GetVar(ts, varname, data)); }

    virtual int GetVar(size_t ts, string varname, int, int, int *data)
    {
        SetErrMsg("Not implemented");
        return (-1);
    }

    virtual int GetVar(size_t ts, string varname, float *data);

    //! \copydoc DC::VariableExists()
    //!
    virtual bool VariableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const;

private:
    NetCDFCollection *_ncdfc;
    VAPoR::UDUnits    _udunits;

    int _ovr_fd;    // File descriptor for currently opened file

    string                                      _proj4String;
    Proj4API *                                  _proj4API;
    std::map<string, DC::Dimension>             _dimsMap;
    std::map<string, DC::CoordVar>              _coordVarsMap;
    std::map<string, DC::Mesh>                  _meshMap;
    std::map<string, DC::DataVar>               _dataVarsMap;
    std::map<string, DC::AuxVar>                _auxVarsMap;
    std::vector<NetCDFCollection::DerivedVar *> _derivedVars;
    std::vector<string>                         _cellVars;
    std::vector<string>                         _pointVars;
    std::vector<string>                         _edgeVars;

    Proj4API *_create_proj4api(double lonmin, double lonmax, double latmin, double latmax, string &proj4string) const;

    int _InitDerivedVars(NetCDFCollection *ncdfc);
    int _InitCoordvars(NetCDFCollection *ncdfc);

    int _InitHorizontalCoordinatesDerived(NetCDFCollection *ncdfc);

    int _InitVerticalCoordinatesDerived(NetCDFCollection *ncdfc);

    int _CheckRequiredFields(NetCDFCollection *ncdfc) const;

    int _InitDimensions(NetCDFCollection *ncdfc);

    int _GetVarCoordinates(NetCDFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar);

    int _InitMeshes(NetCDFCollection *ncdfc);
    int _InitAuxVars(NetCDFCollection *ncdfc);
    int _InitDataVars(NetCDFCollection *ncdfc);

    ///////////////////////////////////////////////////////////////////////////
    //
    //	Specializations of the NetCDFCollection::DerivedVar class used to
    // support derived variables - required variables that are not
    // found in the MPAS data.
    //
    ///////////////////////////////////////////////////////////////////////////

    //
    // Horizontal coordinate  derived variables. This class computes
    // horizontal coordinate variables in meters by using a map projection
    // from geographic to cartographic coordinates
    //
    class DerivedVarHorizontal : public NetCDFCollection::DerivedVar {
    public:
        DerivedVarHorizontal(NetCDFCollection *ncdfc, string lonname, string latname, Proj4API *proj4API, bool lonflag, bool uGridFlag);
        virtual ~DerivedVarHorizontal();

        virtual int                 Open(size_t ts);
        virtual int                 ReadSlice(float *slice, int);
        virtual int                 Read(float *buf, int);
        virtual int                 SeekSlice(int offset, int whence, int);
        virtual int                 Close(int fd);
        virtual bool                TimeVarying() const { return (!_time_dim_name.empty()); };
        virtual std::vector<size_t> GetSpatialDims() const { return (_sdims); }
        virtual std::vector<string> GetSpatialDimNames() const { return (_sdimnames); }
        virtual size_t              GetTimeDim() const { return (_time_dim); }
        virtual string              GetTimeDimName() const { return (_time_dim_name); }
        virtual bool                GetMissingValue(double &mv) const { return (false); }

    private:
        string              _lonname;          // name of longitude variable
        string              _latname;          // name of latitude variable
        bool                _xflag;            // calculate X or Y Cartographic coordinates?
        bool                _uGridFlag;        // unstructured grid?
        bool                _oneDFlag;         // structured grid with lat and lon functions of one variable
        size_t              _time_dim;         // number of time steps
        string              _time_dim_name;    // Name of time dimension
        std::vector<size_t> _sdims;            // spatial dimensions
        std::vector<string> _sdimnames;        // spatial dimension names
        bool                _is_open;          // Open for reading?
        float *             _lonbuf;           // boundary points of lat and lon
        float *             _latbuf;           // boundary points of lat and lon
        Proj4API *          _proj4API;
        int                 _lonfd;
        int                 _latfd;    // file descriptors for reading lat and lon coord vars
    };

    //
    // Vertical coordinate  derived variables. This class computes
    // vertical coordinate variables in meters for the unstaggered grid
    // from the staggered grid.
    //
    class DerivedVarVertical : public NetCDFCollection::DerivedVar {
    public:
        DerivedVarVertical(NetCDFCollection *ncdfc, string staggeredVarName, string zDimNameUnstaggered);
        virtual ~DerivedVarVertical();

        virtual int                 Open(size_t ts);
        virtual int                 ReadSlice(float *slice, int);
        virtual int                 Read(float *buf, int);
        virtual int                 SeekSlice(int offset, int whence, int);
        virtual int                 Close(int fd);
        virtual bool                TimeVarying() const { return (!_time_dim_name.empty()); };
        virtual std::vector<size_t> GetSpatialDims() const { return (_sdims); }
        virtual std::vector<string> GetSpatialDimNames() const { return (_sdimnames); }
        virtual size_t              GetTimeDim() const { return (_time_dim); }
        virtual string              GetTimeDimName() const { return (_time_dim_name); }
        virtual bool                GetMissingValue(double &mv) const { return (false); }

    private:
        string              _staggeredVarName;    // name of unstaggered vertical coord var
        size_t              _time_dim;            // number of time steps
        string              _time_dim_name;       // Name of time dimension
        std::vector<size_t> _sdims;               // spatial dimensions
        std::vector<string> _sdimnames;           // spatial dimension names
        bool                _is_open;             // Open for reading?
        float *             _buf;                 // boundary points of lat and lon
        int                 _fd;                  // file descriptors for reading coord vars
    };

    //
    // Time coordinate derived variables. This class maps a WRF-style
    // time coordinate variable (a formatted string) into a valid
    // time coordinate.
    //
    class DerivedVarWRFTime : public NetCDFCollection::DerivedVar {
    public:
        DerivedVarWRFTime(NetCDFCollection *ncdfc, const VAPoR::UDUnits *udunits, string wrfTimeVar);
        virtual ~DerivedVarWRFTime();

        virtual int                 Open(size_t ts);
        virtual int                 ReadSlice(float *slice, int);
        virtual int                 Read(float *buf, int);
        virtual int                 SeekSlice(int offset, int whence, int);
        virtual int                 Close(int fd);
        virtual bool                TimeVarying() const { return (true); }
        virtual std::vector<size_t> GetSpatialDims() const { return (std::vector<size_t>()); }
        virtual std::vector<string> GetSpatialDimNames() const { return (std::vector<string>()); }
        virtual size_t              GetTimeDim() const { return (_time_dim); }
        virtual string              GetTimeDimName() const { return (_time_dim_name); }
        virtual bool                GetMissingValue(double &mv) const { return (false); }

    private:
        const VAPoR::UDUnits *_udunits;
        string                _wrfTimeVar;
        size_t                _time_dim;         // number of time steps
        string                _time_dim_name;    // Name of time dimension
        bool                  _is_open;          // Open for reading?
        char *                _buf;              // boundary points of lat and lon
        int                   _fd;
    };
};
};    // namespace VAPoR

#endif
