#pragma once

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
#include <vapor/BOVCollection.h>

namespace VAPoR {

class BOVCollection;

// Example BOV header file, bovA1.bov
//
// # TIME is a floating point value specifying the timestep being read in DATA_FILE
// TIME: 1.1
//
// # DATA_FILE points to a binary data file.  It can be a full file path, or a path relative to the BOV header.
// DATA_FILE: bovA1.bin
//
// # The data size corresponds to NX,NY,NZ in the above example code.  It must contain three values
// DATA_SIZE: 10 10 10
//
// # Allowable values for DATA_FORMAT are: INT,FLOAT,DOUBLE
// DATA_FORMAT: FLOAT
//
// # VARIABLE is a string that specifies the variable being read in DATA_FILE.  Must be alphanumeric (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-)
// VARIABLE: myVariable
//
// # BRICK_ORIGIN lets you specify a new coordinate system origin for # the mesh that will be created to suit your data.  It must contain three values.
// BRICK_ORIGIN: 0. 0. 0.
//
// # BRICK_SIZE lets you specify the size of the brick on X, Y, and Z.  It must contain three values.
// BRICK_SIZE: 10. 20. 5.
//
// # BYTE_OFFSET is optional and lets you specify some number of
// # bytes to skip at the front of the file. This can be useful for # skipping the 4-byte header that Fortran tends to write to files. # If your file does not have a header then DO NOT USE
// BYTE_OFFSET. BYTE_OFFSET: 4

//!
//! \class DCBOV
//! \ingroup Public_VDCBOV
//!
//! \brief Class for reading a "Brick of Values", explained in section 3.1 (page 11) in the
//! following VisIt document: https://github.com/NCAR/VAPOR/files/6341067/GettingDataIntoVisIt2.0.0.pdf
//!
//! The following BOV tags are mandatory for Vapor to ingest data:
//! - DATA_FILE    (type: string)
//! - DATA_SIZE    (type: three integer values that are >1)
//! - DATA_FORMAT  (type: string of either INT, FLOAT, or DOUBLE)
//! - TIME         (type: one floating point value.  May not be equal to FLT_MIN)
//!
//! The following BOV tags are optional:
//! - BRICK_ORIGIN (type: three floating point values,   default: 0., 0., 0.)
//! - BRICK_SIZE   (type: three floating point values,   default: 1., 1., 1.)
//! - VARIABLE     (type: one alphanumeric string value, default: "brickVar")
//! - BYTE_OFFSET  (type: one integer value,             default: 0)
//!
//! The following BOV tags are currently unsupported.  They can be included in a BOV header,
//! but they will be unused.
//! - DATA_ENDIAN
//! - CENTERING
//! - DIVIDE_BRICK
//! - DATA_BRICKLETS
//! - DATA_COMPONENTS
//!
//! Each .bov file can only refer to a single data file for a single variable, at a single timestep.
//! If duplicate key/value pairs exist in a BOV header, the value closest to the bottom of the file will be used.
//! If duplicate values exist for whatever reason, all entries must be valid (except for DATA_FILE, which gets validated after parsing)
//! Scientific notation is supported for floating point values like BRICK_ORIGIN and BRICK_SIZE.
//! Scientific notation is not supported for integer values like DATA_SIZE.
//! DATA_SIZE must contain three values greater than 1.
//! Wild card characters are not currently supported in the DATA_FILE token.
//! VARIABLE must be alphanumeric (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-)
//!
//! \author Scott Pearse
//! \date    May, 2021
//!
class VDF_API DCBOV : public VAPoR::DC {
public:
    //! Class constuctor
    //!
    //!
    DCBOV();
    virtual ~DCBOV();

protected:
    //! Initialize the DCBOV class
    //!
    //! Prepare a BOV data set for reading. This method prepares
    //! the DCBOV class for reading the files indicated by
    //! \p paths.
    //! The method should be called immediately after the constructor,
    //! before any other class methods. This method
    //! exists only because C++ constructors can not return error codes.
    //!
    //! \param[in] path A single BOV header file describing a the structore of a "Brick
    //! of raw IEEE floating point numbers stored in a file.
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
    virtual string getMapProjection() const { return (""); }

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
    virtual int openVariableRead(size_t ts, string varname, int, int) { return (DCBOV::openVariableRead(ts, varname)); }

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
    VAPoR::UDUnits _udunits;

    BOVCollection *_bovCollection;

    string                          _varname;
    std::map<string, DC::Dimension> _dimsMap;
    std::map<string, DC::CoordVar>  _coordVarsMap;
    std::map<string, DC::Mesh>      _meshMap;
    std::map<string, DC::DataVar>   _dataVarsMap;
    std::map<string, string>        _coordVarKeys;

    void _InitCoordinates();

    void _InitDimensions();

    void _InitVars();

    int _isCoordinateVariable(std::string variable) const;

    template<class T> void _generateCoordinates(int dim, const vector<size_t> &min, const vector<size_t> &max, T *region) const;

    template<class T> int _readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region);
};
}    // namespace VAPoR
