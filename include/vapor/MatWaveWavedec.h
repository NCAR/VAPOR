
#ifndef	_MatWaveWavedec_h_
#define	_MatWaveWavedec_h_

#include "MatWaveDwt.h"

namespace VAPoR {

//
//! \class MatWaveWavedec
//! \brief Implements a multi-level wavelet filter
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! The MatWaveWavedec class provides multi-level wavelet filters similar
//! to those provided by the Matlab wavedec and waverec functions. 
//! 1D, 2D, and 3D
//! transforms are provided.
//!
class WASP_API MatWaveWavedec : public MatWaveDwt {

public:

 //! Create a wavelet filter bank
 //!
 //! \param[in] wname The name of the wavelet to apply.
 //! \param[in] mode The boundary extension mode.
 //!
 //! \note To ensure that the number of coefficients in a decomposition
 //! is equal to the number of samples in the decomposed signal, \p mode
 //! must be set to 'per'; or a symmetric wavelet must be used and \p mode
 //! set to 'symw' if the filter length is odd, or set to 'symh' if 
 //! the filter length is even. The one exception to this is the Haar
 //! wavelet for which the number of coefficients is the same as the
 //! signal length regardless of the boundary handling mode. 
 //!
 //! \sa dwtmode()
 //!
 MatWaveWavedec(const string &wname, const string &mode);
 MatWaveWavedec(const string &wname);
 virtual ~MatWaveWavedec();

 //! Multi-level discrete 1D wavelet decomposition
 //!
 //! This method performs a multi-level, one-dimensional wavelet
 //! decomposition with respect to the current wavelet.
 //! The number of decompositions to apply is specified by the
 //! parameter \p n, where \p n is in the range (0..max). \b max is the 
 //! value returned by wmaxlev(). The format of the returned 
 //! decomposition vector, \p C, and the bookkeeping vector, \p L, are
 //! as described by the Matlab documentation for the \b wavedec function.
 //!
 //! \param[in] sigIn The discrete signal
 //! \param[in] sigInLength The length of \p sigIn
 //! \param[in] n The transformation level
 //! \param[out] C The wavelet decompostion vector.  The length of \p C 
 //! must be equal to
 //! the value returned by MatWaveWavedec::coefflength().
 //! \param[out] L The booking vector.  The length of \p L must be equal to
 //! \p n + 2.
 //!
 //! \retval status A negative number indicates failure.
 //!
 //! \sa MatWaveWavedec::coefflength(), waverec(), wmaxlev()
 //
 int wavedec(
	const double *sigIn, size_t sigInLength, int n, double *C, size_t *L
 );
 int wavedec(
	const float *sigIn, size_t sigInLength, int n, float *C, size_t *L
 );
 int wavedec(
	const long *sigIn, size_t sigInLength, int n, long *C, size_t *L
 );
 int wavedec(
	const int *sigIn, size_t sigInLength, int n, int *C, size_t *L
 );

 //! Multi-level discrete 1D wavelet reconstruction
 //!
 //! This method performs a multi-level, one-dimensional wavelet
 //! reconstruction with respect to the current wavelet.
 //! The number of reconstructions to apply is \p n.
 //!
 //! \param[in] C The Wavelet decomposition vector
 //! \param[in] L The Wavelet decomposition bookkeping vector. The length 
 //! of \p L must be equal to \p n + 2.
 //! \param[in] n The transformation level
 //! \param[out] sigOut The reconstructed signal.  The length of 
 //! \p sigOut is given by MatWaveWavedec::approxlength().
 //!
 //! \retval status A negative number indicates failure.
 //!
 //! \sa wavedec()
 //
 int waverec(const double *C, const size_t *L, int n, double *sigOut);
 int waverec(const float *C, const size_t *L, int n, float *sigOut);
 int waverec(const long *C, const size_t *L, int n, long *sigOut);
 int waverec(const int *C, const size_t *L, int n, int *sigOut);

