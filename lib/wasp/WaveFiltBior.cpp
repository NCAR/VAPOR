/*
 * -------------------------------------------------------------------------
 * Biorlet wavelets coefficents.
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
#include <vapor/WaveFiltBior.h>

using namespace VAPoR;

/*********************************************
 * Local Variable (Filter Coefficent)
 ********************************************/
namespace {

const double h1[10] = {0.0, 0.0, 0.0, 0.0, 0.70710678118654752440084436210, 0.70710678118654752440084436210, 0.0, 0.0, 0.0, 0.0};

const double hm1_11[2] = {0.70710678118654752440084436210, 0.70710678118654752440084436210};

const double hm1_13[6] = {-0.0883883476483184405501055452631, 0.0883883476483184405501055452631, 0.70710678118654752440084436210,
                          0.70710678118654752440084436210,    0.0883883476483184405501055452631, -0.0883883476483184405501055452631};

const double hm1_15[10] = {0.0165728151840597076031447897368,  -0.0165728151840597076031447897368, -0.1215339780164378557563951247368, 0.1215339780164378557563951247368,
                           0.70710678118654752440084436210,    0.70710678118654752440084436210,    0.1215339780164378557563951247368,  -0.1215339780164378557563951247368,
                           -0.0165728151840597076031447897368, 0.0165728151840597076031447897368};

const double h2[18] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.3535533905932737622004221810524, 0.7071067811865475244008443621048, 0.3535533905932737622004221810524, 0.0,
                       0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

const double hm2_22[6] = {-0.1767766952966368811002110905262, 0.3535533905932737622004221810524, 1.0606601717798212866012665431573, 0.3535533905932737622004221810524,
                          -0.1767766952966368811002110905262};

const double hm2_24[10] = {0.0331456303681194152062895794737,  -0.0662912607362388304125791589473, -0.1767766952966368811002110905262,
                           0.4198446513295125926130013399998,  0.9943689110435824561886873842099,  0.4198446513295125926130013399998,
                           -0.1767766952966368811002110905262, -0.0662912607362388304125791589473, 0.0331456303681194152062895794737};

const double hm2_26[14] = {-0.0069053396600248781679769957237, 0.0138106793200497563359539914474,  0.0469563096881691715422435709210, -0.1077232986963880994204411332894,
                           -0.1698713556366120029322340948025, 0.4474660099696121052849093228945,  0.9667475524034829435167794013152, 0.4474660099696121052849093228945,
                           -0.1698713556366120029322340948025, -0.1077232986963880994204411332894, 0.0469563096881691715422435709210, 0.0138106793200497563359539914474,
                           -0.0069053396600248781679769957237};

const double hm2_28[18] = {0.0015105430506304420992449678146, -0.0030210861012608841984899356291, -0.0129475118625466465649568669819, 0.0289161098263541773284036695929,
                           0.0529984818906909399392234421792, -0.1349130736077360572068505539514, -0.1638291834340902345352542235443, 0.4625714404759165262773590010400,
                           0.9516421218971785225243297231697, 0.4625714404759165262773590010400,  -0.1638291834340902345352542235443, -0.1349130736077360572068505539514,
                           0.0529984818906909399392234421792, 0.0289161098263541773284036695929,  -0.0129475118625466465649568669819, -0.0030210861012608841984899356291,
                           0.0015105430506304420992449678146};

const double h3[20] = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1767766952966368811002110905262, 0.5303300858899106433006332715786, 0.5303300858899106433006332715786, 0.1767766952966368811002110905262,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

const double hm3_31[4] = {-0.3535533905932737622004221810524, 1.0606601717798212866012665431573, 1.0606601717798212866012665431573, -0.3535533905932737622004221810524};

const double hm3_33[8] = {0.0662912607362388304125791589473, -0.1988737822087164912377374768420, -0.1546796083845572709626847042104, 0.9943689110435824561886873842099,
                          0.9943689110435824561886873842099, -0.1546796083845572709626847042104, -0.1988737822087164912377374768420, 0.0662912607362388304125791589473};

const double hm3_35[12] = {-0.0138106793200497563359539914474, 0.0414320379601492690078619743421, 0.0524805814161890740766251675000, -0.2679271788089652729175074340788,
                           -0.0718155324642587329469607555263, 0.9667475524034829435167794013152, 0.9667475524034829435167794013152, -0.0718155324642587329469607555263,
                           -0.2679271788089652729175074340788, 0.0524805814161890740766251675000, 0.0414320379601492690078619743421, -0.0138106793200497563359539914474};

const double hm3_37[16] = {0.0030210861012608841984899356291, -0.0090632583037826525954698068873, -0.0168317654213106405344439270765, 0.0746639850740189951912512662623,
                           0.0313329787073628846871956180962, -0.3011591259228349991008967259990, -0.0264992409453454699696117210896, 0.9516421218971785225243297231697,
                           0.9516421218971785225243297231697, -0.0264992409453454699696117210896, -0.3011591259228349991008967259990, 0.0313329787073628846871956180962,
                           0.0746639850740189951912512662623, -0.0168317654213106405344439270765, -0.0090632583037826525954698068873, 0.0030210861012608841984899356291};

const double hm3_39[20] = {-0.0006797443727836989446602355165, 0.0020392331183510968339807065496, 0.0050603192196119810324706421788, -0.0206189126411055346546938106687,
                           -0.0141127879301758447558029850103, 0.0991347824942321571990197448581, 0.0123001362694193142367090236328, -0.3201919683607785695513833204624,
                           0.0020500227115698857061181706055,  0.9421257006782067372990864259380, 0.9421257006782067372990864259380, 0.0020500227115698857061181706055,
                           -0.3201919683607785695513833204624, 0.0123001362694193142367090236328, 0.0991347824942321571990197448581, -0.0141127879301758447558029850103,
                           -0.0206189126411055346546938106687, 0.0050603192196119810324706421788, 0.0020392331183510968339807065496, -0.0006797443727836989446602355165};

const double hm4_44[] = {0.037828455507264, -0.023849465019557, -0.110624404418437, 0.377402855612831, 0.852698679008894, 0.377402855612831, -0.110624404418437, -0.023849465019557, 0.037828455507264};

const double h4[] = {0.0, -0.064538882628697, -0.040689417609164, 0.418092273221617, 0.788485616405583, 0.418092273221617, -0.0406894176091641, -0.0645388826286971, 0.0};

};    // namespace

