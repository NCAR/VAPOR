//
// $Id$
//
#ifndef	_WaveCodeIO_h_
#define	_WaveCodeIO_h_

#include <vapor/VDFIOBase.h>
#include <vapor/SignificanceMap.h>
#include <vapor/Compressor.h>
#include <vapor/EasyThreads.h>
#include <vapor/NCBuf.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

namespace VAPoR {

//
//! \class WaveCodecIO
//! \brief A sub-region reader for VDF files
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! This class provides an API for reading and writing 
//! VDC2 data. VDC2 data may be accessed with two forms of 
//! wavelet based progressive
//! refinement, : hierarchical or level-of-detail. The former supports 
//! coarsening and refinement of the sampling grid resolution (the 
//! dimensions of the sampling grid) at varying 
//! powers-of-two, and is controled by the \p reflevel parameter. The latter
//! allows for arbitrary approximations by restricting  the number of 
//! wavelet basis coefficients used when reconstructing the data from
//! their wavelet representation. The level-of-detail is controled with
//! the \p lod parameter.
//
class VDF_API WaveCodecIO : public VDFIOBase, protected Wasp::EasyThreads {
public:

 //! \copydoc VDFIOBase::VDFIOBase(MetadataVDC &)
 //!
 //! \param[in] nthreads Number of execution threads that may be used by
 //! the class for parallel execution. If zero, the system hardware will
 //! be queried via sysconf to determine the number of processors 
 //! available and this value will be used.
 //
 WaveCodecIO(const MetadataVDC &metadata, int nthreads = 0);

 //! \copydoc VDFIOBase::VDFIOBase(const string &)
 //!
 //! \param[in] nthreads Number of execution threads that may be used by
 //! the class for parallel execution. If zero, the system hardware will
 //! be queried via sysconf to determine the number of processors 
 //! available and this value will be used.
 //
 WaveCodecIO(const string &metafile, int nthreads = 0);

#ifdef	DEAD
 WaveCodecIO(
	const size_t dim[3], const size_t bs[3], int numTransforms, 
	const vector <size_t> cratios,
	const string &wname, const string &filebase
 );

 WaveCodecIO(const vector <string> &files);
#endif

 virtual ~WaveCodecIO();

 //! Open the named variable for reading
 //!
 //! This method prepares a data volume (slice), indicated by a
 //! variable name and time step pair, for subsequent read operations by
 //! methods of this class.  The number of the refinement levels
 //! parameter, \p reflevel, indicates the resolution of the volume in
 //! the multiresolution hierarchy. The valid range of values for
 //! \p reflevel is [0..max_refinement], where \p max_refinement is the
 //! maximum refinement level of the data set: Metadata::GetNumTransforms().
 //! A value of zero indicates the
 //! coarsest resolution data, a value of \p max_refinement (or -1) indicates 
 //! the
 //! finest resolution data.
 //! The level-of-detail parameter, \p lod, selects 
 //! the approximation level. Valid values for \p lod are integers in 
 //! the range 0..GetCRatios().size()-1, or the value -1 may be used
 //! to select the best approximation available: GetCRatios().size()-1.
 //!
 //! An error occurs, indicated by a negative return value, if the
 //! volume identified by the {varname, timestep, reflevel, lod} tupple
 //! is not available. Note the availability of a volume can be tested
 //! with the VariableExists() method.
 //!
 //! \param[in] timestep Time step of the variable to read
 //! \param[in] varname Name of the variable to read
 //! \param[in] reflevel Refinement level of the variable. A value of -1
 //! indicates the maximum refinment level defined for the VDC
 //! \param[in] lod Approximation level of the variable. A value of -1
 //! indicates the maximum approximation level defined for the VDC
 //! \retval status Returns a non-negative value on success
 //!
 //! \sa Metadata::GetVariableNames(), Metadata::GetNumTransforms(),
 //! GetNumTimeSteps(), GetCRatios()
 //!
 virtual int OpenVariableRead(
	size_t timestep, const char *varname, int reflevel=0, int lod=0
 );

