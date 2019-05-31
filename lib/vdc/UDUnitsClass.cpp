#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include "vapor/VAssert.h"
#ifdef WIN32
    #include "vapor/udunits2.h"
#else
    #include <udunits2.h>
#endif
#include <vapor/ResourcePath.h>
#include <vapor/UDUnitsClass.h>

using namespace VAPoR;
using namespace Wasp;
using namespace std;

UDUnits::UDUnits()
{
    _statmsg[UT_SUCCESS] = "Success";
    _statmsg[UT_BAD_ARG] = "An argument violates the function's contract";
    _statmsg[UT_EXISTS] = "Unit, prefix, or identifier already exists";
    _statmsg[UT_NO_UNIT] = "No such unit exists";
    _statmsg[UT_OS] = "Operating-system error.";
    _statmsg[UT_NOT_SAME_SYSTEM] = "The units belong to different unit-systems";
    _statmsg[UT_MEANINGLESS] = "The operation on the unit(s) is meaningless";
    _statmsg[UT_NO_SECOND] = "The unit-system doesn't have a unit named \"second\"";
    _statmsg[UT_VISIT_ERROR] = "An error occurred while visiting a unit";
    _statmsg[UT_CANT_FORMAT] = "A unit can't be formatted in the desired manner";
    _statmsg[UT_SYNTAX] = "string unit representation contains syntax error";
    _statmsg[UT_UNKNOWN] = "string unit representation contains unknown word";
    _statmsg[UT_OPEN_ARG] = "Can't open argument-specified unit database";
    _statmsg[UT_OPEN_ENV] = "Can't open environment-specified unit database";
    _statmsg[UT_OPEN_DEFAULT] = "Can't open installed, default, unit database";
    _statmsg[UT_PARSE] = "Error parsing unit specification";

    _pressureUnit = NULL;
    _timeUnit = NULL;
    _latUnit = NULL;
    _lonUnit = NULL;
    _lengthUnit = NULL;
    _status = (int)UT_SUCCESS;
    _unitSystem = NULL;
}

std::string UDUnits::GetDatabasePath() { return GetSharePath("udunits/udunits2.xml"); }

int UDUnits::Initialize()
{
    //
    // Need to turn off error messages, which go to stderr by default
    //
    ut_set_error_message_handler(ut_ignore);
    string unitstr;

    string path = GetDatabasePath();
    if (!path.empty()) {
        _unitSystem = ut_read_xml(path.c_str());
    } else {
        MyBase::SetErrMsg("Could not find VAPOR installed UDUnits database");
        MyBase::SetErrMsg("Attempting to fall back to system install");
        _unitSystem = ut_read_xml(NULL);
    }

    if (!_unitSystem) goto UDUnits_Initialize_Error;

    //
    // We need to be able to determine if a given unit is of a
    // particular type (e.g. time, pressure, mass, etc). The udunit2
    // API doesn't support this directly. So we create a 'unit'
    // of a particular known type, and then later we can query udunit2
    // to see if it is possible to convert between the known unit type
    // and a unit of unknown type
    //

    unitstr = "Pa";    // Pascal units of Pressure
    _pressureUnit = ut_parse(_unitSystem, unitstr.c_str(), UT_ASCII);
    if (!_pressureUnit) goto UDUnits_Initialize_Unit_Error;

    unitstr = "seconds";
    _timeUnit = ut_parse(_unitSystem, unitstr.c_str(), UT_ASCII);
    if (!_timeUnit) goto UDUnits_Initialize_Unit_Error;

    unitstr = "degrees_north";
    _latUnit = ut_parse(_unitSystem, unitstr.c_str(), UT_ASCII);
    if (!_latUnit) goto UDUnits_Initialize_Unit_Error;

    unitstr = "degrees_east";
    _lonUnit = ut_parse(_unitSystem, unitstr.c_str(), UT_ASCII);
    if (!_lonUnit) goto UDUnits_Initialize_Unit_Error;

    unitstr = "meter";
    _lengthUnit = ut_parse(_unitSystem, unitstr.c_str(), UT_ASCII);
    if (!_lengthUnit) goto UDUnits_Initialize_Unit_Error;

    return 0;

UDUnits_Initialize_Unit_Error:
    MyBase::SetErrMsg("UDUnits failed to parse unit \"%s\"", unitstr.c_str());
UDUnits_Initialize_Error:
    _status = (int)ut_get_status();
    MyBase::SetErrMsg("UDUnits failed to initialize");
    MyBase::SetErrMsg("UDUnits Error: %s", GetErrMsg().c_str());
    return -1;
}

bool UDUnits::AreUnitsConvertible(const ut_unit *unit, string unitstr) const
{
    ut_unit *myunit = ut_parse(_unitSystem, unitstr.c_str(), UT_ASCII);
    if (!myunit) {
        ut_set_status(UT_SUCCESS);    // clear error message
        return (false);
    }

    bool          status = true;
    cv_converter *cv = NULL;
    if (!(cv = ut_get_converter((ut_unit *)unit, myunit))) {
        status = false;
        ut_set_status(UT_SUCCESS);
    }

    if (myunit) ut_free(myunit);
    if (cv) cv_free(cv);
    return (status);
}

