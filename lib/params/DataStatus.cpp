//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		DataStatus.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2006
//
//	Description:	Implements the DataStatus class
//
#ifdef WIN32
#pragma warning(disable : 4251 4100)
#endif

#include <iostream>
#include <cassert>
#include <cfloat>

#include <vapor/DataStatus.h>
#include <vapor/DataMgr.h>
#include <vapor/Proj4API.h>
#include <vapor/MapperFunction.h>

using namespace VAPoR;
using namespace Wasp;

//Default constructor
//Whether or not it exists on disk, what's its max and min
//What resolutions are available.
//
DataStatus::DataStatus(size_t cacheSize, int nThreads) {

    _cacheSize = cacheSize;
    _nThreads = nThreads;

    _dataMgrs.clear();
    _activeDataMgr = "";
    _useLowerAccuracy = true;

    reset();
}

int DataStatus::Open(
    const std::vector<string> &files, string name, string format) {

    if (name.empty())
        name = "DataSet1";

    Close(name);

    DataMgr *dataMgr = new DataMgr(format, _cacheSize, _nThreads);

    int rc = dataMgr->Initialize(files);
    if (rc < 0) {
        delete dataMgr;
        return (-1);
    }

    _dataMgrs[name] = dataMgr;
    _activeDataMgr = name;

    reset();

    return (0);
}

void DataStatus::Close(string name) {

    if (name.empty())
        name = "DataSet1";

    map<string, DataMgr *>::iterator itr;
    if ((itr = _dataMgrs.find(name)) != _dataMgrs.end()) {
        if (itr->second)
            delete itr->second;
        _dataMgrs.erase(itr);
    }

    if (_activeDataMgr == name) {
        itr = _dataMgrs.begin();
        if (itr != _dataMgrs.end())
            _activeDataMgr = itr->first;
    }
}

//const DataMgr *DataStatus::GetDataMgr(string name) const
DataMgr *DataStatus::GetDataMgr(string name) const {

    if (name.empty())
        name = "DataSet1";

    map<string, DataMgr *>::const_iterator itr;
    itr = _dataMgrs.find(name);

    if (itr == _dataMgrs.end())
        return (NULL);

    return (itr->second);
}

void DataStatus::SetActiveDataMgr(string name) {
    _activeDataMgr = name;

    // Need to manage lists of DMs. For now just handle one
    //
    reset();
}

// After data is loaded or if there is a merge, call DataStatus::reset to
// add additional and/or modify previous variables
// return true if there was anything to set up.
//
// If there are python scripts use their output variables.
// If the python scripts inputs are not in the DataMgr, remove the input variables.
//
// Set the extents based on the domainVariables in the Region
void DataStatus::reset() {
    _minTimeStep = 0;
    _maxTimeStep = 0;
    _numTimesteps = 0;

    for (int i = 0; i < 3; i++) {
        _extents[i] = 0.f;
        _extents[i + 3] = 1.f;
        _stretchedExtents[i] = 0.f;
        _stretchedExtents[i + 3] = 1.f;
        _fullSizes[i] = 1.f;
        _fullStretchedSizes[i] = 1.f;
        _stretchFactors.push_back(1.0);
    }
    _domainVars.clear();

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return;

    vector<double> timecoords;
    dataMgr->GetTimeCoordinates(timecoords);
    size_t numTS = timecoords.size();
    assert(numTS != 0);

    _numTimesteps = numTS;

    //Find the first and last time for which there is data:
    vector<string> varnames = dataMgr->GetDataVarNames();
    int mints = -1;
    int maxts = -1;
    for (size_t i = 0; i < numTS; i++) {
        for (int j = 0; j < varnames.size(); j++) {
            if (!dataMgr->VariableExists(i, varnames[j]))
                continue;
            mints = i;
            break;
        }
        if (mints >= 0)
            break;
    }
    for (int i = numTS - 1; i >= 0; i--) {
        for (int j = 0; j < varnames.size(); j++) {
            if (!dataMgr->VariableExists((size_t)i, varnames[j]))
                continue;
            maxts = i;
            break;
        }
        if (maxts >= 0)
            break;
    }
    assert(!(mints < 0 || maxts < 0));
    _minTimeStep = mints;
    _maxTimeStep = maxts;

#ifdef DEAD
    //Determine the domain extents.  Obtain them from RegionParams, but
    //if they are not available, find the first variable in the VDC
    vector<string> domainVars = RegionParams::GetDomainVariables();
    //Determine the range of time steps for which there is data.
#endif

    size_t firstTimeStep = getMinTimestep();

    bool getDomainVars = true;

#ifdef DEAD
    if (domainVars.size() > 0) {
        //see if a variable exists at the first time.  If so use the domainVars.  Otherwise find another
        getDomainVars = false;
        for (int j = 0; j < domainVars.size(); j++) {
            if (!dataMgr->VariableExists(firstTimeStep, domainVars[j])) {
                getDomainVars = true;
                break;
            }
        }
    }

#endif

    if (getDomainVars) {
        //Use the first variable that has data at the first time step.
        _domainVars.clear();
        vector<string> varnames = dataMgr->GetDataVarNames(3, true);
        for (int j = 0; j < varnames.size(); j++) {
            string varname = varnames[j];
            if (dataMgr->VariableExists(firstTimeStep, varname)) {
                _domainVars.push_back(varname);
                //RegionParams::SetDomainVariables(domainVars);
                getDomainVars = false;
                break;
            }
        }
    }
    //Now use the domain vars to calculate extents. They will be the union of all the extents of the domain-defining variables:
    vector<double> minVarExts, maxVarExts;
    bool ok = GetExtents(firstTimeStep, minVarExts, maxVarExts);
    assert(ok);
    vector<double> stretchFactors = getStretchFactors();
    for (int i = 0; i < 3; i++) {
#ifdef DEAD
        _extents[i + 3] = (maxVarExts[i] - minVarExts[i]);
        _extents[i] = 0.;
#else
        _extents[i] = minVarExts[i];
        _extents[i + 3] = maxVarExts[i];
#endif
        _fullSizes[i] = (maxVarExts[i] - minVarExts[i]);
        _fullStretchedSizes[i] = _fullSizes[i] * stretchFactors[i];
        _stretchedExtents[i] = _extents[i] * stretchFactors[i];
        _stretchedExtents[i + 3] = _extents[i + 3] * stretchFactors[i];
    }

    return;
}

