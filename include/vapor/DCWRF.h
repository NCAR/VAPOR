#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/NetCDFCollection.h>
#include <vapor/Proj4API.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/DC.h>

#ifndef	_DCWRF_H_
#define	_DCWRF_H_

namespace VAPoR {


//!
//! \class DCWRF
//! \ingroup Public_VDCWRF
//!
//! \brief Class for reading a WRF data set stored as a series
//! of NetCDF files.
//!
//! \author John Clyne
//! \date    January, 2015
//!
class VDF_API DCWRF : public VAPoR::DC {
public:


 //! Class constuctor
 //!
 //!
 DCWRF();
 virtual ~DCWRF();

protected:
 //! Initialize the DCWRF class
 //!
 //! Prepare a WRF data set for reading. This method prepares
 //! the DCWRF class for reading the files indicated by
 //! \p paths.
 //! The method should be called immediately after the constructor, 
 //! before any other class methods. This method
 //! exists only because C++ constructors can not return error codes.
 //!
 //! \param[in] path A list of WRF NetCDF files comprising the output of 
 //! a single WRF model run.
 //!
 //! \retval status A negative int is returned on failure
 //!
 //! \sa EndDefine();
 //
 virtual int initialize(
	const vector <string> &paths, const std::vector <string> &options
 );


 //! \copydoc DC::GetDimension()
 //!
 virtual bool getDimension(
	string dimname, DC::Dimension &dimension
 ) const;

 //! \copydoc DC::getDimensionNames()
 //!
 virtual std::vector <string> getDimensionNames() const;

 //! \copydoc DC::getMeshNames()
 //!
 std::vector <string> getMeshNames() const;

 //! \copydoc DC::GetMesh()
 //!
 virtual bool getMesh(
	string mesh_name, DC::Mesh &mesh
 ) const;

 //! \copydoc DC::GetCoordVarInfo()
 //!
 virtual bool getCoordVarInfo(string varname, DC::CoordVar &cvar) const;

 //! \copydoc DC::GetDataVarInfo()
 //!
 virtual bool getDataVarInfo( string varname, DC::DataVar &datavar) const;

 //! \copydoc DC::GetBaseVarInfo()
 //
 virtual bool getAuxVarInfo(string varname, DC::AuxVar &var) const {
	return(false);
 }
 
 //! \copydoc DC::GetBaseVarInfo()
 //
 virtual bool getBaseVarInfo(string varname, DC::BaseVar &var) const;

 //! \copydoc DC::GetDataVarNames()
 //!
 virtual std::vector <string> getDataVarNames() const;

 //! \copydoc DC::GetCoordVarNames()
 //!
 virtual std::vector <string> getCoordVarNames() const;

 virtual std::vector <string> getAuxVarNames() const {
    return (vector <string> ());
 }

 //! \copydoc DC::GetNumRefLevels()
 //!
 virtual size_t getNumRefLevels(string varname) const { return(1); }

 //! \copydoc DC::GetMapProjection(string)
 //!
 virtual string getMapProjection(string varname) const;

 //! \copydoc DC::GetMapProjection()
 //!
 virtual string getMapProjection() const;

 //! \copydoc DC::GetMapProjection()
 //!
 virtual string getMapProjectionDefault() const {
	return(_proj4StringDefault);
 }


 //! \copydoc DC::GetAtt()
 //!
 virtual bool getAtt(
	string varname, string attname, vector <double> &values
 ) const;
 virtual bool getAtt(
	string varname, string attname, vector <long> &values
 ) const;
 virtual bool getAtt(
	string varname, string attname, string &values
 ) const;

 //! \copydoc DC::GetAttNames()
 //!
 virtual std::vector <string> getAttNames(string varname) const;

 //! \copydoc DC::GetAttType()
 //!
 virtual XType getAttType(string varname, string attname) const;

 //! \copydoc DC::GetDimLensAtLevel()
 //!
 virtual int getDimLensAtLevel(
	string varname, int level, std::vector <size_t> &dims_at_level,
	std::vector <size_t> &bs_at_level
 ) const;


 //! \copydoc DC::OpenVariableRead()
 //!
 virtual int openVariableRead(
	size_t ts, string varname, int , int 
 ) {
	return(DCWRF::openVariableRead(ts, varname));
 }

 virtual int openVariableRead(
	size_t ts, string varname
 );


 //! \copydoc DC::CloseVariable()
 //!
 virtual int closeVariable();

 //! \copydoc DC::Read()
 //!
 virtual int read(float *data);
 virtual int read(int *data) {
	return(_ncdfc->Read(data, _ovr_fd));
 }

 //! \copydoc DC::ReadSlice()
 //!
 virtual int readSlice(float *slice);

 //! \copydoc DC::ReadRegion()
 //
 virtual int readRegion(
    const vector <size_t> &min, const vector <size_t> &max, float *region
 ) {
	return(_readRegionTemplate(min, max, region));
 }
 virtual int readRegion(
    const vector <size_t> &min, const vector <size_t> &max, int *region
 ) {
	return(_readRegionTemplate(min, max, region));
 }

