/*
 * -------------------------------------------------------------------------
 * Daublet wavelets coefficents.
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
#include <vapor/WaveFiltDaub.h>

using namespace VAPoR;

/*********************************************
 * Local Variable (Filter Coefficent)
 ********************************************/
namespace {

//const double db1[2] = { 0.70710678, 0.70710678 };
const double db1[2] = {sqrt(2.0) / 2.0, sqrt(2.0) / 2.0};

const double db2[4] = {0.48296291314453414337487159986,
                       0.83651630373780790557529378092,
                       0.22414386804201338102597276224,
                       -0.12940952255126038117444941881};

const double db3[6] = {0.33267055295008261599851158914,
                       0.80689150931109257649449360409,
                       0.45987750211849157009515194215,
                       -0.13501102001025458869638990670,
                       -0.08544127388202666169281916918,
                       0.03522629188570953660274066472};

const double db4[8] = {0.23037781330889650086329118304,
                       0.71484657055291564708992195527,
                       0.63088076792985890788171633830,
                       -0.02798376941685985421141374718,
                       -0.18703481171909308407957067279,
                       0.03084138183556076362721936253,
                       0.03288301166688519973540751355,
                       -0.01059740178506903210488320852};

const double db5[10] = {0.16010239797419291448072374802,
                        0.60382926979718967054011930653,
                        0.72430852843777292772807124410,
                        0.13842814590132073150539714634,
                        -0.24229488706638203186257137947,
                        -0.03224486958463837464847975506,
                        0.07757149384004571352313048939,
                        -0.00624149021279827427419051911,
                        -0.01258075199908199946850973993,
                        0.00333572528547377127799818342};

const double db6[12] = {0.11154074335010946362132391724,
                        0.49462389039845308567720417688,
                        0.75113390802109535067893449844,
                        0.31525035170919762908598965481,
                        -0.22626469396543982007631450066,
                        -0.12976686756726193556228960588,
                        0.09750160558732304910234355254,
                        0.02752286553030572862554083950,
                        -0.03158203931748602956507908070,
                        0.00055384220116149613925191840,
                        0.00477725751094551063963597525,
                        -0.00107730108530847956485262161};

const double db7[14] = {0.07785205408500917901996352196,
                        0.39653931948191730653900039094,
                        0.72913209084623511991694307034,
                        0.46978228740519312247159116097,
                        -0.14390600392856497540506836221,
                        -0.22403618499387498263814042023,
                        0.07130921926683026475087657050,
                        0.08061260915108307191292248036,
                        -0.03802993693501441357959206160,
                        -0.01657454163066688065410767489,
                        0.01255099855609984061298988603,
                        0.00042957797292136652113212912,
                        -0.00180164070404749091526826291,
                        0.00035371379997452024844629584

};

const double db8[16] = {0.05441584224310400995500940520,
                        0.31287159091429997065916237551,
                        0.67563073629728980680780076705,
                        0.58535468365420671277126552005,
                        -0.01582910525634930566738054788,
                        -0.28401554296154692651620313237,
                        0.00047248457391328277036059001,
                        0.12874742662047845885702928751,
                        -0.01736930100180754616961614887,
                        -0.04408825393079475150676372324,
                        0.01398102791739828164872293057,
                        0.00874609404740577671638274325,
                        -0.00487035299345157431042218156,
                        -0.00039174037337694704629808036,
                        0.00067544940645056936636954757,
                        -0.00011747678412476953373062823};

const double db9[18] = {0.03807794736387834658869765888,
                        0.24383467461259035373204158165,
                        0.60482312369011111190307686743,
                        0.65728807805130053807821263905,
                        0.13319738582500757619095494590,
                        -0.29327378327917490880640319524,
                        -0.09684078322297646051350813354,
                        0.14854074933810638013507271751,
                        0.03072568147933337921231740072,
                        -0.06763282906132997367564227483,
                        0.00025094711483145195758718975,
                        0.02236166212367909720537378270,
                        -0.00472320475775139727792570785,
                        -0.00428150368246342983449679500,
                        0.00184764688305622647661912949,
                        0.00023038576352319596720521639,
                        -0.00025196318894271013697498868,
                        0.00003934732031627159948068988};

const double db10[20] = {0.02667005790055555358661744877,
                         0.18817680007769148902089297368,
                         0.52720118893172558648174482796,
                         0.68845903945360356574187178255,
                         0.28117234366057746074872699845,
                         -0.24984642432731537941610189792,
                         -0.19594627437737704350429925432,
                         0.12736934033579326008267723320,
                         0.09305736460357235116035228984,
                         -0.07139414716639708714533609308,
                         -0.02945753682187581285828323760,
                         0.03321267405934100173976365318,
                         0.00360655356695616965542329142,
                         -0.01073317548333057504431811411,
                         0.00139535174705290116578931845,
                         0.00199240529518505611715874224,
                         -0.00068585669495971162656137098,
                         -0.00011646685512928545095148097,
                         0.00009358867032006959133405013,
                         -0.00001326420289452124481243668};

}; // namespace