 //! Multi-level discrete 2D wavelet decomposition
 //!
 //! This method performs a multi-level, two-dimensional wavelet
 //! decomposition with respect to the current wavelet.
 //! The number of decompositions to apply is specified by the
 //! parameter \p n, where \p n is in the range (0..max). \b max is the 
 //! value returned by wmaxlev() for the smallest input dimension (\p sigInX
 //! and \p sigOutY). The format of the returned 
 //! decomposition vector, \p C, and the bookkeeping vector, \p L, are
 //! as described by the Matlab documentation for the \b wavedec2 function.
 //!
 //! \param[in] sigIn The discrete signal
 //! \param[in] sigInX The length of the X dimension of \p sigIn
 //! \param[in] sigInY The length of the Y dimension of \p sigIn
 //! \param[in] n The transformation level
 //! \param[out] C The wavelet decompostion vector.  The length of \p C must 
 //! be equal to the value returned by MatWaveWavedec::coefflength2().
 //! \param[out] L The booking vector.  The length of \p L must be equal to
 //! (\p n * 6) + 4.
 //!
 //! \retval status A negative number indicates failure.
 //!
 //! \sa MatWaveWavedec::coefflength2(), waverec2(), wmaxlev()
 //
 int wavedec2(
	const double *sigIn, size_t sigInX, size_t sigInY, int n, 
	double *C, size_t *L
 );
 int wavedec2(
	const float *sigIn, size_t sigInX, size_t sigInY, int n, 
	float *C, size_t *L
 );
 int wavedec2(
	const long *sigIn, size_t sigInX, size_t sigInY, int n, 
	long *C, size_t *L
 );
 int wavedec2(
	const int *sigIn, size_t sigInX, size_t sigInY, int n, 
	int *C, size_t *L
 );

 //! Multi-level discrete 2D wavelet reconstruction
 //!
 //! This method performs a multi-level, two-dimensional wavelet
 //! reconstruction with respect to the current wavelet.
 //! The number of reconstructions to apply is \p n.
 //!
 //! \param[in] C The Wavelet decomposition vector
 //! \param[in] L The Wavelet decomposition bookkeping vector. The length 
 //! of \p L must be equal to \p n * 6 + 4.
 //! \param[in] n The transformation level
 //! \param[out] sigOut The reconstructed signal.  The dimensions of 
 //! \p sigOut are given by MatWaveWavedec::approxlength2().
 //!
 //! \retval status A negative number indicates failure.
 //!
 //! \sa wavedec2()
 //
 int waverec2(const double *C, const size_t *L, int n, double *sigOut);
 int waverec2(const float *C, const size_t *L, int n, float *sigOut);
 int waverec2(const long *C, const size_t *L, int n, long *sigOut);
 int waverec2(const int *C, const size_t *L, int n, int *sigOut);



 //! Multi-level discrete 3D wavelet decomposition
 //!
 //! This method performs a multi-level, three-dimensional wavelet
 //! decomposition with respect to the current wavelet.
 //! The number of decompositions to apply is specified by the
 //! parameter \p n, where \p n is in the range (0..max). \b max is the 
 //! value returned by wmaxlev() for the smallest input dimension (\p sigInX
 //! \p sigOutY, and \p sigOutZ). The format of the returned 
 //! decomposition vector, \p C, and the bookkeeping vector, \p L, follows
 //! the pattern of the 1D and 2D transforms
 //!
 //! \param[in] sigIn The discrete signal
 //! \param[in] sigInX The length of the X dimension of \p sigIn
 //! \param[in] sigInY The length of the Y dimension of \p sigIn
 //! \param[in] sigInZ The length of the Z dimension of \p sigIn
 //! \param[in] n The transformation level
 //! \param[out] C The wavelet decompostion vector.  The length of \p C must 
 //! be equal to
 //! the value returned by MatWaveWavedec::coefflength3().
 //! \param[out] L The booking vector.  The length of \p L which must 
 //! be equal to
 //! (\p n * 21) + 6.
 //!
 //! \retval status A negative number indicates failure.
 //!
 //! \sa MatWaveWavedec::coefflength3(), waverec3(), wmaxlev()
 //
 int wavedec3(
	const double *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, 
	double *C, size_t *L
 ); 
 int wavedec3(
	const float *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, 
	float *C, size_t *L
 ); 
 int wavedec3(
	const long *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, 
	long *C, size_t *L
 ); 
 int wavedec3(
	const int *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, 
	int *C, size_t *L
 ); 

 //! Multi-level discrete 3D wavelet reconstruction
 //!
 //! This method performs a multi-level, three-dimensional wavelet
 //! reconstruction with respect to the current wavelet.
 //! The number of reconstructions to apply is \p n.
 //!
 //! \param[in] C The Wavelet decomposition vector
 //! \param[in] L The Wavelet decomposition bookkeping vector. The length 
 //! of \p L must be equal to (\p n * 21) + 6.
 //! \param[in] n The transformation level
 //! \param[out] sigOut The reconstructed signal.  The dimensions of 
 //! \p sigOut are given by MatWaveWavedec::approxlength3().
 //!
 //! \retval status A negative number indicates failure.
 //!
 //! \sa wavedec3()
 //
 int waverec3(const double *C, const size_t *L, int n, double *sigOut); 
 int waverec3(const float *C, const size_t *L, int n, float *sigOut); 
 int waverec3(const long *C, const size_t *L, int n, long *sigOut); 
 int waverec3(const int *C, const size_t *L, int n, int *sigOut); 