 //! \copydoc DC::ReadRegionBlock()
 //!
 virtual int readRegionBlock(
    const vector <size_t> &min, const vector <size_t> &max, float *region
 );
 virtual int readRegionBlock(
    const vector <size_t> &min, const vector <size_t> &max, int *region
 ) {
	return (DCWRF::read(region));
 }

 //! \copydoc DC::VariableExists()
 //!
 virtual bool variableExists(
    size_t ts,
    string varname,
    int reflevel = 0,
    int lod = 0
 ) const;

private:
 NetCDFCollection *_ncdfc;
 VAPoR::UDUnits _udunits;

 //
 // Various attributes from a WRF data file needed for computing map 
 // projections
 //
 float _dx;
 float _dy;
 float _cen_lat;
 float _cen_lon;
 float _pole_lat;
 float _pole_lon;
 float _grav;
 float _radius;
 float _p2si;
 float _mapProj;

 int _ovr_fd;	// File descriptor for currently opened file
 string _ovr_varname;	// name of currently opened variable
 string _proj4String;
 string _proj4StringOption;
 string _proj4StringDefault;
 Proj4API _proj4API;

 class DerivedVarHorizontal;
 DerivedVarHorizontal *_derivedX;
 DerivedVarHorizontal *_derivedY;
 DerivedVarHorizontal *_derivedXU;
 DerivedVarHorizontal *_derivedYU;
 DerivedVarHorizontal *_derivedXV;
 DerivedVarHorizontal *_derivedYV;

 class DerivedVarElevation;
 DerivedVarElevation *_derivedElev;
 DerivedVarElevation *_derivedElevU;
 DerivedVarElevation *_derivedElevV;
 DerivedVarElevation *_derivedElevW;

 class DerivedVarTime;
 DerivedVarTime *_derivedTime;

 std::map <string, DC::Dimension> _dimsMap;
 std::map <string, DC::CoordVar> _coordVarsMap;
 std::map <string, DC::Mesh> _meshMap;
 std::map <string, DC::DataVar> _dataVarsMap;
 std::vector <size_t> _timeLookup;


 vector <size_t> _GetSpatialDims(
	NetCDFCollection *ncdfc, string varname
 ) const;

 vector <string> _GetSpatialDimNames(
	NetCDFCollection *ncdfc, string varname
 ) const;

 int _InitAtts(NetCDFCollection *ncdfc);

 int _GetProj4String(
	NetCDFCollection *ncdfc, float radius, int map_proj, string &projstring
 ); 

 int _InitProjection(NetCDFCollection *ncdfc, float radius); 

 DerivedVarHorizontal *_InitHorizontalCoordinatesHelper(
	NetCDFCollection *ncdfc, Proj4API *proj4API,
	string name, string fastest_dim, int axis,
	vector <size_t> latlondims
 );

 int _InitHorizontalCoordinates(NetCDFCollection *ncdfc, Proj4API *proj4API); 

 DCWRF::DerivedVarElevation *_InitVerticalCoordinatesHelper(
	NetCDFCollection *ncdfc, string name, vector <string> sdimnames
 );

 int _InitVerticalCoordinates(NetCDFCollection *ncdfc); 

 int _InitTime(NetCDFCollection *ncdfc);

 int _InitDimensions(NetCDFCollection *ncdfc);

 int _GetCoordVars(
	NetCDFCollection *ncdfc,
	string varname, vector <string> &cvarnames
 ); 

 bool _GetVarCoordinates(
	NetCDFCollection *ncdfc, string varname,
	std::vector <string> &dimnames,
	std::vector <string> &coordvars,
	string &time_dim_name,
	string &time_coordvar
 ); 

 int _InitVars(NetCDFCollection *ncdfc);

 template <class T>
 int _readRegionTemplate(
	const vector <size_t> &min, const vector <size_t> &max, T *region
 );

 ///////////////////////////////////////////////////////////////////////////
 //
 //	Specializations of the NetCDFCollection::DerivedVar class used to 
 // support derived variables - required variables that are not 
 // found in the WRF data.
 //
 ///////////////////////////////////////////////////////////////////////////

 //
 // Elevation derived variable. This class computes a vertical coordinate
 // in meters from the pressure variables found in a WRF data set
 //
 class DerivedVarElevation : public VAPoR::NetCDFCollection::DerivedVar {
 public:
  DerivedVarElevation(
	NetCDFCollection *ncdfc, string name, 
	const vector <DC::Dimension> &dims, float grav
  );
  virtual ~DerivedVarElevation();

