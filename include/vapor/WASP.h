
#ifndef	_WASP_H_
#define	_WASP_H_

#include <vector>
#include <map>
#include <iostream>
#include <netcdf.h>
#include <vapor/NetCDFCpp.h>
#include <vapor/Compressor.h>
#include <vapor/EasyThreads.h>
#include <vapor/utils.h>

namespace VAPoR {

//! \class WASP
//! \ingroup Public_VDC
//! \brief Implements WASP compression conventions for NetCDF
//! 
//! \author John Clyne
//! \date    July, 2014
//!
//! Implements WASP compression conventions for NetCDF by extending
//! the NetCDFCPP class.
//!
//! The WASP conventions establish a policy for compressing, storing,
//! and accessing arrays of data in NetCDF. This API provides 
//! an interface for NetCDF data adhering to the WASP conventions.
//!
//! Fundamental concepts of the WASP compression conventions include the 
//! following:
//!
//! \li \b Blocking Compressed arrays are decomposed into fixed size blocks,
//! and each block is compressed individually and atomically. The rank of the 
//! block may be equal-to, or less-than that of the array. In the latter
//! case blocking (and subsequent compression) is only performed on the
//! the \em n fastest varying array dimensions, where \em n is the rank of
//! the block.
//!
//! \li \b Transformation Each block is transformed prior to compression
//! in an effort to decorrelate (remove redundancy) from the data. The
//! transformation process is typically lossless up to floating point 
//! round off.
//!
//! \li \b Multi-resolution Some transforms, notably wavelets, exhibit the
//! the property of multi-resolution: arrays can be reconstructed from 
//! transformed coefficients at progressively finer resolution. Resolution
//! levels in the WASP API are specified with a \b level parameter, in the
//! range 0..max, where 0 is the coarsest resolution possible for a given
//! variable, and \em max corresponds to the original resolution of the 
//! array. The value of \b -1 is an alias for \em max.
//!
//! \li <b> Progressive access </b> Aka embedded encoding, is the property by
//! which compressed data may be progressively refined during decoding by
//! transmitting (reading) more of the encoded data. The WASP API supports
//! progressive access, but supports a discrete form of refinement: only
//! a small, finite set of refinement levels, indicated by a \b lod 
//! parameter that indexes into an ordered vector of available
//! compression  ratios.
//!
//! This class inherits from Wasp::MyBase. Unless otherwise documented
//! any method that returns an integer value is returning status. A negative
//! value indicates failure. Error messages are logged via 
//! Wasp::MyBase::SetErrMsg(). In general, methods that return
//! boolean values will not record an error message on failure.
//!
//! \param wname Name of biorthogonal wavelet to use for data 
//! transformation. If not specified (if \p wname is the empty string) 
//! no transformation or compression are performed. However, arrays
//! are still decomposed into blocks as per the \p bs parameter.
//! See VAPoR::WaveFiltBior.
//!
//! \param bs An ordered list of block dimensions that specifies the 
//! block decomposition of the variable. The rank of \p bs may be less
//! than that of a variable's array dimensions, in which case only 
//! the \b n fastest varying variable dimensions will be blocked, where
//! \b n is the rank of \p bs.
//!
//! \param cratios A monotonically decreasing vector of 
//! compression ratios. Each element of \p cratios is in 
//! the range 1 
//! (indicating no compression) to 
//! \b max, where \b max is the maximum compression supported by the 
//! specified combination of block size, \p bs, and 
//! wavelet (See InqCompressionInfo()). If the underlying NetCDF file was
//! created with the \b numfiles parameter greater than one then the
//! length of \p cratios must exactly match that of \b numfiles.
//!
//! \param lod An index into the \p cratios vector. A value of -1
//! may be used to index the last element of \p cratios
//! 
//! \param level Array dimensions refinement level for compressed
//! variables. A value of 0 indicates 
//! the coarsest refinement level available, a value of one indicates 
//! next coarsest, and so on. The finest resolution available is given
//! by InqVarNumRefLevels() - 1. If \p level is less than 0 it is interpreted
//! do indicate the finest grid resolution.
//
class WASP_API WASP : public VAPoR::NetCDFCpp {
public:

 //! default constructor
 //!
 //! Construct a WASP object
 //!
 //! \param[in] nthreads Number of parallel execution threads 
 //! to be run during encoding and decoding of compressed data. A value 
 //! of 0, the default, indicates that the thread count should be 
 //! determined by the environment in a platform-specific manner, for
 //! example using sysconf(_SC_NPROCESSORS_ONLN) under *nix OSes. 
 //!
 //
 WASP(int nthreads = 0);
 virtual ~WASP();

