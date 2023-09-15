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
//	Date:		Wed Sep 29 15:40:23 MDT 2004
//
//	Description:	A collection of general purpose utilities - things that
//					probably should be in the STL but aren't.
//

#ifndef _MyBase_h_
#define _MyBase_h_

#include <cmath>
#include <cstdarg>
#include <string>
#include <cstring>
#include <vector>
#include <vapor/common.h>

#ifdef NEW_DEBUG
    #include <new>
void *operator new(size_t sz);
void *operator new[](size_t sz);
#endif

#ifdef WIN32
    // Silence an annoying and unnecessary compiler warning
    #pragma warning(disable : 4251)
#endif
using namespace std;

namespace Wasp {

//! \class MyBase
//! \brief Wasp base class
//! \author John Clyne
//! \version 0.1
//! \date    Mon Dec 13 17:15:12 MST 2004

//! A collection of general purpose utilities - things that
//!                  probably should be in the STL but aren't.
//!

// default values used for variables outside valid grid
const float ABOVE_GRID = 0.f;
const float BELOW_GRID = 0.f;

//
// The MyBase base class provides a simple error reporting mechanism
// that can be used by derrived classes. N.B. the error messages/codes
// are stored in static class members.
//
class COMMON_API MyBase {
public:
    typedef void (*ErrMsgCB_T)(const char *msg, int err_code);
    typedef void (*DiagMsgCB_T)(const char *msg);

    MyBase();
    const string &getClassName() const { return (_className); };

    //! Record a formatted error message.
    //
    //! Formats and records an error message. Subsequent calls will overwrite
    //! the stored error message. The method will also set the error
    //! code to 1.
    //! \param[in] format A 'C' style sprintf format string.
    //! \param[in] args... Arguments to format
    //! \sa GetErrMsg(), GetErrCode()
    //
    static void SetErrMsg(const char *format, ...);

    //! Record a formatted error message and an error code.
    //
    //! Formats and records an error message. Subsequent calls will overwrite
    //! the stored error message. The method will also set the error
    //! code to \b err_code.
    //! \param[in] errcode A application-defined error code
    //! \param[in] format A 'C' style sprintf format string.
    //! \param[in] arg... Arguments to format
    //! \sa GetErrMsg(), GetErrCode()
    //
    static void SetErrMsg(int errcode, const char *format, ...);

    //! Retrieve the current error message
    //!
    //! Retrieves the last error message set with SetErrMsg().
    //! It is the
    //! caller's responsibility to copy the message returned to user space.
    //! \sa SetErrMsg(), SetErrCode()
    //! \retval msg A pointer to null-terminated string.
    //
    static const char *GetErrMsg() { return (ErrMsg); }

    //! Record an error code
    //
    //! Sets the error code to the indicated value.
    //! \param[in] err_code The error code
    //! \sa GetErrMsg(), GetErrCode(), SetErrMsg()
    //
    static void SetErrCode(int err_code) { ErrCode = err_code; }

    //! Retrieve the current error code
    //
    //! Retrieves the last error code set either explicity with SetErrCode()
    //! or indirectly with a call to SetErrMsg().
    //! \sa SetErrMsg(), SetErrCode()
    //! \retval code An erroor code
    //
    static int GetErrCode() { return (ErrCode); }

    //! Set a callback function for error messages
    //!
    //! Set the callback function to be called whenever SetErrMsg()
    //! is called. The callback function, \p cb, will be called and passed
    //! the formatted error message and the error code as an argument. The
    //! default callback function is NULL, i.e. no function is called
    //!
    //! \param[in] cb A callback function or NULL
    //
    static void SetErrMsgCB(ErrMsgCB_T cb) { ErrMsgCB = cb; };

    //! Get the callback function for error messages
    //!
    //! Get the callback function to be called whenever SetErrMsg()
    //! is called. This method returns the address of the callback function
    //! set with the most recent call to SetErrMsgCB(). If no callback function
    //! is defined, NULL is returned.
    //!
    //! \sa SetErrMsgCB
    //
    static ErrMsgCB_T GetErrMsgCB() { return (ErrMsgCB); };

    //! Set the file pointer to whence error messages are written
    //!
    //! This method permits the specification of a file pointer to which
    //! all messages logged with SetErrMsg() will be written. The default
    //! file pointer is NULL. I.e. by default error messages logged by
    //! SetErrMsg() are not written.
    //! \param[in] fp A file pointer opened for writing or NULL
    //! \sa SetErrMsg()
    //
    static void SetErrMsgFilePtr(FILE *fp) { ErrMsgFilePtr = fp; };

