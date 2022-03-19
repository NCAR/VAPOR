//
// $Id$
//

#ifndef _NetCDFCFCollection_h_
#define _NetCDFCFCollection_h_

#include <vector>
#include <map>
#include <algorithm>

#include <sstream>
#include <vapor/MyBase.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/NetCDFCollection.h>

union ut_unit;
struct ut_system;

namespace VAPoR {

//
//! \class NetCDFCFCollection
//! \brief Wrapper for a collection of netCDF files
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! This class provides access to CF-1 compliant collection of netCDF
//! files. This work is based on the "NetCDF Climate and Forecast
//! (CF) Metadata Conventions", version 1.6, 5, December 2011.
//

class VDF_API NetCDFCFCollection : public NetCDFCollection {
public:
    NetCDFCFCollection();
    virtual ~NetCDFCFCollection();

    virtual int Initialize(const std::vector<string> &files);

    //! Return boolean indicating whether variable is a CF "coordinate" variable
    //!
    //! This method returns true if the variable named by \p var is a
    //! "coordinate" variable as defined by the CF specification. I.e.
    //! the variable has a single spatial dimension whose name matches
    //! \p var name; the variable can be identified as a vertical, lat, lon,
    //! or time coordinate variable; or the variable is listed by some
    //! other variable's "coordinate" metadata attribute.
    //!
    //!
    //! \retval true if \p var is a coordinate variable, false otherwise
    //!
    //! \sa IsAuxCoordVarCF()
    //
    virtual bool IsCoordVarCF(string var) const { return (std::find(_coordinateVars.begin(), _coordinateVars.end(), var) != _coordinateVars.end()); }

    //! Return boolean indicating whether the named variable is a CF auxiliary
    //! coordinate variable
    //!
    //! This method returns true if the variable named by \p var is a
    //! "auxiliary coordinate variable"
    //!
    //! CF1.X Definition of <em> auxiliary coordinate variable </em>:
    //!
    //! Any netCDF variable that contains coordinate data, but is not
    //! a coordinate variable (in the sense of that term defined by the
    //! NUG and used by this standard - see below). Unlike coordinate
    //! variables, there is no relationship between the name of an auxiliary
    //! coordinate variable and the name(s) of its dimension(s).
    //!
    //! \retval true if \p var is an auxliary coordinate variable, false otherwise
    //
    virtual bool IsAuxCoordVarCF(string var) const { return (std::find(_auxCoordinateVars.begin(), _auxCoordinateVars.end(), var) != _auxCoordinateVars.end()); }


    //! Return boolean indicating whether the named variable is a
    //! NetCDF CF "coordinate" variable or a "auxilliary" coordinate
    //! variable.
    //!
    //! \sa IsAuxCoordVarCF(), IsCoordVarCF()
    //
    virtual bool IsCoordinateVar(string varName) const { return (IsCoordVarCF(varName) || IsAuxCoordVarCF(varName)); }



    //! Return boolean indicating whether the named variable represents
    //! latitude.
    //!
    //! This method returns true if the variable named by \p var is a
    //! latitude coordinate variable. See section 4.1 of the CF spec.
    //!
    //! \retval true if \p var is latitude coordinate variable, false otherwise
    //
    virtual bool IsLatCoordVar(string var) const { return (std::find(_latCoordVars.begin(), _latCoordVars.end(), var) != _latCoordVars.end()); }

    //! Return boolean indicating whether the named variable represents
    //! longitude.
    //!
    //! This method returns true if the variable named by \p var is a
    //! longitude coordinate variable. See section 4.2 of the CF spec.
    //!
    //! \retval true if \p var is longitude coordinate variable, false otherwise
    //
    virtual bool IsLonCoordVar(string var) const { return (std::find(_lonCoordVars.begin(), _lonCoordVars.end(), var) != _lonCoordVars.end()); }

    //! Return boolean indicating whether the named variable represents
    //! time.
    //!
    //! This method returns true if the variable named by \p var is a
    //! time coordinate variable. See section 4.4 of the CF spec.
    //!
    //! \retval true if \p var is a time coordinate variable, false otherwise
    //
    virtual bool IsTimeCoordVar(string var) const { return (std::find(_timeCoordVars.begin(), _timeCoordVars.end(), var) != _timeCoordVars.end()); }