 //! Open the named variable for writing
 //!
 //! Prepare a VDC for writing a data volume (slice).
 //! The data volume is identified by the specfied time step and
 //! variable name. The number of resulting approximations for 
 //! the data volume is determined by the Metadata object used to
 //! initialize the class. Moreover, the number of levels-of-detail actually
 //! saved to the data collection are determined by \p lod. If
 //! \p lod is maximum level of detail (or the special value -1, the default) 
 //! all of the wavelet coefficients are saved, and it is possible to 
 //! fully reconstruct the volume later without loss of information (beyond
 //! floating point round off).  
 //!
 //! \param[in] timestep Time step of the variable to read
 //! \param[in] varname Name of the variable to read
 //! \param[in] lod Level of detail saved. A value of -1
 //! indicates the maximum level of detail.
 //! 
 //! \retval status Returns a non-negative value on success
 //! \sa Metadata::GetVariableNames(), Metadata::GetNumTransforms()
 //!
 virtual int OpenVariableWrite(
	size_t timestep, const char *varname, 
	int reflevel=-1 /*ignored*/, int lod=-1
 );

 //! Close the currently opened variable.
 //!
 //! \sa OpenVariableWrite(), OpenVariableRead()
 //
 virtual int CloseVariable();

 //! Read in and return a subregion from the currently opened 
 //! data volume.
 //!
 //! The \p bmin and \p bmax vectors identify the minimum and
 //! maximum extents, in block coordinates, of the subregion of interest. The
 //! minimum valid value of 'bmin' is (0,0,0), the maximum valid value of
 //! \p bmax is (nbx-1,nby-1,nbz-1), where nx, ny, and nz are the 
 //! block dimensions of the volume at the currently opened refinement
 //! level as retuned by GetDimBlk().
 //! The volume
 //! returned is stored in the memory region pointed to by \p region. It
 //! is the caller's responsbility to ensure adequate space is available.
 //!
 //! \param[in] bmin Minimum region extents in block coordinates
 //! \param[in] bmax Maximum region extents in block coordinates
 //! \param[out] region The requested volume subregion
 //! \param[in] unblock If true, unblock the data before copying to \p region
 //!
 //! \retval status Returns a non-negative value on success
 //!
 //! \sa OpenVariableRead(), GetBlockSize(), MapVoxToBlk()
 //
 virtual int BlockReadRegion(
	const size_t bmin[3], const size_t bmax[3], float *region, bool unblock=true
 );

 //! Read in and return a subregion from the currently opened 
 //! data volume.
 //!
 //! This method is similar to BlockReadRegion() with the exception
 //! that the region bounds are specified in voxel coordinates.
 //! The \p min and \p max vectors identify the minimum and
 //! maximum extents, in voxel coordinates, of the subregion of interest. The
 //! minimum valid value of 'min' is (0,0,0), the maximum valid value of
 //! \p max is (nx-1,ny-1,nz-1), where nx, ny, and nz are the 
 //! voxel dimensions of the volume at the currently opened refinement
 //! level as returned by GetDim().
 //!
 //! The volume
 //! returned is stored in the memory region pointed to by \p region. It
 //! is the caller's responsbility to ensure adequate space is available.
 //!
 //! \param[in] min Minimum region extents in voxel coordinates
 //! \param[in] max Maximum region extents in voxel coordinates
 //! \param[out] region The requested volume subregion
 //!
 //! \retval status Returns a non-negative value on success
 //! \sa OpenVariableRead(), GetDim(), MapVoxToBlk()
 //
 virtual int ReadRegion(
	const size_t min[3], const size_t max[3], float *region
 );

 virtual int ReadRegion(float *region);

 //! Read the next volume slice from the currently opened file
 //!
 //! Read in and return a slice (2D array) of
 //! voxels from the currently opened data volume at the current 
 //! refinement level.
 //! Subsequent calls will read successive slices
 //! until the entire volume has been read.
 //! It is the caller's responsibility to ensure that the array pointed
 //! to by \p slice contains enough space to accomodate
 //! an NX by NY dimensioned slice, where NX is the dimesion of the
 //! volume along the X axis, specified
 //! in **voxels**, and NY is the Y axis dimension, as returned by
 //! GetDim().
 //!
 //! \note ReadSlice returns 0 if the entire volume has been read.
 //!
 //! \param[out] slice The requested volume slice
 //!
 //! \retval status Returns a non-negative value on success
 //! \sa OpenVariableRead(), Metadata::GetDim()
 //!
 //
 virtual int ReadSlice(float *slice);