void WaveFiltBior::_analysis_initialize(int member)
{
    const double *pFilterCoef = NULL;
    const double *pFilterCoefMirror = NULL;

    _filterLength = 6 * member;

    switch (member) {
    case 11:
        _filterLength = 2;
        pFilterCoef = hm1_11;
        pFilterCoefMirror = h1 + 4;
        break;
    case 13:
        _filterLength = 6;
        pFilterCoef = hm1_13;
        pFilterCoefMirror = h1 + 2;
        break;
    case 15:
        _filterLength = 10;
        pFilterCoef = hm1_15;
        pFilterCoefMirror = h1;
        break;
    case 22:
        _filterLength = 5;
        pFilterCoef = hm2_22;
        pFilterCoefMirror = h2 + 6;
        break;
    case 24:
        _filterLength = 9;
        pFilterCoef = hm2_24;
        pFilterCoefMirror = h2 + 4;
        break;
    case 26:
        _filterLength = 13;
        pFilterCoef = hm2_26;
        pFilterCoefMirror = h2 + 2;
        break;
    case 28:
        _filterLength = 17;
        pFilterCoef = hm2_28;
        pFilterCoefMirror = h2;
        break;
    case 31:
        _filterLength = 4;
        pFilterCoef = hm3_31;
        pFilterCoefMirror = h3 + 8;
        break;
    case 33:
        _filterLength = 8;
        pFilterCoef = hm3_33;
        pFilterCoefMirror = h3 + 6;
        break;
    case 35:
        _filterLength = 12;
        pFilterCoef = hm3_35;
        pFilterCoefMirror = h3 + 4;
        break;
    case 37:
        _filterLength = 16;
        pFilterCoef = hm3_37;
        pFilterCoefMirror = h3 + 2;
        break;
    case 39:
        _filterLength = 20;
        pFilterCoef = hm3_39;
        pFilterCoefMirror = h3;
        break;

    case 44:
        _filterLength = 9;
        pFilterCoef = hm4_44;
        pFilterCoefMirror = h4;
        break;

    default: assert(pFilterCoef != NULL);
    };

    wrev(pFilterCoef, _lowDecomFilCoef, _filterLength);
    qmf_wrev(pFilterCoefMirror, _hiDecomFilCoef, _filterLength);

    return;
}