    //! Return boolean indicating whether the named variable represents
    //! a vertical coordinate.
    //!
    //! This method returns true if the variable named by \p var is a
    //! dimesional or dimensionless vertical coordinate variable.
    //! See section 4.3 of the CF spec.
    //!
    //! \retval true if \p var is a time coordinate variable, false otherwise
    //
    virtual bool IsVertCoordVar(string var) const { return (std::find(_vertCoordVars.begin(), _vertCoordVars.end(), var) != _vertCoordVars.end()); }

    //! Return boolean indicating whether the named variable represents
    //! a vertical pressure coordinate.
    //!
    //! This method returns true if the variable named by \p var is a
    //! vertical coordinate variable with units of pressure.
    //! See section 4.3 of the CF spec.
    //!
    //! \retval true if \p var is a vertical pressure coordinate variable,
    //! false otherwise
    //
    virtual bool IsVertCoordVarPressure(string var) const;

    //! Return boolean indicating whether the named variable represents
    //! a vertical length coordinate.
    //!
    //! This method returns true if the variable named by \p var is a
    //! vertical coordinate variable with units of length (e.g. meters)
    //! See section 4.3 of the CF spec.
    //!
    //! \retval true if \p var is a vertical length coordinate variable,
    //! false otherwise
    //
    virtual bool IsVertCoordVarLength(string var) const;

    //! Return true if the increasing direction of the named vertical coordinate
    //! variable is up
    //!
    //! CF 1.x description of vertical coordinate direction:
    //!
    //! The direction of positive (i.e., the direction in which the coordinate
    //! values are increasing), whether up or down, cannot in all cases be
    //! inferred from the units. The direction of positive is useful for
    //! applications displaying the data. For this reason the attribute
    //! positive as defined in the COARDS standard is required if the
    //! vertical axis units are not a valid unit of pressure (a determination
    //! which can be made using the udunits routine, utScan) -- otherwise
    //! its inclusion is optional. The positive attribute may have the value
    //! up or down (case insensitive).
    //!
    virtual bool IsVertCoordVarUp(string var) const;

    //! Return a vector of all latitude coordinate variables
    //!
    virtual std::vector<string> GetLatCoordVars() const { return (_latCoordVars); };

    //! Return a vector of all longitude coordinate variables
    //!
    virtual std::vector<string> GetLonCoordVars() const { return (_lonCoordVars); };

    //! Return a vector of all time coordinate variables
    //!
    virtual std::vector<string> GetTimeCoordVars() const { return (_timeCoordVars); };

    //! Return a vector of all vertical coordinate variables
    //!
    virtual std::vector<string> GetVertCoordVars() const { return (_vertCoordVars); };

    //! Return a list of data variables with a given rank
    //!
    //! Returns a list of data variables having a dimension rank
    //! of \p ndim. If \p spatial is true only the data variable's spatial
    //! dimension rank is examined.
    //! Thus if \p spatial is true, and the named variable is explicitly
    //! time varying, the
    //! time-varying dimension is not counted. For example, if a variable
    //! named 'v' is defined with 4 dimensions in the netCDF file, and the
    //! slowest varying dimension name matches a named dimension
    //! specified in Initialize()
    //! by \p time_dimnames, then the variable 'v' would be returned by a
    //! query with ndim==3 and spatial==true.
    //!
    //! Names of variables that are coordinate or auxiliary coordinate
    //! variables are not returned, nor are variables that are missing
    //! coordinate variables.
    //!
    //! \param[in] ndim Rank of spatial dimensions
    //! \param[in] spatial Only compare spatial dimensions against \p ndim
    //!
    //! \sa NetCDFCollection::GetVariableNames()
    //
    virtual std::vector<string> GetDataVariableNames(int ndim, bool spatial) const;