 //! Create a new NetCDF data set with support for WASP conventions
 //!
 //! \param[in] path The file base name of the new NetCDF data set
 //! \param[in] cmode Same as in NetCDFCpp::Create()
 //! \param[in] initialsz Same as in NetCDFCpp::Create()
 //! \param[in] bufrsizehintp Same as in NetCDFCpp::Create()
 //! \param[in] numfiles An integer greater than or equal to one indicating 
 //! whether compressed
 //! variables should be stored in separate files, one compression level
 //! (level of detail) per file. A value of one indicates that all 
 //! compression levels for 
 //! compressed variables should be stored in the single file specified by
 //! \p path. A value greater than one results in the creation of 
 //! \p numfile NetCDF files, each of which will contain a separate 
 //! compression level for any compressed variables. Variables that
 //! are not compressed will be stored in their entirety in the file
 //! named by \p path.
 //!
 //! \sa NetCDFCpp::Create(), NetCDFCpp::GetPaths()
 //
 virtual int Create(
	string path, int cmode, size_t initialsz,
    size_t &bufrsizehintp, int numfiles
 );

 //! Open an existing NetCDF file
 //!
 //! \param[in] path The file base name of the new NetCDF data set
 //! \param[in] mode Same as in NetCDFCpp::Open()
 //!
 //! \sa NetCDFCpp::Open()
 //
 virtual int Open(string path, int mode);

 //! \copydoc NetCDFCpp::SetFill()
 //
 virtual int SetFill(int fillmode, int &old_modep);

 //! \copydoc NetCDFCpp::EndDef()
 //
 virtual int EndDef() const; 

 //! Close an open NetCDF file
 //!
 //! This method closes any currently opened NetCDF files that were
 //! opened with Create() or Open(). 
 //!
 virtual int Close();



 //! Return the dimension lengths associated with a variable.
 //!
 //! Returns the dimensions of the named variable at the 
 //! multi-resolution level indicated by \p level.  If the variable
 //! does not support multi-resolution (is not compressed with a 
 //! multi-resolution transform) the \p level parameter is ignored,
 //! the variable's native dimensions will be returned, and the value
 //! of \p bs will not be undefined.
 //!
 //! \param[in] name Name of NetCDF variable
 //! \param[in] level Grid (dimension) refinement level. A value of 0 indicates 
 //! the coarsest refinement level available, a value of one indicates 
 //! next coarsest, and so on. The finest resolution available is given
 //! by InqVarNumRefLevels() - 1. If \p level is less than 0 it is interpreted
 //! do indicate the finest grid resolution.
 //! 
 //! \param[out] dims Ordered list of variable's \p name dimension lengths
 //! at the grid hierarchy level indicated by \p level
 //! \param[out] bs Ordered list of block dimension lengths
 //! at the grid hierarchy level indicated by \p level
 //!
 //! \sa NetCDFCpp::InqVarDims(), InqVarNumRefLevels(),
 //! InqDimsAtLevel()
 //
 virtual int InqVarDimlens(
	string name, int level,  vector <size_t> &dims, vector <size_t> &bs
 ) const;

 //! \copydoc NetCDFCpp::NetCDFCpp()
 //
 virtual int InqVarDims(
    string name, vector <string> &dimnames, vector <size_t> &dims
 ) const;

 //! Returns compression paramaters associated with the named variable
 //!
 //! This method returns various compression parameters associated
 //! with a compressed variabled named by \p name. If the variable
 //! \p name is not compressed \p wname will be empty. If the variable
 //! is not blocked \p bs will be either empty or all elements 
 //! set to one.
 //!
 //! \param[in] name The variable name.
 //! \param[out] wname The name of the wavelet used to transform the variable.
 //! \param[out] bs An ordered list of block dimensions that specifies the 
 //! block decomposition of the variable. 
 //! \param[out] cratios The compression ratios available.
 //! 
 virtual int InqVarCompressionParams(
    string name, string &wname, vector <size_t> &bs, vector <size_t> &cratios
 ) const;

