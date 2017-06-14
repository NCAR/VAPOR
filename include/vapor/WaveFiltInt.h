#include <string>
#include "WaveFiltBase.h"

using namespace std;

#ifndef	_WaveFiltInt
#define	_WaveFiltInt

namespace VAPoR {

//
//! \class WaveFiltInt
//! \brief Integer Biorthogonal spline family FIR filters
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! This class provides FIR filters for the Biorlet family of wavelets
//!
class WASP_API WaveFiltInt : public WaveFiltBase {

public:

 //! Create a set of Biorthogonal spline filters
 //!
 //! \param[in] wavename The Biorlet family wavelet member. Valid values
 //! are "bior2.2"
 //!
 WaveFiltInt(const string &wavename);
 virtual ~WaveFiltInt();

 //! Returns true if the wavelet is symmetric (or antisymmetric)
 //!
 virtual bool issymmetric() const { return(true); };

 //! Returns true if the wavelet operates only on integers and preserves
 //! integer values.
 //!
 virtual bool isint() const { return(true); };
	
 void Analysis(
	const long *sigIn, size_t sigInLen,
	long *cA, long *cD, bool oddlow, bool oddhigh
 ) const;

 void Synthesis(
	const long *cA, const long *cD, size_t sigInLen, long *sigOut
 ) const;

private:
 string _wavename;

 void _analysis_initialize ();
 void _synthesis_initialize ();

 void _AnalysisCDF5_3(
	const long *sigIn, size_t sigInLen, long *cA, long *cD
 ) const;

 void _SynthesisCDF5_3(
	const long *cA, const long *cD, size_t sigInLen, long *sigOut
 ) const;

};

}

#endif