DataStatus::
    ~DataStatus() {
}

#ifdef DEAD
//Map corners of box to voxels.
void DataStatus::mapBoxToVox(
    Box *box, string varname, int refLevel, int lod, int timestep,
    size_t voxExts[6]) {
    double userExts[6];
    box->GetUserExtents(userExts, (size_t)timestep);
    vector<double> minexts, maxexts;
    for (int i = 0; i < 3; i++) {
        minexts.push_back(userExts[i]);
        maxexts.push_back(userExts[i + 3]);
    }
    bool errEnabled = MyBase::EnableErrMsg(false);
    StructuredGrid *rg = dataMgr->GetVariable(timestep, varname, refLevel, lod, minexts, maxexts);
    MyBase::EnableErrMsg(errEnabled);

    if (rg) {
        rg->GetIJKIndex(minexts[0], minexts[1], minexts[2], voxExts, voxExts + 1, voxExts + 2);
        rg->GetIJKIndex(maxexts[0], maxexts[1], maxexts[2], voxExts + 3, voxExts + 4, voxExts + 5);
    } else {
        for (int i = 0; i < 6; i++)
            voxExts[i] = 0;
    }
    //(Note: this can be expensive with layered data)
    return;
}

#endif
void DataStatus::
    localToStretched(const double fromCoords[3], double toCoords[3]) {
    vector<double> stretch = getStretchFactors();
    for (int i = 0; i < 3; i++) {
        toCoords[i] = (fromCoords[i] * stretch[i]);
    }
    return;
}
int DataStatus::maxXFormPresent(string varname, size_t timestep) const {

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return (0);

    int maxx = dataMgr->GetNumRefLevels(varname);
    int i;
    for (i = 0; i < maxx; i++) {
        if (!dataMgr->VariableExists(timestep, varname, i))
            return (i - 1);
    }
    return i - 1;
}
int DataStatus::maxLODPresent(string varname, size_t timestep) const {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return (0);

    vector<size_t> ratios = dataMgr->GetCRatios(varname);
    int i;
    for (i = 0; i < ratios.size(); i++) {
        if (!dataMgr->VariableExists(timestep, varname, 0, i))
            break;
    }
    return i - 1;
}