 //! Return the dimensions of a multi-resolution grid at a specified level in
 //! the hierarchy
 //!
 //! This static method calculates the coarsened dimensions of a 
 //! grid at a specified level in a multiresolution wavelet hierarchy.
 //! The dimensions of an array are determined by the combination of 
 //! the multi-resolution wavelet used, specified by \p wname, the 
 //! refinement level, \p level, in the multi-resolution hierarchy,
 //! the rank and dimension of the decomposition block, specified by
 //! \p bs, and the dimensions of the native (original) grid, \p dims.
 //!
 //! \param[in] wname wavelet name
 //! \param[in] level Grid (dimension) refinement level. A value of 0 indicates 
 //! the coarsest refinement level available, a value of one indicates 
 //! next coarsest, and so on. The finest resolution available is given
 //! by InqVarNumRefLevels() - 1. If \p level is less than 0 it is interpreted
 //! do indicate the finest grid resolution.
 //! \param[in] dims Dimensions of native grid
 //! \param[in] bs Dimensions of native decomposition block. The rank of 
 //! \p bs may be less than or equal to the rank of \p dims.
 //! \param[out] dims_at_level Computed grid dimensions at the specified level
 //! \param[out] bs_at_level Computed block dimensions at the specified level
 //!
 //! \sa VarOpenRead()
 //
 static int InqDimsAtLevel(
    string wname, int level, vector <size_t> dims, vector <size_t> bs,
    vector <size_t> &dims_at_level, vector <size_t> &bs_at_level
 );

 //! Return the number of levels available in a variable's multiresolution
 //! hierarchy.
 //!
 //! Returns the depth of the multi-resolution hierarchy for the variable
 //! specified by \p name. If the variable
 //! does not support multi-resolution (is not compressed with a
 //! multi-resolution transform) value of 1 is returned. 
 //!
 //! \param[in] name The name of the variable
 //!
 //! \retval depth Upon success the number of levels in hierarchy
 //! are returned. If \p varname does not specify a known variable
 //! a -1 is returned. 
 //
 virtual int InqVarNumRefLevels(string name) const;

 //! Compute the number of levels in a multi-resolution hierarchy
 //!
 //! This static method computes and returns the depth (number of levels) in a
 //! a multi-resolution hierarch for a given wavelet, \p wname,
 //! and decomposition block, \p bs.
 //! It also computes the maximum compression ratio, \p cratio, possible 
 //! for the 
 //! the specified combination of block size, \p bs, and wavelet, \p wname.
 //! The maximum compression ratio is \p cratio:1.
 //!
 //! \param[in] wname wavelet name
 //! \param[in] bs Dimensions of native decomposition block. The rank of 
 //! \p bs may be less than or equal to the rank of \p dims.
 //! \param[out] nlevels Number of levels in hierarchy
 //! \param[out] maxcratio Maximum compression ratio
 //!
 //! bool status If \p bs, \p wname, or the combination there of is invalid
 //! false is returned and the values of \p nlevels and \p maxcratio are
 //! undefined. Upon success true is returned.
 //! 
 static bool InqCompressionInfo(
	vector <size_t> bs, string wname, size_t &nlevels, size_t &maxcratio
 );

 //! Define a new compressed variable
 //!
 //! \param[in] name Same as NetCDFCpp::DefVar()
 //! \param[in] xtype Same as NetCDFCpp::DefVar()
 //! \param[in] dimnames Same as NetCDFCpp::DefVar()
 //! \param[in] wname Name of biorthogonal wavelet to use for data 
 //! transformation. See VAPoR::WaveFiltBior. If empty, the variable
 //! will be blocked according to \p bs, but will not be compressed.
 //! \param[in] bs An ordered list of block dimensions that specifies the 
 //! block decomposition of the variable. 
 //! array's associated dimension. The rank of \p bs may be equal to 
 //! or less than 
 //! that of \p dimnames. In the latter case only the rank(bs) fastest
 //! varying dimensions of the variable will be blocked.
 //! The dimension(s) of \p bs[i] need not align with (be integral factors
 //! of) the dimension lengths
 //! associated with \p dimnames in which case boundary blocks will be
 //! padded. If \p bs is empty, or the product of its elements is one, 
 //! the variable will not be blocked or compressed. Hence, the 
 //! \p wname and \p cratio parameters will be ignored. The variable
 //! will not be defined as a \b WASP variable. See InqVarWASP().
 //! \param[in] cratios A monotonically decreasing vector of 
 //! compression ratios. Each element of \p cratios is in 
 //! the range 1 
 //! (indicating no compression) to 
 //! \b max, where \b max is the maximum compression supported by the 
 //! specified combination of block size, \p bs, and 
 //! wavelet (See InqCompressionInfo()). If the underlying NetCDF file was
 //! created with \b numfiles parameter greater than one then the
 //! length of \p cratios must exactly match that of \b numfiles.
 //!
 //! \sa NetCDFCpp::DefVar(), Create(), InqCompressionInfo()
 //
 virtual int DefVar(
    string name, int xtype, vector <string> dimnames, 
	string wname, vector <size_t> bs, vector <size_t> cratios
 );