  virtual int Open(size_t ts);
  virtual int ReadSlice(float *slice, int );
  virtual int Read(float *buf, int );
  virtual int SeekSlice(int offset, int whence, int );
  virtual int Close(int fd);
  virtual bool TimeVarying() const {return(true); };
  virtual std::vector <size_t>  GetSpatialDims() const { return(_sdims); }
  virtual std::vector <string>  GetSpatialDimNames() const {return(_sdimnames);}
  virtual size_t  GetTimeDim() const {return(_time_dim); }
  virtual string  GetTimeDimName() const {return(_time_dim_name); }
  virtual bool GetMissingValue(double &mv) const { return(false); }

 private:
  string _name;	// name of derived variable
  size_t _time_dim; // number of time steps
  string _time_dim_name; // Name of time dimension
  std::vector <size_t> _sdims;	// spatial dimensions
  std::vector <string> _sdimnames;	// spatial dimension names
  float _grav;	// gravitational constant
  string _PHvar; // name of PH variable
  string _PHBvar; // name of PHB variable
  float *_PH; // buffer for PH variable slice
  float *_PHB; // buffer for PHB variable slice
  float *_zsliceBuf;	// buffer for z starggering interpolation
  float *_xysliceBuf;	// buffer for horizontal extrapolation
  int _PHfd;
  int _PHBfd;	// file descriptors
  bool _is_open;	// variable open for reading?
  bool _xextrapolate;	// need extrapolation along X?
  bool _yextrapolate;	// need extrapolation along Y?
  bool _zinterpolate;	// need interpolation along Z?
  std::vector <size_t>_ph_dims;	// spatial dims of PH and PHB variables
  bool _firstSlice; // true if first slice for current open variable

  int _ReadSlice(float *slice);
 };


 //
 // Horizontal coordinate  derived variables. This class computes 
 // horizontal coordinate variables in meters by using a map projection
 // from geographic to cartographic coordinates
 //
 class DerivedVarHorizontal : public NetCDFCollection::DerivedVar {
 public:
  DerivedVarHorizontal(
	NetCDFCollection *ncdfc, string name, const vector <DC::Dimension> &dims,
	const vector <size_t> latlondims, Proj4API *proj4API
  );
  virtual ~DerivedVarHorizontal();

  virtual int Open(size_t ts);
  virtual int ReadSlice(float *slice, int );
  virtual int Read(float *buf, int );
  virtual int SeekSlice(int offset, int whence, int );
  virtual int Close(int fd);
  virtual bool TimeVarying() const {return(true); };
  virtual std::vector <size_t>  GetSpatialDims() const { return(_sdims); }
  virtual std::vector <string>  GetSpatialDimNames() const {return(_sdimnames);}
  virtual size_t  GetTimeDim() const {return(_time_dim); }
  virtual string  GetTimeDimName() const {return(_time_dim_name); }
  virtual bool GetMissingValue(double &mv) const { return(false); }
 private:
  string _name;	// name of derived variable
  size_t _time_dim; // number of time steps
  string _time_dim_name; // Name of time dimension
  std::vector <size_t> _sdims;	// spatial dimensions
  std::vector <string> _sdimnames;	// spatial dimension names
  size_t _nx;
  size_t _ny;	// spatial dimensions  of XLONG and XLAT variables
  bool _is_open;	// Open for reading?
  float *_coords;	// cached coordinates
  float *_sliceBuf;	// space for reading lat an lon variables
  float *_lonBdryBuf;	// boundary points of lat and lon
  float *_latBdryBuf;	// boundary points of lat and lon
  size_t _ncoords;	// length of coords
  size_t _cached_ts;	// 	 time step of cached coordinate
  string _lonname;	
  string _latname;	// name of lat and lon coordinate variables
  Proj4API *_proj4API;
  bool _hasStagCVars;	// file has coord vars for staggered dimensions?

  int _GetCartCoords(size_t ts);
 };

 //
 // Time coordinate  derived variables. This class computes 
 // a time coordinate variable with units in seconds.
 //
 class DerivedVarTime : public NetCDFCollection::DerivedVar {
 public:
  DerivedVarTime(
	NetCDFCollection *ncdfc, DC::Dimension dims, 
	const std::vector <float> &timecoords
  );
  virtual ~DerivedVarTime() {}

  virtual int Open(size_t ts);
  virtual int ReadSlice(float *slice, int );
  virtual int Read(float *buf, int );
  virtual int SeekSlice(int offset, int whence, int );
  virtual int Close(int fd);
  virtual bool TimeVarying() const {return(true); };
  virtual std::vector <size_t>  GetSpatialDims() const { return(_sdims); }
  virtual std::vector <string>  GetSpatialDimNames() const {return(_sdimnames);}
  virtual size_t  GetTimeDim() const {return(_timecoords.size()); }
  virtual string  GetTimeDimName() const {return(_time_dim_name); }
  virtual bool GetMissingValue(double &mv) const { return(false); }
 private:
  string _time_dim_name; // Name of time dimension
  std::vector <size_t> _sdims;	// spatial dimensions
  std::vector <string> _sdimnames;	// spatial dimension names
  vector <float> _timecoords;	// cached coordinates
  size_t _ts;	// 	 current time step
 };

};
};

#endif