//static methods to convert coordinates to and from latlon
//coordinates are in the order longitude,latitude
//result is in user coordinates.
bool DataStatus::convertFromLonLat(double coords[2], int npoints) {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return (true);

    //Set up proj.4 to convert from LatLon to VDC coords
    string projString = dataMgr->GetMapProjection();
    if (projString.size() == 0)
        return false;

    Proj4API proj4API;
    int rc = proj4API.Initialize("", projString);
    if (rc < 0)
        return (false);

    rc = proj4API.Transform(coords, coords + 1, npoints, 2);
    if (rc < 0)
        return (false);

    return true;
}
bool DataStatus::convertLocalFromLonLat(int timestep, double coords[2], int npoints) {

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return (true);

    if (!convertFromLonLat(coords, npoints))
        return false;
    vector<double> minExts, maxExts;
    int rc = GetExtents((size_t)timestep, minExts, maxExts);
    if (rc)
        return false;
    for (int i = 0; i < npoints; i++) {
        coords[2 * i] -= minExts[0];
        coords[2 * i + 1] -= minExts[1];
    }
    return true;
}
bool DataStatus::convertLocalToLonLat(int timestep, double coords[2], int npoints) {

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return (true);

    vector<double> minExts, maxExts;
    int rc = GetExtents((size_t)timestep, minExts, maxExts);
    if (rc)
        return false;

    //Convert local to user coordinates:
    for (int i = 0; i < npoints; i++) {
        coords[2 * i] += minExts[0];
        coords[2 * i + 1] += minExts[1];
    }
    if (!convertToLonLat(coords, npoints))
        return false;
    return true;
}
//coordinates are always in user coordinates.
bool DataStatus::convertToLonLat(double coords[2], int npoints) {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return (true);

    //Set up proj.4 to convert to latlon
    string pstring = dataMgr->GetMapProjection();
    if (pstring.size() == 0)
        return false;

    Proj4API proj4API;
    int rc = proj4API.Initialize(pstring, "");
    if (rc < 0)
        return (false);

    rc = proj4API.Transform(coords, coords + 1, npoints, 2);
    if (rc < 0)
        return (false);

    return true;
}

bool DataStatus::GetExtents(
    size_t timestep, vector<double> &minExts, vector<double> &maxExts) {
    minExts.clear();
    maxExts.clear();

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        for (int i = 0; i < 3; i++) {
            minExts.push_back(0.0);
            maxExts.push_back(0.0);
        }
        return (true);
    }

    vector<double> tempMin, tempMax;
    bool varfound = false;

    for (int j = 0; j < _domainVars.size(); j++) {
        int rc = dataMgr->GetVariableExtents(timestep, _domainVars[j], maxXFormPresent(_domainVars[j], timestep), tempMin, tempMax);
        if (rc != 0)
            continue;
        if (!varfound) {
            minExts = tempMin;
            maxExts = tempMax;
            varfound = true;
        } else {
            for (int k = 0; k < 3; k++) {
                if (minExts[k] > tempMin[k])
                    minExts[k] = tempMin[k];
                if (maxExts[k] < tempMax[k])
                    maxExts[k] = tempMax[k];
            }
        }
    }
    return varfound;
}

bool DataStatus::dataIsPresent(int timestep) {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr)
        return (false);

    vector<string> vars = dataMgr->GetDataVarNames();
    for (int i = 0; i < vars.size(); i++) {
        if (dataMgr->VariableExists(timestep, vars[i]))
            return true;
    }
    return false;
}
int DataStatus::mapVoxToUser(
    size_t timestep, string varname, const size_t vcoords[3],
    double uCoords[3], int reflevel) {

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        for (int i = 0; i < 3; i++)
            uCoords[i] = 0.0;
        return (0);
    }

    StructuredGrid *rGrid = dataMgr->GetVariable(timestep, varname, reflevel, 0);
    //Force the coordinates to valid values
    size_t wcoords[3];
    size_t dims[3];
    rGrid->GetDimensions(dims);
    for (int i = 0; i < 3; i++) {
        size_t w = vcoords[i];
        if (w >= dims[i])
            w = dims[i] - 1;
        wcoords[i] = w;
    }
    int rc = rGrid->GetUserCoordinates(wcoords[0], wcoords[1], wcoords[2], uCoords, uCoords + 1, uCoords + 2);
    return rc;
}
void DataStatus::mapUserToVox(
    size_t timestep, string varname, const double uCoords[3],
    size_t vCoords[3], int reflevel) {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        for (int i = 0; i < 3; i++)
            vCoords[i] = 0.0;
        return;
    }

    StructuredGrid *rGrid = dataMgr->GetVariable(timestep, varname, reflevel, 0);
    rGrid->GetIJKIndex(uCoords[0], uCoords[1], uCoords[2], vCoords, vCoords + 1, vCoords + 2);
}

#ifdef DEAD

