//************************************************************************
//									*
//		     Copyright (C)  2017				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		DataMgrUtils.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2017
//
//	Description:	Implements the DataMgrUtils free functions
//
#ifdef WIN32
#pragma warning(disable : 4251 4100)
#endif

#include <iostream>
#include <cassert>
#include <algorithm>
#include <cfloat>

#include <vapor/DataMgr.h>
#include <vapor/Proj4API.h>
#include <vapor/DataMgrUtils.h>

using namespace VAPoR;
using namespace Wasp;

bool DataMgrUtils::MaxXFormPresent(
    const DataMgr *dataMgr, size_t timestep, string varname, size_t &maxXForm) {

    size_t maxx = dataMgr->GetNumRefLevels(varname);

    maxXForm = 0;
    for (; maxXForm < maxx; maxXForm++) {
        if (!dataMgr->VariableExists(timestep, varname, maxXForm))
            break;
    }
    if (maxXForm == 0)
        return false;

    maxXForm -= 1;
    return true;
}

bool DataMgrUtils::MaxLODPresent(
    const DataMgr *dataMgr, size_t timestep, string varname, size_t &maxLOD) {
    maxLOD = 0;

    vector<size_t> cratios = dataMgr->GetCRatios(varname);
    assert(cratios.size() > 0);

    for (maxLOD = 0; maxLOD < cratios.size(); maxLOD++) {
        if (!dataMgr->VariableExists(timestep, varname, 0, maxLOD))
            break;
    }
    if (maxLOD == 0)
        return false;

    maxLOD -= 1;
    return true;
}

int DataMgrUtils::ConvertPCSToLonLat(
    const DataMgr *dataMgr, double coords[2], int npoints) {
    //Set up proj.4 to convert to latlon
    string pstring = dataMgr->GetMapProjection();
    if (pstring.size() == 0)
        return 0;

    Proj4API proj4API;
    int rc = proj4API.Initialize(pstring, "");
    if (rc < 0)
        return (rc);

    rc = proj4API.Transform(coords, coords + 1, npoints, 2);
    if (rc < 0)
        return (rc);

    return 0;
}

int DataMgrUtils::ConvertLonLatToPCS(
    const DataMgr *dataMgr, double coords[2], int npoints) {
    //Set up proj.4 to convert from LatLon to VDC coords
    string projString = dataMgr->GetMapProjection();
    if (projString.size() == 0)
        return (0);

    Proj4API proj4API;
    int rc = proj4API.Initialize("", projString);
    if (rc < 0)
        return (rc);

    rc = proj4API.Transform(coords, coords + 1, npoints, 2);
    if (rc < 0)
        return (rc);

    return 0;
}

int DataMgrUtils::GetGrids(
    DataMgr *dataMgr,
    size_t ts, const vector<string> &varnames,
    const vector<double> &minExtsReq, const vector<double> &maxExtsReq,
    bool useLowerAccuracy,
    int *refLevel, int *lod, vector<Grid *> &grids) {
    grids.clear();
    assert(minExtsReq.size() == maxExtsReq.size());

    for (int i = 0; i < varnames.size(); i++)
        grids.push_back(NULL);

    // First, find an lod and a refinement level that will work with
    // all variables.
    //
    int tempRefLevel = *refLevel;
    int tempLOD = *lod;
    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i].empty())
            continue;

        size_t maxRefLevel;
        bool status = MaxXFormPresent(dataMgr, ts, varnames[i], maxRefLevel);
        if (!status) {
            MyBase::SetErrMsg(
                "Variable not present at required refinement and LOD");
            return -1;
        }

        tempRefLevel = std::min((int)maxRefLevel, tempRefLevel);

        size_t maxLOD;
        status = MaxLODPresent(dataMgr, ts, varnames[i], maxLOD);
        if (!status) {
            MyBase::SetErrMsg(
                "Variable not present at required refinement and LOD");
            return -1;
        }

        tempLOD = std::min((int)maxLOD, tempLOD);
    }

    if (useLowerAccuracy) {
        *lod = tempLOD;
        *refLevel = tempRefLevel;
    } else {
        if (tempRefLevel < *refLevel || tempLOD < *lod) {
            MyBase::SetErrMsg(
                "Variable not present at required refinement and LOD");
            return -1;
        }
    }

    // Now obtain a regular grid for each valid variable
    //
    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i].empty())
            continue;

        Grid *rGrid = dataMgr->GetVariable(
            ts, varnames[i], *refLevel, *lod, minExtsReq, maxExtsReq, true);

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

    //obtained all of the grids needed
    return 0;
}

int DataMgrUtils::GetGrids(
    DataMgr *dataMgr,
    size_t ts, string varname,
    const vector<double> &minExtsReq, const vector<double> &maxExtsReq,
    bool useLowerAccuracy,
    int *refLevel, int *lod, Grid **gridptr) {
    *gridptr = NULL;

    if (varname == "") {
        MyBase::SetErrMsg("Cannot get grid for variable \"\"");
        return -1;
    }

    vector<string> varnames;
    varnames.push_back(varname);
    vector<Grid *> grids;
    int rc = GetGrids(
        dataMgr, ts, varnames, minExtsReq, maxExtsReq, useLowerAccuracy,
        refLevel, lod, grids);
    if (rc < 0)
        return (rc);

    *gridptr = grids[0];
    return (0);
}