    //!
    //! Return ordered list of coordinate or auxliary coordinate
    //! variables for the named variable.
    //!
    //! This method returns in \p cvars an ordered list of all of the
    //! spatio-temporal coordinate or auxliary coordinate variables
    //! associated with the variable named by \p var. See Chapter 5 of
    //! the CF 1.X spec. for more detail, summarized here:
    //!
    //! CF1.X Chap. 5 excerpt :
    //!
    //! "The use of coordinate variables is required whenever they are
    //! applicable. That is, auxiliary coordinate variables may not be
    //! used as the only way to identify latitude and longitude coordinates
    //! that could be identified using coordinate variables. This is both
    //! to enhance conformance to COARDS and to facilitate the use of
    //! generic applications that recognize the NUG convention for coordinate
    //! variables. An application that is trying to find the latitude
    //! coordinate of a variable should always look first to see if any
    //! of the variable's dimensions correspond to a latitude coordinate
    //! variable. If the latitude coordinate is not found this way, then
    //! the auxiliary coordinate variables listed by the coordinates
    //! attribute should be checked. Note that it is permissible, but
    //! optional, to list coordinate variables as well as auxiliary
    //! coordinate variables in the coordinates attribute."
    //!
    //! \param[in] varname A variable name
    //! \param[out] cvars A ordered vector of coordinate variable names
    //!
    //! \retval status a negative int is returned if the number of
    //! elements in \p cvars does not match the number of spatio-temporal
    //! dimensions of the variable named by \p var
    //
    virtual int GetVarCoordVarNames(string var, std::vector<string> &cvars) const;

    //! Return the value of the 'units' attribute for the named variable
    //!
    //! This method fetches the value of the 'units' attribute, if present,
    //! for the variable named by \p var. If a units attribute is not
    //! present then \p units will contain an empty string.
    //!
    //! \param[in] varname A variable name
    //! \param[out] units The value of the variable's 'units' attribute. If the
    //! 'units' attribute is not present the parameter will contain the
    //! empty string.
    //!
    //! \retval status a non-negative int is returned on success
    //!
    virtual int GetVarUnits(string var, string &units) const;

    //! Return a pointer to the internal UDUnits object used to perform
    //! unit conversion.
    //!
    const UDUnits *GetUDUnits() const { return (_udunit); };

    //! Convert an array of floating point values from one unit to another
    //!
    //! This method uses the UDUnits class to convert an array of floating
    //! point values from one unit measure to another as supported by
    //! Unidata's udunits2 library.
    //! See <A HREF="http://www.unidata.ucar.edu/software/udunits/udunits-2/udunits2.html">
    //!
    //! \param[in] from A string containing the unit to convert
    //! from (e.g. "meters")
    //! \param[in] to A string containing the unit to convert
    //! to (e.g. "feet")
    //! \param[in] src An array of input values to be converted
    //! \param[out] dst An output buffer large enough to contained
    //! the converted data
    //! \param[in] n The number of elements in \p src
    //!
    //! \retval status a non-negative int is returned on success
    //!
    virtual int Convert(const string from, const string to, const double *src, double *dst, size_t n) const;
    virtual int Convert(const string from, const string to, const float *src, float *dst, size_t n) const;

    //! Return the missing value, if any, for a variable
    //!
    //! This method returns the value of the missing data value marker,
    //! if defined, for the variable named by \p varname.
    //!
    //! Missing data values are indicated using the \c _FillValue, or
    //! \c missing_value attributes as defined in section 2.5.1 of the CF 1.6
    //! spec.
    //!
    //! \param[in] varname The variable name
    //! \param[out] mv The missing value for the variabled name by \p varname
    //!
    //! \retval bool The boolean true is returned if a missing value is defined.
    //! If no missing variable is defined the return value is false and the
    //! value of \p mv is not defined.
    //!
    virtual bool GetMissingValue(string varname, double &mv) const;

    //! \copydoc NetCDFCollection::OpenRead()
    //!
    virtual int OpenRead(size_t ts, string varname);

    //! Return true if the named variable is a dimensionless vertical
    //! coordinate variable.
    //!
    //! This method returns true if the variable named by \p cvar is both
    //! a vertical coordinate variable and it is dimensionless. See
    //! section 4.3.2 of the CF 1.6 spec.
    //!
    virtual bool IsVertDimensionless(string cvar) const;