int DataStatus::getActiveVarIndex(int dim, string varname) {
    vector<string> varnames = dataMgr->GetDataVarNames(dim, true);
    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i] == varname)
            return i;
    }
    return -1;
}

#endif

double DataStatus::getVoxelSize(
    size_t ts, string varname, int refLevel, int dir) {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        return (1.0);
    }

    //If refLevel is -1, use maximum refinement level
    //Obtain the variable at lowest refinement level, then convert to higher levels if needed
    //If dir is -1 get maximum side of voxel
    //If dir is -2 get minimum side of voxel
    StructuredGrid *rGrid = dataMgr->GetVariable(ts, varname, 0, 0);
    if (refLevel == -1)
        refLevel = dataMgr->GetNumRefLevels(varname) - 1;
    size_t dims[3];
    rGrid->GetDimensions(dims);
    for (int i = 0; i < 3; i++)
        dims[i] *= 2 << refLevel;
    double extents[6];
    rGrid->GetUserExtents(extents);
    int numdims = rGrid->GetRank();
    if (numdims < dir)
        return 0.;
    if (dir >= 0) {
        double vsize = ((extents[dir + 3] - extents[dir]) / dims[dir]);
        return vsize;
    } else if (dir == -1) { //maximum size
        double maxsize = -1.;
        for (int i = 0; i < numdims; i++) {
            double vsize = ((extents[dir + 3] - extents[dir]) / dims[dir]);
            if (vsize > maxsize)
                maxsize = vsize;
        }
        return maxsize;
    } else { //minimum size
        double minsize = DBL_MAX;
        for (int i = 0; i < numdims; i++) {
            double vsize = ((extents[dir + 3] - extents[dir]) / dims[dir]);
            if (vsize < minsize)
                minsize = vsize;
        }
        return minsize;
    }
}

bool DataStatus::GetDefaultVariableRange(
    string varname, MapperFunction *mf, float minmax[2]) {
    minmax[0] = minmax[1] = 0.0;

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        return (true);
    }

    bool first = false;
    if (mf)
        first = (mf->getMinMapValue() > mf->getMaxMapValue());
    if (!dataMgr->VariableExists(_minTimeStep, varname, 0, 0))
        return false;
    StructuredGrid *rGrid = dataMgr->GetVariable(_minTimeStep, varname, 0, 0);
    rGrid->GetRange(minmax);
    if (!first) {
        return false;
    } else { //Actually do need to set bounds
        if (mf)
            mf->setMinMaxMapValue(minmax[0], minmax[1]);
        return true;
    }
}

// Obtain grids for a set of variables in requested user extents.
// Pointer to requested LOD and refLevel, may change if not available
// Extents are reduced if data not available at requested extents.
// Vector of varnames can include "" for zero variable.
// Variables can be 2D or 3D depending on value of "varsAre2D"
// Returns 0 on success
//
int DataStatus::getGrids(
    size_t ts, const vector<string> &varnames,
    const vector<double> &minExtsReq, const vector<double> &maxExtsReq,
    int *refLevel, int *lod, StructuredGrid **grids) const {
    assert(minExtsReq.size() == maxExtsReq.size());

    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        return (-1);
    }

    for (int i = 0; i < varnames.size(); i++)
        grids[i] = NULL;

    // First, find an lod and a refinement level that will work with
    // all variables.
    //
    int tempRefLevel = *refLevel;
    int tempLOD = *lod;
    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i].empty())
            continue;

        // Legacy crap
        //
        assert(varnames[i] != "0");

        tempRefLevel = Min(
            maxXFormPresent(varnames[i], ts), tempRefLevel);

        tempLOD = Min(maxLODPresent(varnames[i], ts), tempLOD);
    }

    if (useLowerAccuracy()) {
        *lod = tempLOD;
        *refLevel = tempRefLevel;
    } else {
        if (tempRefLevel < *refLevel || tempLOD < *lod) {
            MyBase::SetErrMsg(
                "Variable not present at required refinement and LOD");
            return -1;
        }
    }

    if (*refLevel < 0 || *lod < 0) {
        MyBase::SetErrMsg(
            "Variable not present at required refinement and LOD");
        return -1;
    }

    // Determine what region (inside requested extents) is available
    // for all variables
    //
    vector<double> minExts, maxExts, tminExts, tmaxExts;
    int numvars = 0;
    int ndim;
    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i].empty())
            continue;

        // Find the intersection of extents of the variables in user coords
        //
        numvars++;
        if (numvars == 1) {
            int rc = dataMgr->GetVariableExtents(
                ts, varnames[i], *refLevel, minExts, maxExts);
            ndim = minExts.size();
            if (rc < 0)
                return rc;

            for (int j = 0; j < minExts.size(); j++) {
                //shrink minExts/maxExts to requested extents
                if (minExts[j] < minExtsReq[j]) {
                    minExts[j] = minExtsReq[j];
                }
                if (maxExts[j] > maxExtsReq[j])
                    maxExts[j] = maxExtsReq[j];
            }

        } else { //2nd and later variables
            int rc = dataMgr->GetVariableExtents(
                ts, varnames[i], *refLevel, tminExts, tmaxExts);
            if (rc < 0)
                return rc;

            assert(ndim == tminExts.size());

            for (int j = 0; j < tminExts.size(); j++) {
                // shrink minExts,maxExts to tminExts/tmaxExts
                //
                if (tminExts[j] > minExts[j])
                    minExts[j] = tminExts[j];
                if (tmaxExts[j] < maxExts[j])
                    maxExts[j] = tmaxExts[j];
            }
        }

        // If there is no valid intersection, the min will be
        // greater than the max
        //
        for (int j = 0; j < minExts.size(); j++)
            if (maxExts[j] < minExts[j]) {
                MyBase::SetErrMsg("Variable extents invalid");
                return -1;
            }
    }

    //Now obtain a regular grid for each valid variable

    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i].empty())
            continue;

        tminExts = minExts;
        tmaxExts = maxExts;

        StructuredGrid *rGrid = dataMgr->GetVariable(
            ts, varnames[i], *refLevel, *lod, tminExts, tmaxExts, true);

        if (!rGrid) {
            for (int j = 0; j < i; j++) {
                if (grids[j])
                    dataMgr->UnlockGrid(grids[j]);
            }
            MyBase::SetErrMsg("Error retrieving variable data");
            return -1;
        }
        grids[i] = rGrid;
    }