int DataMgrUtils::GetGrids(
    DataMgr *dataMgr, size_t ts, const vector<string> &varnames,
    bool useLowerAccuracy,
    int *refLevel, int *lod, vector<Grid *> &grids) {
    grids.clear();

    vector<double> minExtsReq, maxExtsReq;
    for (int i = 0; i < 3; i++) {
        minExtsReq.push_back(std::numeric_limits<double>::lowest());
    }

    for (int i = 0; i < 3; i++) {
        maxExtsReq.push_back(std::numeric_limits<double>::max());
    }

    return (
        DataMgrUtils::GetGrids(
            dataMgr, ts, varnames, minExtsReq, maxExtsReq,
            useLowerAccuracy, refLevel, lod, grids));
}

int DataMgrUtils::GetGrids(
    DataMgr *dataMgr,
    size_t ts, string varname,
    bool useLowerAccuracy,
    int *refLevel, int *lod, Grid **gridptr) {
    *gridptr = NULL;

    vector<string> varnames;
    varnames.push_back(varname);
    vector<Grid *> grids;
    int rc = GetGrids(
        dataMgr, ts, varnames, useLowerAccuracy, refLevel, lod, grids);
    if (rc < 0)
        return (rc);

    *gridptr = grids[0];
    return (0);
}

bool DataMgrUtils::GetAxes(
    const DataMgr *dataMgr, string varname, vector<int> &axes) {
    axes.clear();

    vector<string> coordvars;
    bool status = dataMgr->GetVarCoordVars(varname, true, coordvars);
    if (!status)
        return (status);

    for (int i = 0; i < coordvars.size(); i++) {
        VAPoR::DC::CoordVar cvar;

        status = dataMgr->GetCoordVarInfo(coordvars[i], cvar);
        assert(status);

        axes.push_back(cvar.GetAxis());
    }
    return (true);
}

bool DataMgrUtils::GetExtents(
    DataMgr *dataMgr,
    size_t timestep, string varname,
    vector<double> &minExts, vector<double> &maxExts) {
    minExts.clear();
    maxExts.clear();

    // If varname not specified look for first variable of highest
    // dimensionality
    //
    if (varname.empty()) {
        vector<string> varnames;
        for (int ndim = 3; ndim > 0; ndim--) {
            varnames = dataMgr->GetDataVarNames(ndim, true);
            if (!varnames.empty())
                break;
        }
        varname = varnames.size() ? varnames[0] : "";
    }
    if (varname.empty())
        return (false);

    size_t maxXForm;
    bool status = DataMgrUtils::MaxXFormPresent(
        dataMgr, timestep, varname, maxXForm);
    if (!status)
        return (status);

    bool errEnabled = MyBase::EnableErrMsg(false);
    int rc = dataMgr->GetVariableExtents(
        timestep, varname, (int)maxXForm, minExts, maxExts);
    MyBase::EnableErrMsg(errEnabled);

    if (rc < 0)
        return (false);

    return (true);
}

bool DataMgrUtils::GetExtents(
    DataMgr *dataMgr,
    size_t timestep, const vector<string> &varnames,
    vector<double> &minExts, vector<double> &maxExts,
    vector<int> &axes) {
    minExts.clear();
    maxExts.clear();
    axes.clear();

    vector<double> tmpMinExts(3, std::numeric_limits<double>::max());
    vector<double> tmpMaxExts(3, std::numeric_limits<double>::lowest());

    // Get the coordinate extents of each variable. Grow the bounding
    // box to accomodate each new variable. Handle cases where variables
    // have different dimensionality, or, in 2D case, live in different
    // planes.
    //
    // Have to be careful
    // about what coordinate axis the coordinate extents apply to
    //
    for (int i = 0; i < varnames.size(); i++) {

        if (varnames[i] == "") {
            continue;
        }

        vector<double> varMinExts;
        vector<double> varMaxExts;
        vector<int> varAxes;

        bool status = DataMgrUtils::GetExtents(
            dataMgr, timestep, varnames[i], varMinExts, varMaxExts);
        if (!status)
            continue;

        // Figure out which axes the variable coordinate extents
        // apply to.
        //
        status = DataMgrUtils::GetAxes(dataMgr, varnames[i], varAxes);
        assert(status);
        assert(varMinExts.size() == varAxes.size());

        for (int j = 0; j < varAxes.size(); j++) {
            int axis = varAxes[j];

            if (varMinExts[j] < tmpMinExts[axis]) {
                tmpMinExts[axis] = varMinExts[j];
            }
            if (varMaxExts[j] > tmpMaxExts[axis]) {
                tmpMaxExts[axis] = varMaxExts[j];
            }
        }
    }

    // tmp{Min,Max}Exts are always 3D vectors. If all variables are 2D
    // and live in same plane the returned {min,max}Exts should have
    // size of 2.
    //
    for (int i = 0; i < tmpMinExts.size(); i++) {
        if (tmpMinExts[i] != std::numeric_limits<double>::max()) {
            minExts.push_back(tmpMinExts[i]);
            maxExts.push_back(tmpMaxExts[i]);
            axes.push_back(i);
        }
    }
    return (true);
}

#ifdef DEAD
//Map corners of box to voxels.
void DataMgrUtils::mapBoxToVox(
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
    Grid *rg = dataMgr->GetVariable(timestep, varname, refLevel, lod, minexts, maxexts);
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

double DataMgrUtils::getVoxelSize(
    size_t ts, string varname, int refLevel, int dir) {
    DataMgr *dataMgr = GetActiveDataMgr();
    if (!dataMgr) {
        return (1.0);
    }

    //If refLevel is -1, use maximum refinement level
    //Obtain the variable at lowest refinement level, then convert to higher levels if needed
    //If dir is -1 get maximum side of voxel
    //If dir is -2 get minimum side of voxel
    Grid *rGrid = dataMgr->GetVariable(ts, varname, 0, 0);
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

#endif