 //! Write a volume subregion to the currently opened progressive
 //! access data volume.
 //!
 //! This method is identical to the WriteRegion() method with the exception
 //! that the region boundaries are defined in block, not voxel, coordinates.
 //! Secondly, unless the 'block' parameter  is set, the internal
 //! blocking of the data will be preserved. I.e. the data are assumed
 //! to already be blocked. 
 //!
 //! The number of voxels contained in \p region must be the product
 //! over i :
 //!
 //! \p (bmax[i] - \p bmin[i] + 1) * bs[i]
 //!
 //! where bs[i] is the ith dimension of the block size.
 //!
 //! \param[in] bmin Minimum region extents in block coordinates
 //! \param[in] bmax Maximum region extents in block coordinates
 //! \param[in] region The volume subregion to write
 //! \param[in] block If true, block the data before writing/transforming
 //!
 //! \retval status Returns a non-negative value on success
 //! \sa OpenVariableWrite() SetBoundarPadOnOff()
 //
 virtual int BlockWriteRegion(
	const float *region, const size_t bmin[3], const size_t bmax[3], 
	bool block=true
 );

 //! Write a volume subregion to the currently opened progressive
 //! access data volume.
 //!
 //! This method is identical to the WriteRegion() method with the exception
 //! that the region boundaries are defined in block, not voxel, coordinates.
 //! Secondly, unless the 'block' parameter  is set, the internal
 //! blocking of the data will be preserved. I.e. the data are assumed
 //! to already be blocked. 
 //!
 //! The number of voxels contained in \p region must be the product
 //! over i :
 //!
 //! \p (max[i] - \p min[i] + 1) 
 //!
 //! \param[in] min Minimum region extents in voxel coordinates
 //! \param[in] max Maximum region extents in voxel coordinates
 //! \param[in] region The volume subregion to write
 //!
 //! \retval status Returns a non-negative value on success
 //! \sa OpenVariableWrite(), GetBlockSize(), MapVoxToBlk()
 //! \sa SetBoundarPadOnOff()
 //!
 //! \note Unexpected results may be obtained if this method is
 //! invoked multiple times for adjacent regions if the region 
 //! boundaries do not coincide with block boundaries.
 //!
 virtual int WriteRegion(
	const float *region, const size_t min[3], const size_t max[3]
 );

 virtual int WriteRegion(
	const float *region
 );


 //! Write a single slice of voxels to the currently opened variable 
 //!
 //! Transform and write a single slice (2D array) of voxels to the variable
 //! indicated by the most recent call to OpenVariableWrite().
 //! The dimensions of a slices is NX by NY,
 //! where NX is the dimesion of the volume along the X axis, specified
 //! in voxels, and NY is the Y axis dimension.
 //!
 //! This method should be called exactly NZ times for each opened variable,
 //! where NZ is the dimension of the volume in voxels along the Z axis. Each
 //! invocation should pass a successive slice of volume data.
 //!
 //! \param[in] slice A slices of volume data
 //! \retval status Returns a non-negative value on success
 //! \sa OpenVariableRead()
 //!
 virtual int WriteSlice(const float *slice);

 //! Toggle padding of data on writes
 //!
 //! If true, incomplete data blocks will be padded prior to transformation
 //! and storage to disk. A block is incomplete iff it is a boundary block
 //! (a block that contains a volume region boundary)
 //! and the extents of the region do not coincide with block boundaries
 //!
 //! \param[in] pad Boolean indicating whether padding should (true) or should
 //! not (false) take place
 //!
 //! \sa GetBoundaryMode()
 //!
 virtual void SetBoundaryPadOnOff(bool pad) {
	_pad = pad;
 };

