//
// $Id$
//


#ifndef	_UDUnitsClass_h_
#define	_UDUnitsClass_h_

#include <string>
#include <vector>
#include <map>
#include <algorithm>

union ut_unit;
struct ut_system;

namespace VAPoR {


//! \class UDUnits
//!
//! Provides a C++ wrapper for Unidata's Udunits-2 package:
//! http://www.unidata.ucar.edu/software/udunits/udunits-2/udunits2.html
//!
//! Error handling: most methods in this class return an integer status 
//! flag, with a negative value indicating error. Error messages for the 
//! most recent error can be retrieved with GetErrMsg().
//!
class VDF_API UDUnits {
public:
 UDUnits();
 ~UDUnits();

 //! Initialize the Udunits-2 data base. Must be called after the
 //! constructor, prior to any other class methods
 //!
 //! \retval status A negative int is returned on failure
 //!
 int Initialize();
  
 //! Return true if the string provided by \p unitstr is recognized 
 //! by Udunits-2 as a unit of pressure
 //!
 bool IsPressureUnit(std::string unitstr) const;

 //! Return true if the string provided by \p unitstr is recognized 
 //! by Udunits-2 as a unit of time
 //!
 bool IsTimeUnit(std::string unitstr) const;

 //! Return true if the string provided by \p unitstr is recognized 
 //! by Udunits-2 as a unit of Latitude
 //!
 bool IsLatUnit(std::string unitstr) const;

 //! Return true if the string provided by \p unitstr is recognized 
 //! by Udunits-2 as a unit of Longitude
 //!
 bool IsLonUnit(std::string unitstr) const;

 //! Return true if the string provided by \p unitstr is recognized 
 //! by Udunits-2 as a unit of length
 //!
 bool IsLengthUnit(std::string unitstr) const;

 //! Return true if the unit identified by the string contained in \p unitstr 
 //! can be converted into the unit identified by \p unit. Otherwise false
 //! is returned
 //!
 bool AreUnitsConvertible(const ut_unit *unit, std::string unitstr) const;

 //! Returns true if the string contained in \p unitstr is recognized 
 //! as a valid unit by the Udunits-2 data base
 //!
 bool ValidUnit(std::string unitstr) const;

 //! Convert one unit to another
 //!
 //! This method performs conversion between different units, when
 //! possible (see AreUnitsConvertible()).  For example, converting
 //! meters to feet. The string \p from specifies the unit to convert
 //! from, and the string \p to specifies the unit to convert to.
 //!
 //! \param[in] from A string specifying the source unit
 //! \param[in] to A string specifying the destination unit
 //! \param[in] src An array of source unit values
 //! \param[out] dst An array large enough to store the converted 
 //! unit values.
 //! \param[in] n The number of elements in \p from
 //!
 //! \retval boolean A boolean status is returned indicating whether
 //! the conversion took place. Conversion will fail if the units are
 //! not convertible. No error message is generated.
 //!
 //! \sa AreUnitsConvertible
 //!
 bool Convert(
   const std::string from,
   const std::string to,
   const float *src,
   float *dst,
   size_t n
 ) const;

bool Convert(
	const std::string from,
	const std::string to,
	const double *src,
	double *dst,
	size_t n
) const;

 //! Decode time specified in seconds to year, month, day, hour, minute
 //! and second.
 //!
 //! This method uses Udunits-2 ut_decode_time() function to perform
 //! time conversion
 //!
 void DecodeTime(
	double seconds, int* year, int* month, int* day,
	int* hour, int* minute, int* second
 ) const;

 //! Encode time specified to years, month, day, hour, minute
 //! and seconds to seconds
 //!
 //! This method uses Udunits-2 ut_encode_time() function to perform
 //! time conversion
 //!
 double EncodeTime(
	int year, int month, int day, int hour, int minute, int second
 ) const;

 //! Get the most recent error message
 //!
 //! This method returns a string containing the most recently occurring
 //! error message, if any. 
 //!
 std::string GetErrMsg() const;

private:
 std::map <int, std::string> _statmsg;
 int _status;
 ut_unit *_pressureUnit;
 ut_unit *_timeUnit;
 ut_unit *_latUnit;
 ut_unit *_lonUnit;
 ut_unit *_lengthUnit;
 ut_system *_unitSystem;

};
};
#endif