    //! Get the file pointer to whence error messages are written
    //!
    //! This method returns the error message file pointer most recently
    //! set with SetErrMsgFilePtr().
    //!
    //! \sa SetErrMsgFilePtr()
    //
    static const FILE *SetErrMsgFilePtr() { return (ErrMsgFilePtr); };

    //! Record a formatted diagnostic message.
    //
    //! Formats and records a diagnostic message. Subsequent calls will overwrite
    //! the stored error message. This method differs from SetErrMsg() only
    //! in that no associated error code is set - the message is considered
    //! diagnostic only, not an error.
    //! \param[in] format A 'C' style sprintf format string.
    //! \param[in] arg... Arguments to format
    //! \sa GetDiagMsg()
    //
    static void SetDiagMsg(const char *format, ...);

    //! Retrieve the current diagnostic message
    //!
    //! Retrieves the last error message set with \b SetDiagMsg(). It is the
    //! caller's responsibility to copy the message returned to user space.
    //! \sa SetDiagMsg()
    //! \retval msg A pointer to null-terminated string.
    //
    static const char *GetDiagMsg() { return (DiagMsg); }

    //! Set a callback function for diagnostic messages
    //!
    //! Set the callback function to be called whenever SetDiagMsg()
    //! is called. The callback function, \p cb, will be called and passed
    //! the formatted error message as an argument. The
    //! default callback function is NULL, i.e. no function is called
    //!
    //! \param[in] cb A callback function or NULL
    //
    static void SetDiagMsgCB(DiagMsgCB_T cb) { DiagMsgCB = cb; };

    //! Get the callback function for error messages
    //!
    //! Get the callback function to be called whenever SetDiagMsg()
    //! is called. This method returns the address of the callback function
    //! set with the most recent call to SetDiagMsgCB(). If no callback function
    //! is defined, NULL is returned.
    //!
    //! \sa SetDiagMsgCB
    //
    static DiagMsgCB_T GetDiagMsgCB() { return (DiagMsgCB); };

    //! Set the file pointer to whence diagnostic messages are written
    //!
    //! This method permits the specification of a file pointer to which
    //! all messages logged with SetDiagMsg() will be written. The default
    //! file pointer is NULL. I.e. by default error messages logged by
    //! SetDiagMsg() are not written.
    //! \param[in] fp A file pointer opened for writing or NULL
    //! \sa SetDiagMsg()
    //
    static void SetDiagMsgFilePtr(FILE *fp) { DiagMsgFilePtr = fp; };

    //! Get the file pointer to whence diagnostic messages are written
    //!
    //! This method returns the error message file pointer most recently
    //! set with SetDiagMsgFilePtr().
    //!
    //! \sa SetDiagMsgFilePtr()
    //

    //!
    //! Enable or disable error message reporting.
    //!
    //! When disabled calls to SetErrMsg() report no error messages
    //! either through the error message callback or the error message
    //! FILE pointer.
    //!
    //! \param[in] enable Boolean flag to enable or disable error reporting
    //!
    static bool EnableErrMsg(bool enable)
    {
        bool prev = Enabled;
        Enabled = enable;
        return (prev);
    };

    static bool GetEnableErrMsg() { return Enabled; }

    // N.B. the error codes/messages are stored in static class members!!!
    static char *     ErrMsg;
    static int        ErrCode;
    static int        ErrMsgSize;
    static FILE *     ErrMsgFilePtr;
    static ErrMsgCB_T ErrMsgCB;