void WaveFiltDaub::_analysis_initialize(int member) {
    const double *pFilterCoef = NULL;

    _filterLength = 2 * member;

    switch (member) {
    case 1:
        pFilterCoef = db1;
        break;
    case 2:
        pFilterCoef = db2;
        break;
    case 3:
        pFilterCoef = db3;
        break;
    case 4:
        pFilterCoef = db4;
        break;
    case 5:
        pFilterCoef = db5;
        break;
    case 6:
        pFilterCoef = db6;
        break;
    case 7:
        pFilterCoef = db7;
        break;
    case 8:
        pFilterCoef = db8;
        break;
    case 9:
        pFilterCoef = db9;
        break;
    case 10:
        pFilterCoef = db10;
        break;
    default:
        assert(pFilterCoef != NULL);
    }

    wrev(pFilterCoef, _lowDecomFilCoef, _filterLength);
    qmf_wrev(pFilterCoef, _hiDecomFilCoef, _filterLength);

    return;
}

void WaveFiltDaub::_synthesis_initialize(int member) {
    const double *pFilterCoef = NULL;

    _filterLength = 2 * member;

    switch (member) {
    case 1:
        pFilterCoef = db1;
        break;
    case 2:
        pFilterCoef = db2;
        break;
    case 3:
        pFilterCoef = db3;
        break;
    case 4:
        pFilterCoef = db4;
        break;
    case 5:
        pFilterCoef = db5;
        break;
    case 6:
        pFilterCoef = db6;
        break;
    case 7:
        pFilterCoef = db7;
        break;
    case 8:
        pFilterCoef = db8;
        break;
    case 9:
        pFilterCoef = db9;
        break;
    case 10:
        pFilterCoef = db10;
        break;
    default:
        assert(pFilterCoef != NULL);
    }

    verbatim_copy(pFilterCoef, _lowReconFilCoef, _filterLength);
    qmf_even(pFilterCoef, _hiReconFilCoef, _filterLength);

    return;
}

WaveFiltDaub::WaveFiltDaub(
    const string &wavename) : WaveFiltBase() {
    int member = -1;

    if (wavename.compare("db1") == 0) {
        member = 1;
    } else if (wavename.compare("db2") == 0) {
        member = 2;
    } else if (wavename.compare("db3") == 0) {
        member = 3;
    } else if (wavename.compare("db4") == 0) {
        member = 4;
    } else if (wavename.compare("db5") == 0) {
        member = 5;
    } else if (wavename.compare("db6") == 0) {
        member = 6;
    } else if (wavename.compare("db7") == 0) {
        member = 7;
    } else if (wavename.compare("db8") == 0) {
        member = 8;
    } else if (wavename.compare("db9") == 0) {
        member = 9;
    } else if (wavename.compare("db10") == 0) {
        member = 10;
    } else {
        member = 1; // default db1
    }

    _analysis_initialize(member);
    _synthesis_initialize(member);
}

WaveFiltDaub::~WaveFiltDaub() {
}