 int appcoef(
    const double *C, const size_t *L, int n, int l, bool normal, double *sigOut
 ); 
 int appcoef(
    const float *C, const size_t *L, int n, int l, bool normal, float *sigOut
 ); 
 int appcoef2(
    const double *C, const size_t *L, int n, int l, bool normal, double *sigOut
 ); 
 int appcoef2(
    const float *C, const size_t *L, int n, int l, bool normal, float *sigOut
 ); 
 int appcoef3(
    const double *C, const size_t *L, int n, int l, bool normal, double *sigOut
 ); 
 int appcoef3(
    const float *C, const size_t *L, int n, int l, bool normal, float *sigOut
 ); 
 int appcoef(
    const long *C, const size_t *L, int n, int l, bool normal, long *sigOut
 ); 
 int appcoef(
    const int *C, const size_t *L, int n, int l, bool normal, int *sigOut
 ); 
 int appcoef2(
    const long *C, const size_t *L, int n, int l, bool normal, long *sigOut
 ); 
 int appcoef2(
    const int *C, const size_t *L, int n, int l, bool normal, int *sigOut
 ); 
 int appcoef3(
    const long *C, const size_t *L, int n, int l, bool normal, long *sigOut
 ); 
 int appcoef3(
    const int *C, const size_t *L, int n, int l, bool normal, int *sigOut
 ); 

 // 
 //! Returns length of coefficient vector generated in a 
 //! multi-level 1D decompostition pass
 //!
 //! This method returns the number of coefficients (approximation plus detail)
 //! generated by a multi-level, one-dimensional
 //!  decomposition pass through the filter 
 //! bank for a signal
 //! of length, \p sigInLen, using the current wavelet.
 //!
 //! \param[in] sigInLen Length of input signal (number of samples)
 //! \param[in] L The Wavelet decomposition bookkeping vector. The length 
 //! of \p L must be equal to (\p n + 2).
 //! \param[in] n The transformation level
 //! \retval length returns the number of coefficients. 
 //!
 //! \sa MatWaveDwt::wavdec()
 //
 size_t coefflength(size_t sigInLen, int n) const;
 size_t coefflength(const size_t *L, int n) const;

 //
 //! Returns the number of approximation coefficients in a
 //! reconstruction.
 //!
 //! This method returns the number of coefficients in the reconstruction
 //! of a signal of length \p sigInLen at level \p n. If \p n == 0, the 
 //! return value equals \p sigInLen
 //!
 //! \param[in] sigInLen Length of input signal (number of samples)
 //! \param[in] n The transformation level
 //! \retval length returns the number of coefficients
 //
 size_t approxlength(size_t sigInLen, int n) const;

 //
 //! Returns the number of approximation coefficients in a
 //! reconstruction.
 //!
 //! This method returns the number of coefficients in the reconstruction
 //! of a signal whose decompostion is described by the book keeping 
 //! vector, \p L. The total number of transformation levels in \p L
 //! is given by \p n. The approximation level is given by \p l. 
 //! If \p l == 0, the number of coefficients equals the length of
 //! the orginal signal.
 //!
 //! \param[in] L The Wavelet decomposition bookkeping vector. The length 
 //! of \p L must be equal to (\p n + 2).
 //! \param[in] n The transformation level
 //! \param[in] l The approximation level sought. \p l must be in the 
 //! range (0..\p n ).
 //! \param[out] len The returned number of approximation coefficients
 //
 void approxlength(const size_t *L, int n, int l, size_t *len) const; 


 // 
 //! Returns length of coefficient vector generated in a 
 //! multi-level 2D decompostition pass
 //!
 //! This method returns the number of coefficients (approximation plus detail)
 //! generated by a multi-level, two-dimensional
 //!  decomposition pass through the filter 
 //! bank for a signal
 //! of length, \p sigInLen, using the current wavelet.
 //!
 //! \param[in] sigInX Length X dimension of input signal (number of samples)
 //! \param[in] sigInY Length Y dimension of input signal (number of samples)
 //! \param[in] L The booking vector.  The length of \p L must be equal to
 //! (\p n * 6) + 4.
 //! \param[in] n The transformation level
 //! \retval length returns the number of coefficients. 
 //!
 //! \sa MatWaveDwt::wavdec2()
 //
 size_t coefflength2(size_t sigInX, size_t sigInY, int n) const;
 size_t coefflength2(const size_t *L, int n) const;

