/*
 * -------------------------------------------------------------------------
 * Coiflet wavelets coefficents.
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
#include <cassert>
#include <vapor/WaveFiltCoif.h>

using namespace VAPoR;

/*********************************************
 * Local Variable (Filter Coefficent)
 ********************************************/
namespace {

const double coif1[6] = {-0.051429728471000, 0.238929728471000, 0.602859456942000, 0.272140543058000, -0.051429728471000, -0.011070271529000};

const double coif2[12] = {0.011587596739000,  -0.029320137980000, -0.047639590310000, 0.273021046535000, 0.574682393857000,  0.294867193696000,
                          -0.054085607092000, -0.042026480461000, 0.016744410163000,  0.003967883613000, -0.001289203356000, -0.000509505399000};

const double coif3[18] = {
    -0.002682418671000, 0.005503126709000, 0.016583560479000, -0.046507764479000, -0.043220763560000, 0.286503335274000, 0.561285256870000, 0.302983571773000,  -0.050770140755000,
    -0.058196250762000, 0.024434094321000, 0.011229240962000, -0.006369601011000, -0.001820458916000, 0.000790205101000, 0.000329665174000, -0.000050192775000, -0.000024465734000,
};

const double coif4[24] = {0.000630961046000, -0.001152224852000, -0.005194524026000, 0.011362459244000,  0.018867235378000, -0.057464234429000, -0.039652648517000, 0.293667390895000,
                          0.553126452562000, 0.307157326198000,  -0.047112738865000, -0.068038127051000, 0.027813640153000, 0.017735837438000,  -0.010756318517000, -0.004001012886000,
                          0.002652665946000, 0.000895594529000,  -0.000416500571000, -0.000183829769000, 0.000044080354000, 0.000022082857000,  -0.000002304942000, -0.000001262175000};

const double coif5[30] = {-0.000149963800000, 0.000253561200000,  0.001540245700000, -0.002941110800000, -0.007163781900000, 0.016552066400000,  0.019917804300000, -0.064997262800000,
                          -0.036800073600000, 0.298092323500000,  0.547505429400000, 0.309706849000000,  -0.043866050800000, -0.074652238900000, 0.029195879500000, 0.023110777000000,
                          -0.013973687900000, -0.006480090000000, 0.004783001400000, 0.001720654700000,  -0.001175822200000, -0.000451227000000, 0.000213729800000, 0.000099377600000,
                          -0.000029232100000, -0.000015072000000, 0.000002640800000, 0.000001459300000,  -0.000000118400000, -0.000000067300000};

};    // namespace

void WaveFiltCoif::_analysis_initialize(int member)
{
    const double *pFilterCoef = NULL;

    _filterLength = 6 * member;

    switch (member) {
    case 1: pFilterCoef = coif1; break;
    case 2: pFilterCoef = coif2; break;
    case 3: pFilterCoef = coif3; break;
    case 4: pFilterCoef = coif4; break;
    case 5: pFilterCoef = coif5; break;
    default: assert(pFilterCoef != NULL);
    }

    wrev(pFilterCoef, _lowDecomFilCoef, _filterLength);
    qmf_wrev(pFilterCoef, _hiDecomFilCoef, _filterLength);
    for (int count = 0; count < _filterLength; count++) _lowDecomFilCoef[count] *= sqrt(2.0);
    for (int count = 0; count < _filterLength; count++) _hiDecomFilCoef[count] *= sqrt(2.0);

    return;
}

void WaveFiltCoif::_synthesis_initialize(int member)
{
    const double *pFilterCoef = NULL;

    _filterLength = 6 * member;

    switch (member) {
    case 1: pFilterCoef = coif1; break;
    case 2: pFilterCoef = coif2; break;
    case 3: pFilterCoef = coif3; break;
    case 4: pFilterCoef = coif4; break;
    case 5: pFilterCoef = coif5; break;
    default: assert(pFilterCoef != NULL);
    }

    verbatim_copy(pFilterCoef, _lowReconFilCoef, _filterLength);
    qmf_even(pFilterCoef, _hiReconFilCoef, _filterLength);

    for (int count = 0; count < _filterLength; count++) _lowReconFilCoef[count] *= sqrt(2.0);
    for (int count = 0; count < _filterLength; count++) _hiReconFilCoef[count] *= sqrt(2.0);

    return;
}

WaveFiltCoif::WaveFiltCoif(const string &wavename) : WaveFiltBase()
{
    int member = -1;

    if (wavename.compare("coif1") == 0) {
        member = 1;
    } else if (wavename.compare("coif2") == 0) {
        member = 2;
    } else if (wavename.compare("coif3") == 0) {
        member = 3;
    } else if (wavename.compare("coif4") == 0) {
        member = 4;
    } else if (wavename.compare("coif5") == 0) {
        member = 5;
    } else {
        member = 1;    // default coif1
    }

    _analysis_initialize(member);
    _synthesis_initialize(member);
}

WaveFiltCoif::~WaveFiltCoif() {}
