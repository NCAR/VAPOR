#ifndef	_WaveFiltBase_h_
#define	_WaveFiltBase_h_

#include <vapor/MyBase.h>

namespace VAPoR {

//
//! \class WaveFiltBase
//! \brief A base class for wavelet family filters
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! The WaveFiltBase class is a base class for building classes of 
//! wavelet families that can be implemented as FIR filters. A wavelet 
//! family class contains scaling and 
//! wavelet coefficients that define a particular wavelet. All 
//! filters are normalized.
//!
class WASP_API WaveFiltBase {

public:
	WaveFiltBase();
	virtual ~WaveFiltBase();

	//! Return the number of coefficients in both the scaling and
	//! wavelet FIR filter.
	//!
	//! This method returns the number of elements in the arrays
	//! returned by the classes filter retrieval methods
	//!
	//! \retval filter length
	//!
	//! \sa GetLowDecomFilCoef(), GetLowReconFilCoef(), GetHighDecomFilCoef()
	//! GetHighReconFilCoef()
	//!
	int GetLength() const { return(_filterLength); };

	//! Return scaling (low pass) decompostion filter coefficients
	const double *GetLowDecomFilCoef() const {return (_lowDecomFilCoef); };

	//! Return scaling (low pass) reconstruction filter coefficients
	const double *GetLowReconFilCoef() const {return (_lowReconFilCoef); };

	//! Return wavelet (high pass) decompostion filter coefficients
	const double *GetHighDecomFilCoef() const {return (_hiDecomFilCoef); };

	//! Return wavelet (high pass) decompostion filter coefficients
	const double *GetHighReconFilCoef() const {return (_hiReconFilCoef); };
	
	//! Returns true if the wavelet is symmetric (or antisymmetric)
	//!
	virtual bool issymmetric() const { return(false); };

	//! Returns true if the wavelet is an integer transform
	//!
	virtual bool isint() const { return(false); };

protected:
	static const int MAX_FILTER_SIZE = 32;	// space allocated to filters
	int _filterLength;	// length of filters
	double *_lowDecomFilCoef;
	double *_lowReconFilCoef;
	double *_hiDecomFilCoef;
	double *_hiReconFilCoef;

	/*-------------------------------------------
	 * Flipping Operation
	 *-----------------------------------------*/
	void wrev (
		const double *sigIn, double *sigOut, int sigLength
	) const;


	/*-------------------------------------------
	 * Quadrature Mirror Filtering Operation
	 *-----------------------------------------*/
	void qmf_even (
		const double *sigIn, double *sigOut, int sigLength
	) const;

	/*-------------------------------------------
	 * Flipping and QMF at the same time
	 *-----------------------------------------*/
	void qmf_wrev (
		const double *sigIn, double *sigOut, int sigLength
	) const;

	/*-------------------------------------------
	 * Verbatim Copying
	 *-----------------------------------------*/
	void verbatim_copy (
		const double *sigIn, double *sigOut, int sigLength
	) const;

};

}

#endif