 //
 //! Returns the number of approximation coefficients in a
 //! reconstruction.
 //!
 //! This method returns the number of coefficients in the reconstruction
 //! of a 2D signal whose decompostion is described by the book keeping 
 //! vector, \p L. The total number of transformation levels in \p L
 //! is given by \p n. The approximation level is given by \p l. 
 //! If \p l == 0, the number of coefficients equals the length of
 //! the orginal signal.
 //!
 //! \param[in] L The Wavelet decomposition bookkeping vector. The length 
 //! of \p L must be equal to (\p n * 6) + 4.
 //! \param[in] n The transformation level
 //! \param[in] l The approximation level sought. \p l must be in the 
 //! range (0..\p n ).
 //! \param[out] lenx The returned X dimension of approximation coefficients
 //! \param[out] leny The returned Y dimension of approximation coefficients
 //
 void approxlength2(const size_t *L, int n, int l, size_t *lenx, size_t *leny) const; 


 // 
 //! Returns length of coefficient vector generated in a 
 //! multi-level 3D decompostition pass
 //!
 //! This method returns the number of coefficients (approximation plus detail)
 //! generated by a multi-level, three-dimensional
 //!  decomposition pass through the filter 
 //! bank for a signal
 //! of length, \p sigInLen, using the current wavelet.
 //!
 //! \param[in] sigInX Length X dimension of input signal (number of samples)
 //! \param[in] sigInY Length Y dimension of input signal (number of samples)
 //! \param[in] sigInZ Length Z dimension of input signal (number of samples)
 //! \param[in] L The booking vector.  The length of \p L must be equal to
 //! (\p n * 21) + 6.
 //! \param[in] n The transformation level
 //! \retval length returns the number of coefficients. 
 //!
 //! \sa MatWaveDwt::wavdec3()
 size_t coefflength3(size_t sigInX, size_t sigInY, size_t sigInZ, int n) const;
 size_t coefflength3(const size_t *L, int n) const;


 //! Returns the number of approximation coefficients in a
 //! reconstruction.
 //!
 //! This method returns the number of coefficients in the reconstruction
 //! of a 3D signal whose decompostion is described by the book keeping 
 //! vector, \p L. The total number of transformation levels in \p L
 //! is given by \p n. The approximation level is given by \p l. 
 //! If \p l == 0, the number of coefficients equals the length of
 //! the orginal signal.
 //!
 //! \param[in] L The Wavelet decomposition bookkeping vector. The length 
 //! of \p L must be equal to (\p n * 6) + 4.
 //! \param[in] n The transformation level
 //! \param[in] l The approximation level sought. \p l must be in the 
 //! range (0..\p n ).
 //! \param[out] lenx The returned X dimension of approximation coefficients
 //! \param[out] leny The returned Y dimension of approximation coefficients
 //! \param[out] lenz The returned Z dimension of approximation coefficients
 //
 void approxlength3(
	const size_t *L, int n, int l, size_t *lenx, size_t *leny, size_t *lenz
 ) const; 


 //
 //! Computes the book keeping vector, L, for a 1D wavelet decomposition
 //!
 //!
 //! \param[in] sigInLength The length of the input signal
 //! \param[in] n The transformation level
 //! \param[out] L The booking vector.  The length of \p L must be equal to
 //! \p n + 2.
 //
 void computeL(size_t sigInLen, int n, size_t *L) const;

 //
 //! Computes the book keeping vector, L, for a 2D wavelet decomposition
 //!
 //!
 //! \param[in] sigInX The length of the X dimension of input signal
 //! \param[in] sigInY The length of the Y dimension of input signal
 //! \param[in] n The transformation level
 //! \param[out] L The booking vector.  The length of \p L must be equal to
 //! (\p n * 6) + 4.
 //
 void computeL2(size_t sigInX, size_t sigInY, int n, size_t *L) const;

 //
 //! Computes the book keeping vector, L, for a 3D wavelet decomposition
 //!
 //!
 //! \param[in] sigInX The length of the X dimension of input signal
 //! \param[in] sigInY The length of the Y dimension of input signal
 //! \param[in] sigInZ The length of the Z dimension of input signal
 //! \param[in] n The transformation level
 //! \param[out] L The booking vector.  The length of \p L must be equal to
 //! (\p n * 21) + 6.
 //
 void computeL3(
	size_t sigInX, size_t sigInY, size_t sigInZ, int n, size_t *L
 ) const;



private:

};

}

#endif


