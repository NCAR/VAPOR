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

#ifndef _DCCF_H_
#define _DCCF_H_

namespace VAPoR {

//!
//! \class DCCF
//! \ingroup Public_VDCCF
//!
//! \brief Class for reading a NetCDF Climate Forecast (CF)  data set
//! stored as a series
//! of NetCDF files.
//!
//! \author John Clyne
//! \date    March, 2015
//!
class VDF_API DCCF : public VAPoR::DC {
  public:
    //! Class constuctor
    //!
    //!
    DCCF();
    virtual ~DCCF();

  protected:
    //! Initialize the DCCF class
    //!
    //! Prepare a CF data set for reading. This method prepares
    //! the DCCF class for reading the files indicated by
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
    virtual int initialize(
        const vector<string> &paths, const std::vector<string> &options);

    //! \copydoc DC::getDimension()
    //!
    virtual bool getDimension(
        string dimname, DC::Dimension &dimension) const;

    //! \copydoc DC::getDimensionNames()
    //!
    virtual std::vector<string> getDimensionNames() const;

    //! \copydoc DC::getMeshNames()
    //!
    std::vector<string> getMeshNames() const;

    //! \copydoc DC::getMesh()
    //!
    virtual bool getMesh(
        string mesh_name, DC::Mesh &mesh) const;

    //! \copydoc DC::GetCoordVarInfo()
    //!
    virtual bool getCoordVarInfo(string varname, DC::CoordVar &cvar) const;

    //! \copydoc DC::GetDataVarInfo()
    //!
    virtual bool getDataVarInfo(string varname, DC::DataVar &datavar) const;

    //! \copydoc DC::GetAuxVarInfo()
    //!
    virtual bool getAuxVarInfo(string varname, DC::AuxVar &var) const {
        return (false);
    }

    //! \copydoc DC::GetBaseVarInfo()
    //
    virtual bool getBaseVarInfo(string varname, DC::BaseVar &var) const;

    //! \copydoc DC::GetDataVarNames()
    //!
    virtual std::vector<string> getDataVarNames() const;

    virtual std::vector<string> getAuxVarNames() const {
        return (vector<string>());
    }

    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual std::vector<string> getCoordVarNames() const;

    //! \copydoc DC::GetCoordVarNames()
    //!
    virtual size_t getNumRefLevels(string varname) const { return (1); }

    //! \copydoc DC::GetMapProjection(string)
    //!
    virtual string getMapProjection(string varname) const;

    //! \copydoc DC::GetMapProjection()
    //!
    virtual string getMapProjection() const;

    //! \copydoc DC::GetMapProjectionDefault()
    //!
    virtual string getMapProjectionDefault() const {
        return (_proj4StringDefault);
    }

    //! \copydoc DC::GetAtt()
    //!
    virtual bool getAtt(
        string varname, string attname, vector<double> &values) const;
    virtual bool getAtt(
        string varname, string attname, vector<long> &values) const;
    virtual bool getAtt(
        string varname, string attname, string &values) const;

    //! \copydoc DC::GetAttNames()
    //!
    virtual std::vector<string> getAttNames(string varname) const;

    //! \copydoc DC::GetAttType()
    //!
    virtual XType getAttType(string varname, string attname) const;

    //! \copydoc DC::GetDimLensAtLevel()
    //!
    virtual int getDimLensAtLevel(
        string varname, int level, std::vector<size_t> &dims_at_level,
        std::vector<size_t> &bs_at_level) const;

    //! \copydoc DC::OpenVariableRead()
    //!
    virtual int openVariableRead(
        size_t ts, string varname, int, int) {
        return (DCCF::openVariableRead(ts, varname));
    }

    virtual int openVariableRead(
        size_t ts, string varname);

    //! \copydoc DC::CloseVariable()
    //!
    virtual int closeVariable();

    //! \copydoc DC::Read()
    //!
    int virtual read(float *data);
    virtual int read(int *data) {
        return (_ncdfc->Read(data, _ovr_fd));
    }

    //! \copydoc DC::ReadSlice()
    //!
    virtual int readSlice(float *slice);

    //! \copydoc DC::ReadRegion()
    //
    virtual int readRegion(
        const vector<size_t> &min, const vector<size_t> &max, float *region) {
        return (_readRegionTemplate(min, max, region));
    }
    virtual int readRegion(
        const vector<size_t> &min, const vector<size_t> &max, int *region) {
        return (_readRegionTemplate(min, max, region));
    }

    //! \copydoc DC::ReadRegionBlock()
    //!
    virtual int readRegionBlock(
        const vector<size_t> &min, const vector<size_t> &max, float *region);
    virtual int readRegionBlock(
        const vector<size_t> &min, const vector<size_t> &max, int *region) {
        return (DCCF::read(region));
    }

    //! \copydoc DC::VariableExists()
    //!
    virtual bool variableExists(
        size_t ts,
        string varname,
        int reflevel = 0,
        int lod = 0) const;

