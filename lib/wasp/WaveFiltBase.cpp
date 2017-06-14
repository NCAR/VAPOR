
#include <iostream>
#include <vapor/WaveFiltBase.h>

using namespace std;
using namespace VAPoR;

WaveFiltBase::WaveFiltBase() {
	_lowDecomFilCoef = new double[MAX_FILTER_SIZE];
	_lowReconFilCoef = new double[MAX_FILTER_SIZE];
	_hiDecomFilCoef = new double[MAX_FILTER_SIZE];
	_hiReconFilCoef = new double[MAX_FILTER_SIZE];
} 

WaveFiltBase::~WaveFiltBase() {
	if (_lowDecomFilCoef) delete [] _lowDecomFilCoef;
	_lowDecomFilCoef = NULL;

	if (_lowReconFilCoef) delete [] _lowReconFilCoef;
	_lowReconFilCoef = NULL;

	if (_hiDecomFilCoef) delete [] _hiDecomFilCoef;
	_hiDecomFilCoef = NULL;

	if (_hiReconFilCoef) delete [] _hiReconFilCoef;
	_hiReconFilCoef = NULL;
}


/*-------------------------------------------
 * Flipping Operation
 *-----------------------------------------*/

void
WaveFiltBase::wrev (const double *sigIn, double *sigOut, int sigLength)
const {
  int count = 0;
  for (count = 0; count < sigLength; count++)
    sigOut[count] = sigIn[sigLength - count - 1];
  return;
}


/*-------------------------------------------
 * Quadrature Mirror Filtering Operation
 *-----------------------------------------*/
void
WaveFiltBase::qmf_even (const double *sigIn, double *sigOut, int sigLength)
const {

	int count = 0;
	for (count = 0; count < sigLength; count++) {
		sigOut[count] = sigIn[sigLength - count - 1];

		if (sigLength % 2 == 0) {
			if (count % 2 != 0) {
				sigOut[count] = -1 * sigOut[count];
			}
		}
		else {
			if (count % 2 == 0) {
				sigOut[count] = -1 * sigOut[count];
			}
		}
	}
	return;
}

/*-------------------------------------------
 * Flipping and QMF at the same time
 *-----------------------------------------*/
void
WaveFiltBase::qmf_wrev (const double *sigIn, double *sigOut, int sigLength)
const {

	int count = 0;
	for (count = 0; count < sigLength; count++) {
		sigOut[count] = sigIn[sigLength - count - 1];

		if (sigLength % 2 == 0) {
			if (count % 2 != 0) {
				sigOut[count] = -1 * sigOut[count];
			}
		}
		else {
			if (count % 2 == 0) {
				sigOut[count] = -1 * sigOut[count];
			}
		}
	}

	double tmp;
	for (count = 0; count < sigLength/2; count++) {
		tmp = sigOut[count];
		sigOut[count] = sigOut[sigLength - count - 1];
		sigOut[sigLength - count - 1] = tmp;
	}
	return;
}

/*-------------------------------------------
 * Verbatim Copying
 *-----------------------------------------*/

void
WaveFiltBase::verbatim_copy (const double *sigIn, double *sigOut, int sigLength)
const {
  int count = 0;
  for (count = 0; count < sigLength; count++)
    sigOut[count] = sigIn[count];
}