bool UDUnits::ValidUnit(string unitstr) const
{
    ut_unit *myunit = ut_parse(_unitSystem, unitstr.c_str(), UT_ASCII);
    if (!myunit) {
        ut_set_status(UT_SUCCESS);    // clear error message
        return (false);
    }
    return (true);
}

bool UDUnits::IsPressureUnit(string unitstr) const { return (AreUnitsConvertible(_pressureUnit, unitstr)); }

bool UDUnits::IsTimeUnit(string unitstr) const { return (AreUnitsConvertible(_timeUnit, unitstr)); }

bool UDUnits::IsLatUnit(string unitstr) const
{
    bool status = AreUnitsConvertible(_latUnit, unitstr);
    if (!status) return (false);

    // udunits2 does not distinguish between longitude and latitude, only
    // whether a unit is a plane angle measure. N.B. the conditional
    // below is probably all that is needed for this method.
    //
    if (!((unitstr.compare("degrees_north") == 0) || (unitstr.compare("degree_north") == 0) || (unitstr.compare("degree_N") == 0) || (unitstr.compare("degrees_N") == 0)
          || (unitstr.compare("degreeN") == 0) || (unitstr.compare("degreesN") == 0))) {
        status = false;
    }
    return (status);
}

bool UDUnits::IsLonUnit(string unitstr) const
{
    bool status = AreUnitsConvertible(_lonUnit, unitstr);
    if (!status) return (false);

    // udunits2 does not distinguish between longitude and latitude, only
    // whether a unit is a plane angle measure. N.B. the conditional
    // below is probably all that is needed for this method.
    //
    if (!((unitstr.compare("degrees_east") == 0) || (unitstr.compare("degree_east") == 0) || (unitstr.compare("degree_E") == 0) || (unitstr.compare("degrees_E") == 0)
          || (unitstr.compare("degreeE") == 0) || (unitstr.compare("degreesE") == 0))) {
        status = false;
    }
    return (status);
}
bool UDUnits::IsLatOrLonUnit(string unitstr) const { return (AreUnitsConvertible(_lonUnit, unitstr) || AreUnitsConvertible(_latUnit, unitstr)); }

bool UDUnits::IsLengthUnit(string unitstr) const { return (AreUnitsConvertible(_lengthUnit, unitstr)); }

bool UDUnits::Convert(const string from, const string to, const float *src, float *dst, size_t n) const
{
    ut_unit *fromunit = ut_parse(_unitSystem, from.c_str(), UT_ASCII);
    if (!fromunit) {
        ut_set_status(UT_SUCCESS);    // clear error message
        return (false);
    }

    ut_unit *tounit = ut_parse(_unitSystem, to.c_str(), UT_ASCII);
    if (!tounit) {
        ut_free(fromunit);
        ut_set_status(UT_SUCCESS);    // clear error message
        return (false);
    }

    cv_converter *cv = NULL;
    cv = ut_get_converter((ut_unit *)fromunit, tounit);
    if (!cv) {
        ut_free(tounit);
        ut_free(fromunit);
        ut_set_status(UT_SUCCESS);
        return (false);
    }

    cv_convert_floats(cv, src, n, dst);

    if (fromunit) ut_free(fromunit);
    if (tounit) ut_free(tounit);
    if (cv) cv_free(cv);
    return (true);
}

bool UDUnits::Convert(const string from, const string to, const double *src, double *dst, size_t n) const
{
    ut_unit *fromunit = ut_parse(_unitSystem, from.c_str(), UT_ASCII);
    if (!fromunit) {
        ut_set_status(UT_SUCCESS);    // clear error message
        return (false);
    }

    ut_unit *tounit = ut_parse(_unitSystem, to.c_str(), UT_ASCII);
    if (!tounit) {
        ut_free(fromunit);
        ut_set_status(UT_SUCCESS);    // clear error message
        return (false);
    }

    cv_converter *cv = NULL;
    cv = ut_get_converter((ut_unit *)fromunit, tounit);
    if (!cv) {
        ut_free(tounit);
        ut_free(fromunit);
        ut_set_status(UT_SUCCESS);
        return (false);
    }

    cv_convert_doubles(cv, src, n, dst);

    if (fromunit) ut_free(fromunit);
    if (tounit) ut_free(tounit);
    if (cv) cv_free(cv);
    return (true);
}

void UDUnits::DecodeTime(double seconds, int *year, int *month, int *day, int *hour, int *minute, int *second) const
{
    double dummy;
    double second_d;
    ut_decode_time(seconds, year, month, day, hour, minute, &second_d, &dummy);
    *second = (int)second_d;
}

double UDUnits::EncodeTime(int year, int month, int day, int hour, int minute, int second) const { return (ut_encode_time(year, month, day, hour, minute, (double)second)); }

string UDUnits::GetErrMsg() const
{
    map<int, string>::const_iterator itr;
    itr = _statmsg.find(_status);
    if (itr == _statmsg.end()) return (string("UNKNOWN"));

    return (itr->second);
}

UDUnits::~UDUnits()
{
    if (_pressureUnit) ut_free(_pressureUnit);
    if (_timeUnit) ut_free(_timeUnit);
    if (_latUnit) ut_free(_latUnit);
    if (_lonUnit) ut_free(_lonUnit);
    if (_lengthUnit) ut_free(_lengthUnit);
    if (_unitSystem) ut_free_system(_unitSystem);
}
