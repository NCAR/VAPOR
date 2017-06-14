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
#include <vapor/WaveFiltHaar.h>

using namespace VAPoR;

/*********************************************
 * Local Variable (Filter Coefficent)
 ********************************************/
namespace {

//const double haar[2] = { 0.707106781186548, 0.707106781186548 };
const double haar[2] = {sqrt(2.0) / 2.0, sqrt(2.0) / 2.0};

}; // namespace

void WaveFiltHaar::_analysis_initialize() {
    const double *pFilterCoef;

    pFilterCoef = haar;
    _filterLength = 2;

    wrev(pFilterCoef, _lowDecomFilCoef, _filterLength);
    qmf_wrev(pFilterCoef, _hiDecomFilCoef, _filterLength);
    return;
}

void WaveFiltHaar::_synthesis_initialize() {
    const double *pFilterCoef;

    pFilterCoef = haar;

    _filterLength = 2;

    verbatim_copy(pFilterCoef, _lowReconFilCoef, _filterLength);
    qmf_even(pFilterCoef, _hiReconFilCoef, _filterLength);
    return;
}

WaveFiltHaar::WaveFiltHaar() : WaveFiltBase() {

    _analysis_initialize();
    _synthesis_initialize();
}

WaveFiltHaar::~WaveFiltHaar() {
}