 //! Define a compressed variable with missing data values
 //!
 //! The defined variable may contain missing data values. These 
 //! values will not be transformed and compressed
 //!
 //! \copydoc DefVar(
 //!	string name, int xtype, vector <string> dimnames, 
 //!	string wname, vector <size_t> bs, vector <size_t> cratios
 //! )
 //!
 //! \param[in] missing_value Value of missing value indicator. 
 //!
 //! \sa NetCDFCpp::DefVar()
 //!
 virtual int DefVar(
	string name, int xtype, vector <string> dimnames, 
	string wname, vector <size_t> bs, vector <size_t> cratios,
	double missing_value
 );

 //! \copydoc NetCDFCpp::DefVar()
 // Is this needed?
 virtual int DefVar(
    string name, int xtype, vector <string> dimnames
 ) {
	return(NetCDFCpp::DefVar(name, xtype, dimnames)); 
 };

 //! \copydoc NetCDFCpp::DefDim()
 //
 int DefDim(string name, size_t len) const;


 //! Inquire whether a named variable is compressed
 //!
 //! \param[in] name The name of the variable
 //! \param[out] compressed A boolean return value indicating whether
 //! variable \p name is compressed
 //
 virtual int InqVarCompressed(
    string varname, bool &compressed
 ) const;

 //! Inquire whether a variable is a WASP variable
 //!
 //! This method returns true if the variable named by \p varname
 //! was defined by the WASP API and is either compressed, blocked, or
 //! both.
 //!
 int InqVarWASP(
    string varname, bool &wasp
 ) const;

 //! Prepare a variable for writing 
 //!
 //! Compressed or blocked variables must be opened prior to writing.
 //! This method initializes the variable named by \p name for writing
 //! using the PutVara() method. If the variable is defined as compressed
 //! the \p lod parameter indicates which compression levels will be stored.
 //! Valid values for \p lod are in the range 0..max, where \p max the size
 //! of cratios - 1.
 //!
 //! Any currently opened variable is first closed with Close()
 //!
 //! \param[in] name Name of variable
 //! \param[in] lod Level-of-detail to save. If not -1, all LOD's from
 //! 0 to \p lod will subsequently be written. 
 //!
 //! \note Is \p lod needed? Since cratios can be specified on a per
 //! variable basis perhaps this is not needed? For single file
 //! representations it would be better to limit the lod using cratios,
 //! which will result in a smaller file.
 //!
 //! \sa PutVara(), 
 //
 virtual int OpenVarWrite(string name, int lod);

 //! Prepare a variable for reading 
 //!
 //! Compressed or blocked variables must be opened prior to reading.
 //! This method initializes the variable named by \p name for reading
 //! using the GetVara() or GetVar() methods. If the variable is 
 //! defined as compressed
 //! the \p lod parameter indicates which compression levels will used
 //! during reconstruction of the variable.
 //! Valid values for \p lod are in the range 0..max, where \p max the size
 //! of \b cratios - 1.
 //! If the transform used to compress this variable supports
 //! multiresolution then the \p level parameter indicates the 
 //! grid hierarchy refinement level for which to reconstruct the data.
 //!
 //! Any currently opened variable is first closed with Close()
 //!
 //! \param[in] name Name of variable
 //! \param[in] lod Level-of-detail to read. 
 //! \param[in] level Grid refinement level
 //!
 //! \sa GetVara()
 //
 virtual int OpenVarRead(string name, int level, int lod);

 //! Close the currently opened variable
 //!
 //! If a variable is opened for writing this method will flush 
 //! all buffers to disk and perform cleanup. If opened for reading
 //! only cleanup is performed. If no variables are open this method
 //! is a no-op.
 //
 //!
 virtual int CloseVar();