void WaveFiltBior::_synthesis_initialize(int member)
{
    const double *pFilterCoef = NULL;
    const double *pFilterCoefMirror = NULL;

    switch (member) {
    case 11:
        _filterLength = 2;
        pFilterCoef = h1 + 4;
        pFilterCoefMirror = hm1_11;
        break;
    case 13:
        _filterLength = 6;
        pFilterCoef = h1 + 2;
        pFilterCoefMirror = hm1_13;
        break;
    case 15:
        _filterLength = 10;
        pFilterCoef = h1;
        pFilterCoefMirror = hm1_15;
        break;
    case 22:
        _filterLength = 5;
        pFilterCoef = h2 + 6;
        pFilterCoefMirror = hm2_22;
        break;
    case 24:
        _filterLength = 9;
        pFilterCoef = h2 + 4;
        pFilterCoefMirror = hm2_24;
        break;
    case 26:
        _filterLength = 13;
        pFilterCoef = h2 + 2;
        pFilterCoefMirror = hm2_26;
        break;
    case 28:
        _filterLength = 17;
        pFilterCoef = h2;
        pFilterCoefMirror = hm2_28;
        break;
    case 31:
        _filterLength = 4;
        pFilterCoef = h3 + 8;
        pFilterCoefMirror = hm3_31;
        break;
    case 33:
        _filterLength = 8;
        pFilterCoef = h3 + 6;
        pFilterCoefMirror = hm3_33;
        break;
    case 35:
        _filterLength = 12;
        pFilterCoef = h3 + 4;
        pFilterCoefMirror = hm3_35;
        break;
    case 37:
        _filterLength = 16;
        pFilterCoef = h3 + 2;
        pFilterCoefMirror = hm3_37;
        break;
    case 39:
        _filterLength = 20;
        pFilterCoef = h3;
        pFilterCoefMirror = hm3_39;
        break;

    case 44:
        _filterLength = 9;
        pFilterCoef = h4;
        pFilterCoefMirror = hm4_44;
        break;

    default: assert(pFilterCoef != NULL);
    };

    verbatim_copy(pFilterCoef, _lowReconFilCoef, _filterLength);
    qmf_even(pFilterCoefMirror, _hiReconFilCoef, _filterLength);
}

WaveFiltBior::WaveFiltBior(const string &wavename) : WaveFiltBase()
{
    int member = -1;

    if (wavename.compare("bior1.1") == 0) {
        member = 11;
    } else if (wavename.compare("bior1.3") == 0) {
        member = 13;
    } else if (wavename.compare("bior1.5") == 0) {
        member = 15;
    } else if (wavename.compare("bior2.2") == 0) {
        member = 22;
    } else if (wavename.compare("bior2.4") == 0) {
        member = 24;
    } else if (wavename.compare("bior2.6") == 0) {
        member = 26;
    } else if (wavename.compare("bior2.8") == 0) {
        member = 28;
    } else if (wavename.compare("bior3.1") == 0) {
        member = 31;
    } else if (wavename.compare("bior3.3") == 0) {
        member = 33;
    } else if (wavename.compare("bior3.5") == 0) {
        member = 35;
    } else if (wavename.compare("bior3.7") == 0) {
        member = 37;
    } else if (wavename.compare("bior3.9") == 0) {
        member = 39;
    } else if (wavename.compare("bior4.4") == 0) {
        member = 44;
    } else {
        member = 11;    // default to bior1.1
    }

    _analysis_initialize(member);
    _synthesis_initialize(member);
}

WaveFiltBior::~WaveFiltBior() {}
