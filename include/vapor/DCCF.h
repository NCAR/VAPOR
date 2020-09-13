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
    virtual int initialize(const vector<string> &paths, const std::vector<string> &options);

    //! \copydoc DC::getDimension()
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
    //!
    virtual bool getAuxVarInfo(string varname, DC::AuxVar &var) const { return (false); }

    //! \copydoc DC::GetBaseVarInfo()
    //
    virtual bool getBaseVarInfo(string varname, DC::BaseVar &var) const;

    //! \copydoc DC::GetDataVarNames()
    //!
    virtual std::vector<string> getDataVarNames() const;

    virtual std::vector<string> getAuxVarNames() const { return (vector<string>()); }

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

    //! \copydoc DC::OpenVariableRead()
    //!
    virtual int openVariableRead(size_t ts, string varname, int, int) { return (DCCF::openVariableRead(ts, varname)); }

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
    NetCDFCFCollection *_ncdfc;
    VAPoR::UDUnits      _udunits;

    string                                      _proj4String;
    std::map<string, DC::Dimension>             _dimsMap;
    std::map<string, DC::CoordVar>              _coordVarsMap;
    std::map<string, DC::Mesh>                  _meshMap;
    std::map<string, DC::DataVar>               _dataVarsMap;
    std::map<string, string>                    _coordVarKeys;
    std::vector<NetCDFCollection::DerivedVar *> _derivedVars;

    int _get_vertical_coordvar(NetCDFCFCollection *ncdfc, string dvar, string &cvar);

    int _get_time_coordvar(NetCDFCFCollection *ncdfc, string dvar, string &cvar);

    int _get_latlon_coordvars(NetCDFCFCollection *ncdfc, string dvar, string &loncvar, string &latcvar) const;

    int _AddCoordvars(NetCDFCFCollection *ncdfc, const vector<string> &cvars);

    // Return true if a 1D variable has uniform, absolute deltas between elements
    //
    bool _isUniform(NetCDFCFCollection *ncdfc, string varname);

    int _InitHorizontalCoordinates(NetCDFCFCollection *ncdfc);

    int _InitVerticalCoordinates(NetCDFCFCollection *ncdfc);

    int _InitTimeCoordinates(NetCDFCFCollection *ncdfc);

    int _InitDimensions(NetCDFCFCollection *ncdfc);

    int _GetVarCoordinates(NetCDFCFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar);

    int _InitVars(NetCDFCFCollection *ncdfc);

    template<class T> int _readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region);

    template<class T> bool _getAttTemplate(string varname, string attname, T &values) const;

    ///////////////////////////////////////////////////////////////////////////
    //
    //	Specializations of the NetCDFCollection::DerivedVar class used to
    // support derived variables - required variables that are not
    // found in the CF data.
    //
    ///////////////////////////////////////////////////////////////////////////

    //
};
};    // namespace VAPoR

#endif