 //! Return the data range of the currently opened volume as a two-element array
 //!
 //! This method returns the minimum and maximum data values
 //! of the currently opened variable within the valid domain
 //! bounds. See GetValidRegion(). 
 //!
 //! \note The range values returned are valid for the native
 //! data only. Data approximations produced by level-of-detail or
 //! through multi-resolution may have values outside of this range.
 //!
 //! \retval[out] range  A two-element vector containing the current
 //! minimum and maximum.
 //!
 //
 const float *GetDataRange() const {return (_dataRange);}

 //! Return the valid region bounds for the currently opened
 //! variable
 //!
 //! This method returns the minimum and maximum valid coordinate
 //! bounds (in voxels) of the currently opened variable. In general,
 //! the minimum bounds are (0,0,0) and the maximum bounds are
 //! (nx-1, ny-1, nz-1), where nx, ny, and nz are the volume dimensions
 //! returned by GetDim(). However, partial regions (sub-volumes) may be 
 //! written to the VDC as well.
 //!
 //!
 //! \param[in] reflevel Refinement level of the variable
 //! \param[out] min Minimum coordinate bounds (in voxels) of volume
 //! \param[out] max Maximum coordinate bounds (in voxels) of volume
 //! \retval status A non-negative int is returned on success
 //!
 void    GetValidRegion(
	size_t min[3], size_t max[3], int reflevel
 ) const;

 //! Returns true if the indicated data volume exists on disk
 //!
 //! Returns true if the variable identified by the timestep, variable
 //! name, refinement level, and level-of-detail is present on disk. 
 //! Returns 0 if
 //! the variable is not present.
 //! \param[in] ts A valid time step from the Metadata object used
 //! to initialize the class
 //! \param[in] varname A valid variable name
 //! \param[in] reflevel Ignored
 //! \param[in] lod Compression level of detail requested. The coarsest 
 //! approximation level is 0 (zero). A value of -1 indicates the finest
 //! refinement level contained in the VDC.
 //
 virtual int    VariableExists(
    size_t ts,
    const char *varname,
    int reflevel = 0,
	int lod = 0
 ) const ;

 //! Return the maximimum compression ratio possible 
 //! 
 //! This static methods returns the maximum possible compression ratio
 //! possible for a given combination of blocksize, \p ps, wavele name, 
 //! \p wname, and wavelet boundary handling mode, \p wmode. 
 //!
 //! \param[in] bs A three-element vector providing the dimensions of 
 //! a block 
 //! \param[in] wavename The name of the wavelet
 //! \param[in] wmode The wavelet boundary handling mode
 //!
 //! \retval ratio A value of zero is returned if the wavename or wmode
 //! are invalid, otherwise the maximum possible data compression ratio
 //! is returned. 
 //!
 //! \sa Metadata::GetCRatios()
 //! 
 static size_t GetMaxCRatio(const size_t bs[3], string wavename, string wmode);

 //! \copydoc Metadata::GetNumTransforms()
 //
 virtual int GetNumTransforms() const;

 //! \copydoc Metadata::GetBlockSize(size_t, int)
 //
 virtual void GetBlockSize(size_t bs[3], int reflevel) const;
#ifdef PARALLEL
 void SetIOComm(MPI_Comm NewIOComm) {_IO_Comm = NewIOComm;};
#endif
 void SetCollectiveIO(bool newCollectiveIO) {
   _collectiveIO = newCollectiveIO;
 };
 friend void     *RunBlockReadRegionThread(void *object);
 friend void     *RunBlockWriteRegionThread(void *object);

private:
#ifdef PARALLEL
 MPI_Comm _IO_Comm;
#endif
 bool _collectiveIO;
 double _xformMPI;
 double _methodTimer;
 double _methodThreadTimer;
 double _ioMPI;
 //
 // Threaded read object for parallel inverse transforms 
 // (data reconstruction)
 //
 class ReadWriteThreadObj {
 public:
	ReadWriteThreadObj(
		WaveCodecIO *wc,
		int id,
		float *region,
		const size_t bmin_p[3],
		const size_t bmax_p[3],
		const size_t bdim_p[3],
		const size_t dim_p[3],
		const size_t bs_p[3],
		bool reblock,
		bool pad
	);
	void BlockReadRegionThread();
	void BlockWriteRegionThread();
	const float *GetDataRange() const {return (_dataRange);}
	
