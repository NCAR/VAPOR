#include <iostream>
#include <cmath>
#include <cassert>
#include <vapor/MatWaveBase.h>
#include <vapor/WaveFiltDaub.h>
#include <vapor/WaveFiltCoif.h>
#include <vapor/WaveFiltBior.h>
#include <vapor/WaveFiltHaar.h>
#include <vapor/WaveFiltInt.h>

using namespace VAPoR;

MatWaveBase::MatWaveBase(const string &wname, const string &mode) {
    _wf = NULL;
    _mode = PER;
    _wname = wname;

    _wf = _create_wf(wname);
    if (!_wf)
        return;

    if (dwtmode(mode) < 0) {
        return; // nothing to do
    };

    _InvalidFloatAbort = false;
}

MatWaveBase::MatWaveBase(const string &wname) {
    _wf = NULL;
    _mode = PER;
    _wname = wname;

    _wf = _create_wf(wname);
    if (!_wf)
        return;

    string wmode;
    if ((_wname.compare("bior1.1") == 0) ||
        (_wname.compare("bior1.3") == 0) ||
        (_wname.compare("bior1.5") == 0) ||
        (_wname.compare("bior3.3") == 0) ||
        (_wname.compare("bior3.5") == 0) ||
        (_wname.compare("bior3.7") == 0) ||
        (_wname.compare("bior3.9") == 0)) {

        wmode = "symh";
    } else if ((_wname.compare("bior2.2") == 0) ||
               (_wname.compare("bior2.4") == 0) ||
               (_wname.compare("bior2.6") == 0) ||
               (_wname.compare("bior2.8") == 0) ||
               (_wname.compare("bior4.4") == 0) ||
               (_wname.compare("intbior2.2") == 0)) {

        wmode = "symw";
    } else {
        wmode = "sp0";
    }
    if (dwtmode(wmode) < 0) {
        return; // nothing to do
    };

    _InvalidFloatAbort = false;
}

MatWaveBase::~MatWaveBase() {
    if (_wf)
        delete _wf;
    _wf = NULL;
}

int MatWaveBase::dwtmode(const string &mode) {
    dwtmode_t imode = _dwtmodestr2enum(mode);

    if (imode == INVALID) {
        SetErrMsg("Invalid boundary handling mode : %s", mode.c_str());
        return (-1);
    }

    _mode = imode;
    return (0);
}

int MatWaveBase::dwtmode(dwtmode_t mode) {

    if (mode == INVALID) {
        SetErrMsg("Invalid boundary handling mode : %d", mode);
        return (-1);
    }
    _mode = mode;
    return (0);
}

int MatWaveBase::wavelet(const string &wname) {
    if (_wf)
        delete _wf;

    _wf = _create_wf(wname);
    if (!_wf) {
        SetErrMsg("Invalid wavelet : %s", wname.c_str());
        return (-1);
    }

    return (0);
}

const string MatWaveBase::dwtmode() const {
    return (_dwtmodeenum2str(_mode));
}

size_t MatWaveBase::wmaxlev(size_t s) const {
    size_t lev, val;

    if (!_wf)
        return (0);

    int waveLength = _wf->GetLength();
    _wave_len_validate(s, waveLength, &lev, &val);

    if (!val)
        return (0);
    return (lev);
}

size_t MatWaveBase::detaillength(size_t sigInLen) const {
    if (!_wf)
        return (0);

    int filterLen = _wf->GetLength();

    if (_mode == PER) {
        return ((size_t)ceil(((double)(sigInLen)) / 2.0));
    } else if (_wf->issymmetric()) {
        if (
            (_mode == MatWaveBase::SYMW && (filterLen % 2)) ||
            (_mode == MatWaveBase::SYMH && (!(filterLen % 2)))) {
            if (sigInLen % 2) {
                return ((sigInLen - 1) / 2);
            } else {
                return ((sigInLen) / 2);
            }
        }
    }

    return ((size_t)floor((sigInLen + filterLen - 1.0) / 2.0));
}