 //! Write an array of values to the currently opened variable
 //!
 //! The currently opened variable may or may not be a WASP
 //! variable (See InqVarWASP()).
 //!
 //! The combination of \p start and \p count specify the 
 //! coordinates of a hyperslab to write as described by
 //! NetCDFCpp::PutVara(). However, for blocked data dimensions
 //! the values of \p start and \p count must be block aligned  
 //! unless the hyperslab includes the array boundary, in which case
 //! the hyperslab must be aligned to the boundary. 
 //!
 //! \param[in] start A vector of block-aligned integers specifying 
 //! the index in the variable where the first of the data values will 
 //! be read. See NetCDFCpp::PutVara() 
 //! \param[in] count A vector of size_t, block-aligned integers 
 //! specifying the edge 
 //! lengths along each dimension of the block of data values to be read. 
 //! See NetCDFCpp::PutVara()
 //! \param[in] data Same as NetCDFCpp::PutVara()
 //!
 //! \sa OpenVarWrite();
 //
 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const float *data
 );
 virtual int PutVar(const float *data);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const double *data
 );
 virtual int PutVar(const double *data);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const int *data
 );
 virtual int PutVar(const int *data);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const int16_t *data
 );
 virtual int PutVar(const int16_t *data);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const unsigned char *data
 );
 virtual int PutVar(const unsigned char *data);

 //! Write an array of masked values to the currently opened variable
 //!
 //! This version of PutVar() handles missing data values whose 
 //! presence is indicated by a boolean mask, \p mask. Missing values
 //! are replaced with values that perform better when compressed (e.g. the
 //! average of the field)
 //!
 //! \copydoc PutVar(
 //!    vector <size_t> start, vector <size_t> count, const float *data
 //! )
 //!
 //! \param[in] mask a boolean array with the same shape as \p data
 //! indicting valid and invalid values in \p data. Elements of \p data
 //! corresponding to false values in \p mask are not preserved when 
 //! written to storage.
 //!
 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const float *data,
	const unsigned char *mask
 );
 virtual int PutVar(const float *data, const unsigned char *mask);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const double *data,
	const unsigned char *mask
 );
 virtual int PutVar(const double *data, const unsigned char *mask);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const int *data,
	const unsigned char *mask
 );
 virtual int PutVar(const int *data, const unsigned char *mask);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const int16_t *data,
	const unsigned char *mask
 );
 virtual int PutVar(const int16_t *data, const unsigned char *mask);

 virtual int PutVara(
	vector <size_t> start, vector <size_t> count, const unsigned char *data,
	const unsigned char *mask
 );
 virtual int PutVar(const unsigned char *data, const unsigned char *mask);

 //! Read a hyper-slab of values from the currently opened variable
 //!
 //! The currently opened variable may or may not be a WASP
 //! variable (See InqVarWASP()).
 //!
 //! If a compressed variable is being read and the transform
 //! supports multi-resolution the method InqVarDimlens()
 //! should be be used to determine the dimensions of the variable
 //! at the opened refinement level.
 //!
 //! \param[in] start A vector of size_t integers specifying the index in 
 //! the variable where the first of the data values will be read.
 //! The coordinates are specified relative to the dimensions of the
 //! array at the currently opened refinement level.
 //! See NetCDFCpp::InqVarDimlens()
 //! \param[in] count  A vector of size_t integers specifying the 
 //! edge lengths along each dimension of the hyperslab of data values to 
 //! be read.
 //! The coordinates are specified relative to the dimensions of the
 //! array at the currently opened refinement level.  See NetCDFCpp::PutVara()
 //! \param[out] data Same as NetCDFCpp::PutVara()
 //!
 //! \sa InqVarDimlens(), OpenVarRead()
 //
 virtual int GetVara(
	vector <size_t> start, vector <size_t> count, float *data
 );
 virtual int GetVara(
	vector <size_t> start, vector <size_t> count, double *data
 );
 virtual int GetVara(
	vector <size_t> start, vector <size_t> count, int *data
 );
 virtual int GetVara(
	vector <size_t> start, vector <size_t> count, int16_t *data
 );

 virtual int GetVara(
	vector <size_t> start, vector <size_t> count, unsigned char *data
 );

 //! Read a hyper-slab of blocked values from currently opened variable
 //!
 //! This method is identical to GetVara() with the exceptions
 //! that: 
 //! \li The vectors \p start and \p count must be aligned
 //! with the underlying storage block of the variable. See
 //! WASP::DefVar()
 //!
 //! \li The hyperslab copied to \p data will preserve its underlying
 //! storage blocking (the data will not be contiguous)
 //!
 //! \param[in] start A block-aligned vector of size_t integers specifying 
 //! the index in 
 //! the variable where the first of the data values will be read.
 //! \param[in] count  A block-aligned vector of size_t integers specifying the 
 //! edge lengths along each dimension of the hyperslab of data values to 
 //! be read.
 //!
 //! \sa WASP::DefVar()
 //
 virtual int GetVaraBlock(
	vector <size_t> start, vector <size_t> count, float *data
 );
 virtual int GetVaraBlock(
	vector <size_t> start, vector <size_t> count, double *data
 );
 virtual int GetVaraBlock(
	vector <size_t> start, vector <size_t> count, int *data
 );
 virtual int GetVaraBlock(
	vector <size_t> start, vector <size_t> count, int16_t *data
 );
 virtual int GetVaraBlock(
	vector <size_t> start, vector <size_t> count, unsigned char *data
 );

 //! Read an array of values from the currently opened variable
 //!
 //! The currently opened variable may or may not be a WASP
 //! variable (See InqVarWASP()).
 //!
 //! The entire variable is read and copied into the array pointed to
 //! by \p data. The caller is responsible for ensuring that
 //! adequate space is availble in \p data.
 //!
 //! If a compressed variable is being read and the transform
 //! supports multi-resolution the method InqVarDimlens()
 //! should be be used to determine the dimensions of the variable
 //! at the opened refinement level
 //!
 //! \param[in] data Same as NetCDFCpp::PutVara()
 //!
 //! \sa InqVarDimlens(), OpenVarRead()
 //
 virtual int GetVar(float *data);
 virtual int GetVar(double *data);
 virtual int GetVar(int *data);
 virtual int GetVar(int16_t *data);
 virtual int GetVar(unsigned char *data);

 //! Copy a variable from one WASP file to another WASP file
 //!
 //! Copy a variable from the WASP file associated with this
 //! object instance to the WASP file associated with \p wasp
 //! The variable \p varname must be defined in both the source
 //! and destination files, and must have matching dimensions.
 //!
 //! If the source and destination variables are compressed this method
 //! attempts to copy data verbatim, avoiding decoding and encoding.
 //
 virtual int CopyVar(string varname, WASP &wasp);

 //! Copy a variable from a NetCDF file to a WASP file
 //!
 //! Copy a variable from the NetCDF file associated with \p ncdf
 //! to the WASP file associated with this object instance. 
 //! The variable \p varname must be defined in both the source
 //! and destination files, and must have matching dimensions.
 //
 virtual int CopyVarFrom(string varname, NetCDFCpp &ncdf);

 //! Copy a variable from a WASP file to a NetCDF file
 //!
 //! Copy a variable from the WASP file associated this object instance
 //! to the WASP file associated with \p ncdf.
 //! The variable \p varname must be defined in both the source
 //! and destination files, and must have matching dimensions.
 //
 virtual int CopyVarTo(string varname, NetCDFCpp &ncdf);


 //! Return the NetCDF file paths that would be created from a base
 //! path.
 //!
 //!
 //! \param[in] path The file base name of the new NetCDF data set
 //! \param[in] numfiles An integer greater than or equal to one indicating 
 //! the number of files to split a variable into
 //! \retval vector The path names generated from \p path
 //!
 //! \sa Create()
 //
 static std::vector <string> GetPaths(string path, int numfiles) {
	if (numfiles > 1) {
        return(mkmultipaths(path, numfiles));
    }
    else {
		std::vector <string> t(1,path); return(t);
    }
 }

 //! NetCDF attribute name specifying Wavelet name
 static string AttNameWavelet() {return("WASP.Wavelet");}

 //! NetCDF attribute name specifying compression block dimensions 
 static string AttNameBlockSize() {return("WASP.BlockSize");}

 //! NetCDF attribute name specifying number of compression files
 static string AttNameNumFiles() {return("WASP.NumFiles");}

 //! NetCDF attribute name specifying compression ratios
 static string AttNameCRatios() {return("WASP.CRatios");}

 //! NetCDF attribute name specifying if this is a WASP file or
 //! variable
 static string AttNameWASP() {return("WASP");}

 //! NetCDF attribute name specifying names of uncompressed dimensions
 static string AttNameDimNames() {return("WASP.DimNames");}

 //! NetCDF attribute name specifying if missing data values are present
 static string AttNameMissingValue() {return("WASP.MissingValue");}

 //! NetCDF attribute name specifying WASP version number
 static string AttNameVersion() {return("WASP.Version");}


