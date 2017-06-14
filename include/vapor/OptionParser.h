//
//      $Id$
//
//************************************************************************
//								*
//		     Copyright (C)  2004			*
//     University Corporation for Atmospheric Research		*
//		     All Rights Reserved			*
//								*
//************************************************************************/
//
//	File:
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Wed Oct  6 10:49:44 MDT 2004
//
//	Description:	A class for parsing command line options.  This
//					class  manages a resource data base of valid command line
//					options. Valid options may be merged into the data base
//					at any time and later extracted with their
//					coresponding values as determined by the command line.
//
//

#ifndef _OptionParser_h_
#define _OptionParser_h_

#include <string>
#include <cmath>
#include <vector>
#include <vapor/MyBase.h>
#include <vapor/common.h>

#ifdef WIN32
    // Silence an annoying and unnecessary compiler warning
    #pragma warning(disable : 4251)
#endif
using namespace std;

namespace Wasp {

//
//
class COMMON_API OptionParser : public MyBase {
public:
    //! An option description record (odr)
    //
    //! A structure for descriping an option
    //! \param option The name of the option
    //! \param arg_count Number of arguments expected by the option
    //! \param value Option's default value
    //! \param help A C string containing a help message for the option
    //
    typedef struct _OptDescRec {
        const char *option;
        int         arg_count;
        const char *value;
        const char *help;
    } OptDescRec_T;

    //
    //  structure for returning the value of an option
    //
    typedef struct _DPOption {
        const char *option_name;    // the options name

        //
        // option type converter
        //
        int (*type_conv)(const char *from, void *to);

        void *offset;    // offset of return address
        int   size;      // size of option in bytes
    } Option_T;

    typedef struct _EnvOpt {
        const char *option;     // option name
        const char *env_var;    // coresponding enviroment var
    } EnvOpt_T;

    typedef int Boolean_T;
    typedef struct Dimension2D_ {
        int nx, ny;
    } Dimension2D_T;

    typedef struct Dimension3D_ {
        int nx, ny, nz;
    } Dimension3D_T;

    // Converter type for CvtToIntRange()
    //
    typedef struct IntRange_ {
        int min, max;
    } IntRange_T;

    OptionParser();
    ~OptionParser();

    //! Append a list of option descriptions
    //
    //! Append a list of option descriptions. The input option
    //! descriptor records are appended to the current list of option
    //! description records.
    //! \param[in] odr A null-terminated option descriptor record.
    //! \sa ParseOptions(), RemoveOptions()
    //
    int AppendOptions(const OptDescRec_T *odr);

    //! Parse a command line argument vector
    //
    //! Destrutively parse a command line argument vector against
    //! the option descriptor records (odr) supplied by previous invocations
    //! of \b AppendOptions(). Command line arguments that match option
    //! names in the odr and the input option table are
    //! \param[in,out] argc A pointer to a count of the number of elements
    //! in \b argv
    //! \param[in,out] argv A null-terminated vector of command line arguments
    //! \param[in,out] opts A null-terminated option table
    //! \sa ParseOptions(), RemoveOptions()
    int  ParseOptions(int *argc, char **argv, Option_T *opts);
    int  ParseOptions(const EnvOpt_T *envv, Option_T *opts);
    void RemoveOptions(std::vector<string> options);
    void PrintOptionHelp(FILE *fp, int linelimit = 80, bool docopyright = true);

    typedef struct _OptRec {
        const char *option;           // name of option without preceeding '-'
        const char *value;            // current val for the argument
        const char *default_value;    // default val for the argument
        const char *help;             // help string for option
        int         arg_count;        // num args expected by option
    } _OptRec_T;

private:
    vector<struct _OptRec *> _optTbl;

    _OptRec_T *_get_option(const char *name);
    int        _parse_options(const Option_T *opts);

    friend bool opt_cmp(OptionParser::_OptRec_T *a, OptionParser::_OptRec_T *b);
};

COMMON_API int CvtToInt(const char *from, void *to);

COMMON_API int CvtToFloat(const char *from, void *to);

COMMON_API int CvtToDouble(const char *from, void *to);

COMMON_API int CvtToChar(const char *from, void *to);

COMMON_API int CvtToBoolean(const char *from, void *to);

COMMON_API int CvtToString(const char *from, void *to);

COMMON_API int CvtToCPPStr(const char *from, void *to);

COMMON_API int CvtToDimension2D(const char *from, void *to);

COMMON_API int CvtToDimension3D(const char *from, void *to);

// convert a colon delimited ascii string to vector of C++
// STL strings: (vector <string> *)
//
COMMON_API int CvtToStrVec(const char *from, void *to);

// convert a colon delimited ascii string to vector of C++
// STL ints: (vector <int> *)
//
COMMON_API int CvtToIntVec(const char *from, void *to);

// convert a colon delimited ascii string to vector of C++
// STL size_t: (vector <size_t> *)
//
COMMON_API int CvtToSize_tVec(const char *from, void *to);

// convert a colon delimited ascii string to vector of C++
// STL ints: (vector <float> *)
//
COMMON_API int CvtToFloatVec(const char *from, void *to);

COMMON_API int CvtToDoubleVec(const char *from, void *to);

// Convert a colon-delimited pair of integers to a IntRange_T type
//
COMMON_API int CvtToIntRange(const char *from, void *to);

};    // namespace Wasp

#endif