    //! Get a map projection as a proj4 string
    //!
    //! If a variable has a map projection generate a proj4
    //! transformation string for converting from geographic to
    //! Cartographic coordinates. Returns false if no proj4 string
    //! could be generated: either no map projection exists, or one
    //! exists but is not supported.
    //
    bool GetMapProjectionProj4(string varname, string &proj4string) const;

    void FormatTimeStr(double time, string &str) const;

    friend std::ostream &operator<<(std::ostream &o, const NetCDFCFCollection &ncdfc);

private:
    std::vector<std::string>       _coordinateVars;
    std::vector<std::string>       _auxCoordinateVars;
    std::vector<std::string>       _lonCoordVars;
    std::vector<std::string>       _latCoordVars;
    std::vector<std::string>       _vertCoordVars;
    std::vector<std::string>       _timeCoordVars;

    UDUnits *_udunit;

    //
    // Map a variable name to it's missing value (if any)
    //
    std::map<string, double> _missingValueMap;

    std::vector<std::string> _GetCoordAttrs(const NetCDFSimple::Variable &varinfo) const;

    int _Initialize(const std::vector<string> &files);

    //! CF1.X Definition of <em> coordinate variable </em>:
    //!
    //! "We use this term precisely as it is defined in section 2.3.1 of the NUG.
    //! It is a one- dimensional variable with the same name as its
    //! dimension [e.g., time(time)], and it is defined as a numeric data
    //! type with values that are ordered monotonically. Missing values are
    //! not allowed in coordinate variables."
    //
    bool _IsCoordinateVar(const NetCDFSimple::Variable &varinfo) const;

    //!
    //! CF1.X Determination of a longitude coordinate variable:
    //!
    //! "We recommend the determination that a coordinate is a longitude
    //! type should be done via a string match between the given unit and
    //! one of the acceptable forms of degrees_east.
    //!
    //! Optionally, the longitude type may be indicated additionally by
    //! providing the standard_name attribute with the value longitude,
    //! and/or the axis attribute with the value X.
    //!
    //! Coordinates of longitude with respect to a rotated pole should be
    //! given units of degrees, not degrees_east or equivalents, because
    //! applications which use the units to identify axes would have no
    //! means of distinguishing such an axis from real longitude, and might
    //! draw incorrect coastlines, for instance."

    bool _IsLonCoordVar(const NetCDFSimple::Variable &varinfo) const;

    //! CF1.X Determination of a latitude coordinate variable:
    //!
    //! Hence, determination that a coordinate is a latitude type should
    //! be done via a string match between the given unit and one of the
    //! acceptable forms of degrees_north.
    //!
    //! Optionally, the latitude type may be indicated additionally by
    //! providing the standard_name attribute with the value latitude,
    //! and/or the axis attribute with the value Y
    //!
    bool _IsLatCoordVar(const NetCDFSimple::Variable &varinfo) const;

    //! CF1.X Determination of vertical coordinate variable
    //!
    //! A vertical coordinate will be identifiable by:
    //!
    //! units of pressure; or
    //!
    //! the presence of the positive attribute with a value of up or down
    //! (case insensitive).
    //!
    //! Optionally, the vertical type may be indicated additionally by
    //! providing the standard_name attribute with an appropriate value,
    //! and/or the axis attribute with the value Z.
    //!
    bool _IsVertCoordVar(const NetCDFSimple::Variable &varinfo) const;

    //! CF1.X Determination of time coordinate variable
    //!
    //! A time coordinate is identifiable from its units string alone. The
    //! Udunits routines utScan() and utIsTime() can be used to make this
    //! determination.
    //!
    //! Optionally, the time coordinate may be indicated additionally by
    //! providing the standard_name attribute with an appropriate value,
    //! and/or the axis attribute with the value T.
    //!
    bool _IsTimeCoordVar(const NetCDFSimple::Variable &varinfo) const;

    bool _GetMissingValue(string varname, string attname, double &mv) const;
    void _GetMissingValueMap(map<string, double> &missingValueMap, string &attname) const;

};
};    // namespace VAPoR

#endif
