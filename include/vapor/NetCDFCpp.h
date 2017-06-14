#include <vector>
#include <map>
#include <iostream>
#include <netcdf.h>
#include <vapor/utils.h>
#include <vapor/MyBase.h>

#ifndef	_NetCDFCpp_H_
#define	_NetCDFCpp_H_

namespace VAPoR {

//! \class NetCDFCpp
//! \ingroup Public_VDC
//! \brief Defines simple C++ wrapper for NetCDF
//!
//! This class provdies a simple object-oriented wrapper for the NetCDF
//! API C language binding. In most cases the member functions provided
//! by this class are identical or near identical to the NetCDF API 
//! functions of the same name. Only when significant differences
//! exist  between 
//! the NetCDF native functions and the member functions provided 
//! herein is anything other than brief documention provided.
//!
//! The ordering of dimension and coordinate parameters specified
//! as arrays or STL vectors follows that of
//! NetCDF: The first element is the slowest varying dimension, the
//! second element is the next slowest, and so on. \note This ordering
//! is the opposite of that used by the VAPoR::VDC class.
//!
//! One particular difference of note: the various identifiers used
//! by NetCDF (e.g. variable id, dimesion id, etc) are not exposed by
//! the NetCDFCpp class methods. These objects are instead referred to by
//! their ascii string names. Moreover, as the file access methods
//! Open() and Create() do not return a NetCDF file identifier, only a single
//! NetCDF file may be opened at a time (multiple NetCDF files may be
//! opened, if needed, by instantiating multiple NetCDFCpp objects).
//!
//! Unless otherwise noted the return value of any member function that 
//! returns an integer may be interpreted as status. A negative value
//! indicates an error. Upon error an error message will
//! be logged via Wasp::MyBase::SetErrMsg().
//
class WASP_API NetCDFCpp : public Wasp::MyBase {
public:
 NetCDFCpp();
 virtual ~NetCDFCpp();

 //! Create a new NetCDF file
 //!
 //! Create an new NetCDF file named by 'path'. Any currently opened 
 //! NetCDF files
 //! are closed prior to attempting to create the named file
 //
 virtual int Create(
	string path, int cmode, size_t initialsz,
    size_t &bufrsizehintp
 );

 //! Open an existing NetCDF file
 //!
 //! Open an existing named file. Any currently opened NetCDF files
 //! are closed prior to attempting to open the named file
 //
 virtual int Open(string path, int mode);

 //! Define a dimension
 virtual int DefDim(string name, size_t len) const;

 //! Define a variable
 virtual int DefVar(
	string name, int xtype, vector <string> dimnames
 );

 //! Learn the dimension names associated with a variable
 virtual int InqVarDims(
	string name, vector <string> &dimnames, vector <size_t> &dims
 ) const;

 //! Learn the dimension names and lengths defined in a file
 //
 virtual int InqDims(
	vector <string> &dimnames, vector <size_t> &dims
 ) const;

 //! Learn the length of a named dimension
 virtual int InqDimlen(string name, size_t &len) const;

 //! Learn the names of all the global or variable attributes
 //
 int InqAttnames(string varname, std::vector <string> &attnames) const;


 //! Copy global or variable attribute 
 //!
 //! This method copies an attribute from one open netCDF dataset to another. 
 //! It can also be used to copy an attribute from one variable to another 
 //! within the same netCDF.
 //!
 //! \param[in] varname_in Name of source variable, empty if global
 //! \param[in] attname Name of attribute to copy
 //! \param[in] ncdf_out Destination NetCDF object
 //! \param[in] varname_out Name of destination variable, empty if global
 //!
 int CopyAtt(
	string varname_in, string attname, NetCDFCpp &ncdf_out, string varname_out
 ) const;



 //
 // PutAtt - Integer
 //

 //! Write an integer attribute
 virtual int PutAtt(
	string varname, string attname, int value
 ) const; 
 virtual int PutAtt(
	string varname, string attname, vector <int> values
 ) const; 
 virtual int PutAtt(
	string varname, string attname, const int values[], size_t n
 ) const; 

 //
 // GetAtt - Integer
 //

 //! Read an integer attribute
 virtual int GetAtt(
	string varname, string attname, int &value
 ) const;
 virtual int GetAtt(
	string varname, string attname, vector <int> &values
 ) const; 
 virtual int GetAtt(
	string varname, string attname, int values[], size_t n
 ) const; 

 //
 // PutAtt - size_t
 //
 virtual int PutAtt(
	string varname, string attname, size_t value
 ) const; 
 virtual int PutAtt(
	string varname, string attname, vector <size_t> values
 ) const; 
 virtual int PutAtt(
	string varname, string attname, 
	const size_t values[], size_t n
 ) const; 

 //
 // GetAtt - size_t
 //
 virtual int GetAtt(
	string varname, string attname, size_t &value
 ) const; 
 virtual int GetAtt(
	string varname, string attname, vector <size_t> &values
 ) const; 
 virtual int GetAtt(
	string varname, string attname, size_t values[], size_t n
 ) const; 

 //
 // PutAtt - Float
 //
 virtual int PutAtt(
	string varname, string attname, float value
 ) const; 
 virtual int PutAtt(
	string varname, string attname, vector <float> values
 ) const;
 virtual int PutAtt(
	string varname, string attname, const float values[], size_t n
 ) const; 

 //
 // GetAtt - Float
 //
 virtual int GetAtt(
	string varname, string attname, float &value
 ) const; 
 virtual int GetAtt(
	string varname, string attname, vector <float> &values
 ) const; 
 virtual int GetAtt(
	string varname, string attname, float values[], size_t n
 ) const; 