size_t MatWaveBase::approxlength(size_t sigInLen) const {
    if (!_wf)
        return (sigInLen);

    int filterLen = _wf->GetLength();

    if (_mode == PER) {
        return ((size_t)ceil(((double)(sigInLen)) / 2.0));
    } else if (_wf->issymmetric()) {
        if (
            (_mode == MatWaveBase::SYMW && (filterLen % 2)) ||
            (_mode == MatWaveBase::SYMH && (!(filterLen % 2)))) {
            if (sigInLen % 2) {
                return ((sigInLen + 1) / 2);
            } else {
                return ((sigInLen) / 2);
            }
        }
    }

    return ((size_t)floor((double)((sigInLen + filterLen - 1) / 2)));
}

void MatWaveBase::_wave_len_validate(
    size_t sigInLen, int waveLength, size_t *lev, size_t *status) const {
    *lev = 0;
    *status = 1;

    double d = (log((double)sigInLen / (double)(waveLength)) / log(2.0));
    if (d < 0) {
        *status = 0;
        return;
    }
    *lev = (int)d + 1;
}

WaveFiltBase *MatWaveBase::_create_wf(const string &wname) const {
    WaveFiltBase *wf = NULL;

    if (wname.compare(0, 2, "db") == 0) {
        wf = new WaveFiltDaub(wname);
    } else if (wname.compare(0, 4, "coif") == 0) {
        wf = new WaveFiltCoif(wname);
    } else if (wname.compare(0, 4, "bior") == 0) {
        wf = new WaveFiltBior(wname);
    } else if (wname.compare(0, 4, "haar") == 0) {
        wf = new WaveFiltHaar();
    } else if (wname.compare(0, 3, "int") == 0) {
        wf = new WaveFiltInt(wname);
    } else {
        return (NULL);
    }

    return (wf);
}

MatWaveBase::dwtmode_t MatWaveBase::_dwtmodestr2enum(const string &mode) const {

    if ((mode.compare("zpd") == 0) || (mode.compare("ZPD") == 0)) {
        return (ZPD);
    } else if ((mode.compare("symh") == 0) || (mode.compare("SYMH") == 0)) {
        return (SYMH);
    } else if ((mode.compare("symw") == 0) || (mode.compare("SYMW") == 0)) {
        return (SYMW);
    } else if ((mode.compare("asymh") == 0) || (mode.compare("ASYMH") == 0)) {
        return (ASYMH);
    } else if ((mode.compare("asymw") == 0) || (mode.compare("ASYMW") == 0)) {
        return (ASYMW);
    } else if ((mode.compare("sp0") == 0) || (mode.compare("SP0") == 0)) {
        return (SP0);
    } else if ((mode.compare("sp1") == 0) || (mode.compare("SP1") == 0)) {
        return (SP1);
    } else if ((mode.compare("spd") == 0) || (mode.compare("SPD") == 0)) {
        return (SP1);
    } else if ((mode.compare("ppd") == 0) || (mode.compare("PPD") == 0)) {
        return (PPD);
    } else if ((mode.compare("per") == 0) || (mode.compare("PER") == 0)) {
        return (PER);
    } else {
        return (INVALID);
    }
}

string MatWaveBase::_dwtmodeenum2str(dwtmode_t mode) const {

    switch (mode) {
    case ZPD:
        return ("zpd");
        break;
    case SYMH:
        return ("symh");
        break;
    case SYMW:
        return ("symw");
        break;
    case ASYMH:
        return ("asymh");
        break;
    case ASYMW:
        return ("asymw");
        break;
    case SP0:
        return ("sp0");
        break;
    case SP1:
        return ("sp1");
        break;
    case PPD:
        return ("ppd");
        break;
    case PER:
        return ("per");
        break;
    default:
        return ("invalid");
        break;
    }
}
