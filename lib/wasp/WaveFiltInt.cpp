/*
 * -------------------------------------------------------------------------
 * Haar wavelets coefficents.
 * SWT - Scilab wavelet toolbox
 * Copyright (C) 2005-2006  Roger Liu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * -------------------------------------------------------------------------
 */

#include <cmath>
#include <vapor/WaveFiltBase.h>
#include <vapor/WaveFiltInt.h>


using namespace VAPoR;

/*********************************************
 * Local Variable (Filter Coefficent)
 ********************************************/
namespace {

	const double h2_2[] = {-1/8, 1/4, 3/4, 1/4, -1/8};
	const double hm2_2[] = {0, -1/2, 1, -1/2, 0};
};

void WaveFiltInt::_analysis_initialize () 
{
  const double *pFilterCoef = NULL;
  const double *pFilterCoefMirror = NULL;

  if (_wavename == "intbior2.2" || _wavename == "intcdf5/3") {
    _filterLength = 5;
    pFilterCoef = h2_2; 
    pFilterCoefMirror = hm2_2; 
  }
  else {
    _filterLength = 5;
    pFilterCoef = h2_2; 
    pFilterCoefMirror = hm2_2; 
  }

  wrev(pFilterCoef, _lowDecomFilCoef, _filterLength);
  qmf_wrev(pFilterCoefMirror, _hiDecomFilCoef, _filterLength);

}


void WaveFiltInt::_synthesis_initialize () 
{
  const double *pFilterCoef = NULL;
  const double *pFilterCoefMirror = NULL;

  if (_wavename == "intbior2.2" || _wavename == "intcdf5/3") {
    _filterLength = 5;
    pFilterCoef = hm2_2; 
    pFilterCoefMirror = h2_2; 
  }
  else {
    _filterLength = 5;
    pFilterCoef = hm2_2; 
    pFilterCoefMirror = h2_2; 
  }

  verbatim_copy(pFilterCoef, _lowReconFilCoef, _filterLength);
  qmf_even(pFilterCoefMirror, _hiReconFilCoef, _filterLength);

  return;
}

WaveFiltInt::WaveFiltInt(
	const string &wavename
) : WaveFiltBase() {

	_wavename = wavename;

	_analysis_initialize();
	_synthesis_initialize();
}

WaveFiltInt::~WaveFiltInt() {
}

void WaveFiltInt::Analysis(
    const long *sigIn, size_t sigInLen,
    long *cA, long *cD, bool oddlow, bool oddhigh
) const {
  if (_wavename == "intbior2.2" || _wavename == "intcdf5/3") {
    _AnalysisCDF5_3(sigIn, sigInLen, cA, cD);
  }
  else {
    _AnalysisCDF5_3(sigIn, sigInLen, cA, cD);
  }
}

void WaveFiltInt::_AnalysisCDF5_3(
    const long *sigIn, size_t sigInLen, long *cA, long *cD
) const {
	const long *x = sigIn + 2;	// skip signal extension

	size_t nD = sigInLen >> 1;	// num detail coefficients
	size_t nC = sigInLen >> 1;	// num approximation coefficients
	if (sigInLen % 2) nC++;

	for (size_t i=0; i<nD; i++) {
		cD[i] = x[2*i+1] - floor(0.5 * (x[2*(i+1)] + x[2*i]));
	}

	// Left boundary of approximation coefficients requires special 
	// handling (we don't have cD[i] for i==-1
	//
	long cDm1 = x[-1] - floor(0.5 * (x[0] + x[-2]));
	cA[0] = x[0] + floor(0.25 * (cD[0] + cDm1) + 0.5);

	// For even length signals nD=nC. For odd, nD=nC-1 and we need
	// special handling for right boundary
	//
	for (size_t i=1; i<nD; i++) {
		cA[i] = x[2*i] + floor(0.25 * (cD[i] + cD[i-1]) +0.5);
	}

	// Boundary handling for odd length signals
	//
	if (sigInLen % 2) {
		size_t i = nC-1;
		long cDp1  = x[2*i+1] - floor(0.5 * (x[2*(i+1)] + x[2*i]));
		cA[i] = x[2*i] + floor(0.25 * (cDp1 + cD[i-1]) +0.5);
	}
}

void WaveFiltInt::Synthesis(
    const long *cA, const long *cD, size_t sigInLen, long *sigOut
) const {
  if (_wavename == "intbior2.2" || _wavename == "intcdf5/3") {
    _SynthesisCDF5_3(cA, cD, sigInLen, sigOut);
  }
  else {
    _SynthesisCDF5_3(cA, cD, sigInLen, sigOut);
  }
}

void WaveFiltInt::_SynthesisCDF5_3(
    const long *cA, const long *cD, size_t sigInLen, long *sigOut
) const {
	cA += 1;	// skip signal extension
	cD += 1;	// skip signal extension

	size_t n = sigInLen;

	// Even samples
	//
	for (size_t i=0; i<n; i++) {
		sigOut[2*i] = cA[i] - floor(0.25 * (cD[i-1]+cD[i]) + 0.5);
	}

	// Odd  samples
	//
	for (size_t i=0; i<n-1; i++) {
		sigOut[2*i+1] = cD[i] + floor(0.5 * (sigOut[2*(i+1)]+sigOut[2*i]));
	}

	// Right boundary requires special handling - we don't have 
	// even sample for sigOut[2*(i+1)] when i==n-1
	//
	size_t i = n-1;
	long sp1 = cA[n] - floor(0.25 * (cD[n-1]+cD[n]) + 0.5);
	sigOut[2*i+1] = cD[i] + floor(0.5 * (sp1+sigOut[2*i]));
}