private:

 Wasp::EasyThreads *_et;
 int _nthreads;
 vector <NetCDFCpp> _ncdfcs;
 vector <NetCDFCpp *> _ncdfcptrs;	// pointers into _ncdfcs;
 bool _waspFile; // Is this a WASP file
 int _numfiles; // Number of NetCDF files 
 int _currentVersion; // Current WASP version number;
 int _fileVersion; // version number of opened file;
 Wasp::SmartBuf _blockbuf;    // Dynamic storage for blocks
 Wasp::SmartBuf _coeffbuf;    // Dynamic storage wavelet coefficients
 Wasp::SmartBuf _sigbuf;  // Dynamic storage encoded signficance maps

 bool _open;    // compressed variable open for reading or writing?
 string _open_wname;  // wavelet name of opened variable
 vector <size_t> _open_bs;  // block size of opened variable
 vector <size_t> _open_cratios; // compression ratios of opened variable
 vector <size_t> _open_udims;   // uncompressed dims of opened variable
 vector <size_t> _open_dims;    // compressed dims of opened variable
 int _open_lod; // level-of-detail of opened variable
 int _open_level;   // grid refinement level of opened variable
 bool _open_write;  // opened variable open for writing?
 bool _open_waspvar;	// opened variable is a WASP variable?
 string _open_varname;  // name of opened variable
 nc_type _open_varxtype;  // external type of opened variable
 vector <Compressor *> _open_compressors;  // Compressor for opened variable


 int _GetBlockAlignedDims(
	vector <string> dimnames,
	vector <size_t> bs,
	vector <string> &badimnames,
	vector <size_t> &badims
 ) const;

 int _GetCompressedDims(
    vector <string> dimnames,
    string wname,
    vector <size_t> bs,
    vector <size_t> cratios,
	int xtype,
    vector <string> &cdimnames,
    vector <size_t> &cdims,
    vector <string> &encoded_dim_names,
    vector <size_t> &encoded_dims
 ) const;

 int _InqDimlen(string name, size_t &len) const;

 void _get_encoding_vectors(
    string wname, vector <size_t> bs, vector <size_t> cratios, int xtype,
    vector <size_t> &ncoeffs, vector <size_t> &encoded_dims
 ) const;


 bool _validate_compression_params(
	string wname, vector <size_t> dims, 
	vector <size_t> bs, vector <size_t> cratios
 ) const;

 bool _validate_put_vara_compressed(
    vector <size_t> start, vector <size_t> count, 
    vector <size_t> bs, vector <size_t> udims, vector <size_t> cratios
 ) const;

 bool _validate_get_vara_compressed(
	vector <size_t> start, vector <size_t> count,
	vector <size_t> bs, vector <size_t> udims, vector <size_t> cratios,
	bool unblock
 ) const;

 int _get_compression_params(
    string name, vector <size_t> &bs, vector <size_t> &cratios,
    vector <size_t> &udims, vector <size_t> &dims, string &wname
 ) const;

 template <class T, class U>
 int _GetVara(
    vector <size_t> start, vector <size_t> count, bool unblock, T *data,
	U dummy
 );

 template <class T>
 int _GetVara(
	vector <size_t> start, vector <size_t> count, bool unblock_flag, T *data
 );

 static void _dims_at_level(
    vector <size_t> dims, vector <size_t> bs, int level,
	string wname, vector <size_t> &dims_level, vector <size_t> &bs_level
 ) ;

 static vector <string> mkmultipaths(string path, int n);


 template <class T, class U>
 int _PutVara(
	vector <size_t> start, vector <size_t> count, const T *data,
	const unsigned char *mask, U dummy
 );

 template <class T>
 int _PutVara(
	vector <size_t> start, vector <size_t> count, const T *data,
	const unsigned char *mask
 );

 template <class T>
 int _CopyHyperSlice(
	string varname, NetCDFCpp &src_ncdf, NetCDFCpp &dst_ncdf,
	vector <size_t> start, vector <size_t> count, T *buf
 ) const;

 int _CopyVar(
    string varname, NetCDFCpp &src_ncdf, NetCDFCpp &dst_ncdf
 ) const;


};

}

#endif //	_WASP_H_