 private:
	WaveCodecIO *_wc;
	int _id;	// thread id
	float *_region;	// destination buffer for read
	const size_t *_bmin_p;
	const size_t *_bmax_p;	// block coordinates of data
	const size_t *_bdim_p;
	const size_t *_dim_p;	
	const size_t *_bs_p;	// dimensions of block
	float _dataRange[2];
	bool _reblock;
	bool _pad;
	int _FetchBlock(
		size_t bx, size_t by, size_t bz
	);
	int _WriteBlock(size_t bx, size_t by, size_t bz);
 };
 

public:
 int _nthreads; // num execution threads
 int getNumThread(){return _nthreads;}
 void EnableBuffering(size_t count[3], size_t divisor, int rank);
private:
 int _next_block;
 int _threadStatus;
 size_t _NC_BUF_SIZE; //buffering disabled by default
 ReadWriteThreadObj **_rw_thread_objs;
 vector < vector <SignificanceMap> > _sigmapsThread;// one set for each thread
 vector <size_t> _sigmapsizes;	// size of each encoded sig map
 Compressor *_compressor3D;	// 3D compressor
 Compressor *_compressor2DXY;
 Compressor *_compressor2DXZ;
 Compressor *_compressor2DYZ;
 Compressor *_compressor;	// compressor for currently opened variable
 vector <Compressor *> _compressorThread3D;
 vector <Compressor *> _compressorThread2DXY;
 vector <Compressor *> _compressorThread2DXZ;
 vector <Compressor *> _compressorThread2DYZ;
 vector <Compressor *> _compressorThread; // current compressor threads
 vector <NCBuf *> _ncbufs;

 VarType_T _vtype;  // Type (2d, or 3d) of currently opened variable
 VarType_T _compressorType;  // Type (2d, or 3d) of current _compressor
 int _lod;	// compression level of currently opened file
 int _reflevel;	// current refinement level
 size_t _validRegMin[3];	// min region bounds of current file
 size_t _validRegMax[3];	// max region bounds of current file
 bool _writeMode;	// true if opened for writes
 bool _isOpen;	// true if a file is opened
 size_t _timeStep;	// currently opened time step
 string _varName;	// Currently opened variable
 vector <string> _ncpaths; 
 vector <int> _ncids; 
 vector <int> _nc_sig_vars;	// ncdf ids for wave and sig vars 
 vector <int> _nc_wave_vars; 
 float *_cvector;	// storage for wavelet coefficients
 size_t _cvectorsize;	// amount of space allocated to _cvector 
 vector <float *> _cvectorThread;	
 unsigned char *_svector;	// storage for encoded signficance map 
 size_t _svectorsize;	// amount of space allocated to _svector 
 vector <unsigned char *> _svectorThread;
 float *_block;	// storage for a block
 vector <float *> _blockThread;
 float *_blockReg;	// more storage
 float _dataRange[2];
 vector <size_t> _ncoeffs; // num wave coeff. at each compression level
 vector <size_t> _cratios3D;	// 3D compression ratios
 vector <size_t> _cratios2D;	// 2D compression ratios
 vector <size_t> _cratios;	// compression ratios for currently opened file

 float *_sliceBuffer;
 size_t _sliceBufferSize;	// size of slice buffer in elements
 int _sliceCount;	// num slices written 

 bool _pad;	// Padding enabled?

 int _OpenVarWrite(const string &basename);
 int _OpenVarRead(const string &basename);
 int _WaveCodecIO(int nthreads);
 int _SetupCompressor();

 void _UnpackCoord(
    VarType_T vtype, const size_t src[3], size_t dst[3], size_t fill
 ) const;

 void _PackCoord(
	VarType_T vtype, const size_t src[3], size_t dst[3], size_t fill
 ) const;

 void _FillPackedCoord(
	VarType_T vtype, const size_t src[3], size_t dst[3], size_t fill
 ) const;


};
};

#endif	// _WaveCodeIO_h_