 //
 // PutAtt - Double
 //
 virtual int PutAtt(
	string varname, string attname, double value
 ) const; 
 virtual int PutAtt(
	string varname, string attname, vector <double> values
 ) const;
 virtual int PutAtt(
	string varname, string attname, const double values[], size_t n
 ) const; 

 //
 // GetAtt - Double
 //
 virtual int GetAtt(
	string varname, string attname, double &value
 ) const; 
 virtual int GetAtt(
	string varname, string attname, vector <double> &values
 ) const; 
 virtual int GetAtt(
	string varname, string attname, double values[], size_t n
 ) const; 

 //
 // PutAtt - String
 //
 virtual int PutAtt(
	string varname, string attname, string value
 ) const; 
 virtual int PutAtt(
	string varname, string attname, vector <string> values
 ) const;
 virtual int PutAtt(
	string varname, string attname, const char values[], size_t n
 ) const; 

 //
 // GetAtt - String
 //
 virtual int GetAtt(
	string varname, string attname, string &value
 ) const; 
 virtual int GetAtt(
	string varname, string attname, char values[], size_t n
 ) const; 

 //! Return a text attribute as a vector of strings
 //!
 //! This method attempts to return the value of the named 
 //! text attribute as a vector of words. The value of the
 //! attribute named by \p attname is tokenized (parsed) using
 //! white space as a delimeter. The extracted tokens are returned
 //! in the order of occurence in the vector \p values.
 //
 virtual int GetAtt(
	string varname, string attname, vector <string> &values
 ) const; 

 //! Find the NetCDF ID of a variable
 virtual int InqVarid(string varname, int &varid ) const; 

 //! Return information about a NetCDF attribute
 virtual int InqAtt(
	string varname, string attname, nc_type &xtype, size_t &len
 ) const;

 //! Find a variable's external representation type
 //
 virtual int InqVartype(string varname, nc_type &xtype) const;

 //! Set the fill value
 virtual int SetFill(int fillmode, int &old_modep);

 //! End the metadata definition section 
 virtual int EndDef() const;

 //! Close the currently opened file
 virtual int Close();

 //! Write an array of values to a variable
 virtual int PutVara(
	string varname,
	vector <size_t> start, vector <size_t> count, const void *data
 );
 virtual int PutVara(
	string varname,
	vector <size_t> start, vector <size_t> count, const float *data
 );
 virtual int PutVara(
	string varname,
	vector <size_t> start, vector <size_t> count, const double *data
 );
 virtual int PutVara(
	string varname,
	vector <size_t> start, vector <size_t> count, const int *data
 );
 virtual int PutVara(
	string varname,
	vector <size_t> start, vector <size_t> count, const long *data
 );
 virtual int PutVara(
	string varname,
	vector <size_t> start, vector <size_t> count, const unsigned char *data
 );

 //! Write an entire variable with one function call
 virtual int PutVar(string varname, const void *data);
 virtual int PutVar(string varname, const float *data);
 virtual int PutVar(string varname, const double *data);
 virtual int PutVar(string varname, const int *data);
 virtual int PutVar(string varname, const long *data);
 virtual int PutVar(string varname, const unsigned char *data);

 //! Read an array of values from a variable
 virtual int GetVara(
	string varname,
	vector <size_t> start, vector <size_t> count, void *data
 ) const;
 virtual int GetVara(
	string varname,
	vector <size_t> start, vector <size_t> count, float *data
 ) const;
 virtual int GetVara(
	string varname,
	vector <size_t> start, vector <size_t> count, double *data
 ) const;
 virtual int GetVara(
	string varname,
	vector <size_t> start, vector <size_t> count, int *data
 ) const;
 virtual int GetVara(
	string varname,
	vector <size_t> start, vector <size_t> count, long *data
 ) const;
 virtual int GetVara(
	string varname,
	vector <size_t> start, vector <size_t> count, unsigned char *data
 ) const;

 //! Read an entire variable with one function call
 virtual int GetVar(string varname, void *data) const;
 virtual int GetVar(string varname, float *data) const;
 virtual int GetVar(string varname, double *data) const;
 virtual int GetVar(string varname, int *data) const;
 virtual int GetVar(string varname, long *data) const;
 virtual int GetVar(string varname, unsigned char *data) const;

 //! Copy a variable from one file to another
 //
 virtual int CopyVar(string varname, NetCDFCpp &ncdf_out) const;


 //! Return the size in bytes of a NetCDF external data type
 //!
 static size_t SizeOf(int nctype);

 //! Return true if file exists and is a valid NetCDF file
 //!
 //! Returns true if both the file specified by \p path exists, and
 //! it can be opened with nc_open(). 
 //!
 virtual bool ValidFile(string path);

 //! Returns true if the named dimension is defined
 //!
 //! \param[in] dimname A NetCDF dimension name
 //
 virtual bool InqDimDefined(string dimname);

 //! Returns true if the named attribute is defined
 //!
 //! \param[in] dimname A NetCDF dimension name
 //
 virtual bool InqAttDefined(string varname, string attname);

 //! Learn the names of the user defined variables present
 //!
 //! \param[out] varnames A vector containing the names of all of the
 //! variables defined in the presently opened NetCDF file.
 //
 virtual int InqVarnames(vector <string> &varnames) const;


 //! Return netCDF ID associated with this object
 //!
 int GetNCID() const {return(_ncid); }

private:

 int _ncid;
 string _path;

 int _PutVara(
	string varname, vector <size_t> start, vector <size_t> count,
	const void *data, string func
 );
 int _PutVar( string varname, const void *data, string func);

 int _GetVara(
	string varname, vector <size_t> start, vector <size_t> count,
	void *data, string func
 ) const;
 int _GetVar( string varname, void *data, string func) const;

};

}

#endif //	_NetCDFCpp_H_