#ifdef DEAD
    //see if we have enough space:
    int numMBs = 0;
    const vector<string> varnames3D = dataMgr->GetDataVarNames(3, true);
    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i] == "0" || varnames[i] == "")
            continue;
        bool varIs3D = (ndim == 3);

        size_t dims[3];
        grids[i]->GetDimensions(dims);
        if (varIs3D)
            numMBs += 4 * (dims[0]) * (dims[1]) * (dims[2]) / 1000000;
        else
            numMBs += 4 * (dims[0]) * (dims[1]) / 1000000;
    }
    int cacheSize = _dataStatus->getCacheMB();
    if (numMBs > (int)(cacheSize * 0.75)) {
        MyBase::SetErrMsg(VAPOR_ERROR_DATA_TOO_BIG, "Current cache size is too small\nfor requested extents and resolution.\n%s",
                          "Lower the refinement level, reduce the size, or increase the cache size.");
        for (int j = 0; j < varnames.size(); j++) {
            if (grids[j])
                dataMgr->UnlockGrid(grids[j]);
        }
        return -1;
    }
#endif

    //obtained all of the grids needed
    return 0;
}

int DataStatus::getGrids(
    size_t ts, const vector<string> &varnames,
    int *refLevel, int *lod, StructuredGrid **grids) const {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        return (-1);
    }

    vector<double> minExtsReq, maxExtsReq;
    for (int i = 0; i < 3; i++) {
        minExtsReq.push_back(std::numeric_limits<double>::min());
    }

    for (int i = 0; i < 3; i++) {
        maxExtsReq.push_back(std::numeric_limits<double>::max());
    }

    return (
        DataStatus::getGrids(
            ts, varnames, minExtsReq, maxExtsReq, refLevel, lod, grids));
}

int DataStatus::getGrids(
    size_t ts, const vector<string> &varnames,
    const double extents[6],
    int *refLevel, int *lod, StructuredGrid **grids) const {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        return (-1);
    }

    if (!varnames.size())
        return (0);

    vector<size_t> dims_at_level;
    vector<size_t> bs_at_level;

    int rc = dataMgr->GetDimLensAtLevel(
        varnames[0], 0, dims_at_level, bs_at_level);
    if (rc < 0)
        return (-1);

    vector<double> minExtsReq, maxExtsReq;
    for (int i = 0; i < dims_at_level.size(); i++) {
        minExtsReq.push_back(extents[i]);
        maxExtsReq.push_back(extents[i + 3]);
    }

    return (getGrids(
        ts, varnames, minExtsReq, maxExtsReq, refLevel, lod, grids));
}