    static char *      DiagMsg;
    static int         DiagMsgSize;
    static FILE *      DiagMsgFilePtr;
    static DiagMsgCB_T DiagMsgCB;
    static bool        Enabled;

protected:
    void SetClassName(const string &name) { _className = name; };

private:
    static void _SetErrMsg(char **msg, int *sz, const char *format, va_list args);
    string      _className;    // name of class
};

COMMON_API inline int IsOdd(int x) { return (x % 2); };

//! Return true if power of two
//
//! Returns a non-zero value if the input parameter is a power of two
//! \param[in] x An integer
//! \retval status
COMMON_API int IsPowerOfTwo(unsigned int x);

COMMON_API inline int Min(int a, int b) { return (a < b ? a : b); };
COMMON_API inline int Max(int a, int b) { return (a > b ? a : b); };

COMMON_API inline size_t Min(size_t a, size_t b) { return (a < b ? a : b); };
COMMON_API inline size_t Max(size_t a, size_t b) { return (a > b ? a : b); };

COMMON_API inline float Min(float a, float b) { return (a < b ? a : b); };
COMMON_API inline float Max(float a, float b) { return (a > b ? a : b); };

COMMON_API inline double Min(double a, double b) { return (a < b ? a : b); };
COMMON_API inline double Max(double a, double b) { return (a > b ? a : b); };

COMMON_API inline double LogBaseN(double x, double n) { return (log(x) / log(n)); };

// Find integer log, base 2:
COMMON_API int ILog2(int n);

//! Case-insensitive string comparison
//
//! Performs a case-insensitive comparison of two C++ strings. Behaviour
//! is otherwise identical to the C++ std::string.compare() method.
//
COMMON_API int StrCmpNoCase(const string &s, const string &t);

//! Remove white space from a string
//
//! Performs in-place removal of all white space from a string.
//! \param[in,out] s The string.
COMMON_API void StrRmWhiteSpace(string &s);

//! Parse a string, returning a vector of words
//
//! Parses a string containing a white-space delimited collection
//! of words. The words are in order of occurence and returned
//! as a vector of strings with all white space removed.
//!
//! \param[in] s The input string.
//! \param[out] v The output vector.
//
COMMON_API void StrToWordVec(const string &s, vector<string> &v);

COMMON_API std::vector<std::string> &SplitString(const std::string &s, char delim, std::vector<std::string> &elems);
COMMON_API std::vector<size_t> &SplitString(const std::string &s, char delim, std::vector<size_t> &elems);
COMMON_API std::vector<int> &SplitString(const std::string &s, char delim, std::vector<int> &elems);
COMMON_API std::vector<float> &SplitString(const std::string &s, char delim, std::vector<float> &elems);
COMMON_API std::vector<double> &SplitString(const std::string &s, char delim, std::vector<double> &elems);

//! Retrieve a sequence of bits
//!
//! Extract \p n bits from \p targ starting at position
//! \p pos counting from the left. E.g. GetBits64(I,4,3) will
//! extract bits at bit position 4,3,2, right adjusted
//!
//! \retval returns the extracted bits
//! \sa GetBits64()
//
COMMON_API unsigned long long GetBits64(unsigned long long targ, int pos, int n);

//! Set a sequence of bits
//!
//! Set \p n bits in \p targ starting at position
//! \p pos counting from the left. The bits are obtained from
//! \p src
//!
//! \retval returns \targ with the indicated bits set
//! \sa GetBits64()
//
COMMON_API unsigned long long SetBits64(unsigned long long targ, int pos, int n, unsigned long long src);

// ran1 function declaration (from numerical recipes in C)
COMMON_API double ran1(long *);
};    // namespace Wasp

//
// Handle OS differences in 64-bit IO operators
//

// 64-bit fseek
//

#ifdef FSEEK64
    #undef FSEEK64
#endif

#if defined(WIN32)
      // Note: win32 won't seek beyond 32 bits
    #define FSEEK64 fseek
#endif

#if defined(Linux) || defined(AIX)
    #define FSEEK64 fseeko64
#endif

#if defined(Darwin)
    #define FSEEK64 fseeko
#endif

#ifndef FSEEK64
    #define FSEEK64 fseek64
#endif

// 64-bit fopen
//

#ifdef FOPEN64
    #undef FOPEN64
#endif

#if defined(WIN32) || defined(Darwin)
    #define FOPEN64 fopen
#endif

#ifndef FOPEN64
    #define FOPEN64 fopen64
#endif

// 64-bit stat
//

#ifdef STAT64
    #undef STAT64
#endif

#ifdef STAT64_T
    #undef STAT64_T
#endif

#if defined(WIN32)
    #define STAT64_T _stat
    #define STAT64   _stat
#endif

#if defined(Darwin)
    #define STAT64_T stat
    #define STAT64   stat
#endif

#if defined(__CYGWIN__)
    #define STAT64_T stat
    #define STAT64   stat
#endif

#ifndef STAT64
    #define STAT64_T stat64
    #define STAT64   stat64
#endif

#ifndef TIME64_T
    #ifdef WIN32
        #define TIME64_T __int64
    #else
        #define TIME64_T int64_t
    #endif
#endif

#endif    // MYBASE_H
