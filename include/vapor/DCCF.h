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
    
    int BuildCache();
    int Reinitialize();

protected:
    NetCDFCFCollection *_ncdfc = nullptr;
    vector<string> _paths;

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
    virtual int initialize(const vector<string> &paths, const std::vector<string> &options, NetCDFCFCollection *ncdfc);

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
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, double *region) { return (_readRegionTemplate(fd, min, max, region)); }
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }

    //! \copydoc DC::VariableExists()
    //!
    virtual bool variableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const;

    virtual int initDimensions(NetCDFCFCollection *ncdfc, std::map<string, DC::Dimension> &dimsMap);

    virtual int initCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap);

    virtual int addCoordvars(NetCDFCFCollection *ncdfc, const vector<string> &cvars, std::map<string, DC::CoordVar> &coordVarsMap);

    virtual int initDataVars(NetCDFCFCollection *ncdfc, std::map<string, DC::DataVar> &dataVarsMap);

    virtual int initAuxilliaryVars(NetCDFCFCollection *ncdfc, std::map<string, DC::AuxVar> &auxVarsMap);

    virtual int initMesh(NetCDFCFCollection *ncdfc, std::map<string, DC::Mesh> &_meshMap);

    virtual int getVarCoordinates(NetCDFCFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar) const;


private:
    VAPoR::UDUnits _udunits;

    string                                      _proj4String;
    std::map<string, DC::Dimension>             _dimsMap;
    std::map<string, DC::CoordVar>              _coordVarsMap;
    std::map<string, DC::Mesh>                  _meshMap;
    std::map<string, DC::DataVar>               _dataVarsMap;
    std::map<string, DC::AuxVar>                _auxVarsMap;

    int _initHorizontalCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap);

    int _initVerticalCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap);

    int _initTimeCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap);

    // Return true if a 1D variable has uniform, absolute deltas between elements
    //
    bool _isUniform(NetCDFCFCollection *ncdfc, string varname);


    template<class T> int _readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region);

    template<class T> bool _getAttTemplate(string varname, string attname, T &values) const;

};
};    // namespace VAPoR

#endif