  private:
    NetCDFCFCollection *_ncdfc;
    VAPoR::UDUnits _udunits;
    Wasp::SmartBuf _buf;

    int _ovr_fd; // File descriptor for currently opened file

    string _proj4StringOption;
    string _proj4StringDefault;
    string _proj4String;
    std::map<string, DC::Dimension> _dimsMap;
    std::map<string, DC::CoordVar> _coordVarsMap;
    std::map<string, DC::Mesh> _meshMap;
    std::map<string, DC::DataVar> _dataVarsMap;
    std::map<string, string> _coordVarKeys;
    Proj4API *_proj4API;
    std::vector<NetCDFCollection::DerivedVar *> _derivedVars;

    int _get_latlon_coordvars(
        NetCDFCFCollection *ncdfc, string dvar, string &loncvar, string &latcvar) const;

    int _get_latlon_extents(
        NetCDFCFCollection *ncdfc, string latlon, bool lonflag,
        float &min, float &max);

    int _get_coord_pair_extents(
        NetCDFCFCollection *ncdfc, string lon, string lat,
        double &lonmin, double &lonmax, double &latmin, double &latmax);

    Proj4API *_create_proj4api(
        double lonmin, double lonmax, double latmin, double latmax,
        string &proj4string) const;

    int _get_vertical_coordvar(
        NetCDFCFCollection *ncdfc, string dvar, string &cvar);

    int _get_time_coordvar(
        NetCDFCFCollection *ncdfc, string dvar, string &cvar);

    int _AddCoordvars(
        NetCDFCFCollection *ncdfc, const vector<string> &cvars);

    int _InitHorizontalCoordinates(NetCDFCFCollection *ncdfc);
    int _InitHorizontalCoordinatesDerived(
        NetCDFCFCollection *ncdfc, const vector<pair<string, string>> &coordpairs);

    int _InitVerticalCoordinates(NetCDFCFCollection *ncdfc);
    int _InitVerticalCoordinatesDerived(
        NetCDFCFCollection *ncdfc, const vector<string> &cvars);

    int _InitTimeCoordinates(NetCDFCFCollection *ncdfc);
    int _InitTimeCoordinatesDerived(
        NetCDFCFCollection *ncdfc, const vector<string> &cvars);

    int _InitDimensions(NetCDFCFCollection *ncdfc);

    int _GetVarCoordinates(
        NetCDFCFCollection *ncdfc, string varname,
        vector<string> &sdimnames,
        vector<string> &scoordvars,
        string &time_dim_name,
        string &time_coordvar);

    int _InitVars(NetCDFCFCollection *ncdfc);

    template <class T>
    int _readRegionTemplate(
        const vector<size_t> &min, const vector<size_t> &max, T *region);

    ///////////////////////////////////////////////////////////////////////////
    //
    //	Specializations of the NetCDFCollection::DerivedVar class used to
    // support derived variables - required variables that are not
    // found in the CF data.
    //
    ///////////////////////////////////////////////////////////////////////////

    //
    // Horizontal coordinate  derived variables. This class computes
    // horizontal coordinate variables in meters by using a map projection
    // from geographic to cartographic coordinates
    //
    class DerivedVarHorizontal : public NetCDFCollection::DerivedVar {
      public:
        DerivedVarHorizontal(
            NetCDFCFCollection *ncdfc, string lonname, string latname,
            const vector<DC::Dimension> &dims, Proj4API *proj4API, bool lonflag);
        virtual ~DerivedVarHorizontal();

        virtual int Open(size_t ts);
        virtual int ReadSlice(float *slice, int);
        virtual int Read(float *buf, int);
        virtual int SeekSlice(int offset, int whence, int);
        virtual int Close(int fd);
        virtual bool TimeVarying() const { return (!_time_dim_name.empty()); };
        virtual std::vector<size_t> GetSpatialDims() const { return (_sdims); }
        virtual std::vector<string> GetSpatialDimNames() const { return (_sdimnames); }
        virtual size_t GetTimeDim() const { return (_time_dim); }
        virtual string GetTimeDimName() const { return (_time_dim_name); }
        virtual bool GetMissingValue(double &mv) const { return (false); }

      private:
        string _lonname;                // name of longitude variable
        string _latname;                // name of latitude variable
        bool _xflag;                    // calculate X or Y Cartographic coordinates?
        size_t _time_dim;               // number of time steps
        string _time_dim_name;          // Name of time dimension
        std::vector<size_t> _sdims;     // spatial dimensions
        std::vector<string> _sdimnames; // spatial dimension names
        bool _is_open;                  // Open for reading?
        float *_lonbuf;                 // boundary points of lat and lon
        float *_latbuf;                 // boundary points of lat and lon
        Proj4API *_proj4API;
        int _lonfd;
        int _latfd; // file descriptors for reading lat and lon coord vars
    };
};
}; // namespace VAPoR

#endif
