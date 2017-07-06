//
//		     Copyright (C)  2017
//     University Corporation for Atmospheric Research
//		     All Rights Reserved
//
//////////////////////////////////////////////////////////////////////////
//
//	File:		RenderEventRouter.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		January 2017
//
//	Description:	Implements the (pure virtual) RenderEventRouter class.
//		This class supports routing messages from the gui to the params
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <QFileDialog>
#include <vapor/GetAppPath.h>
#include <vapor/DataMgrUtils.h>
#include "MessageReporter.h"
#include "MappingFrame.h"
#include "RenderEventRouter.h"

using namespace VAPoR;

RenderParams *RenderEventRouter::GetActiveParams() const
{
    assert(!_instName.empty());

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    string winName, dataSetName, paramsType;
    bool   status = paramsMgr->RenderParamsLookup(_instName, winName, dataSetName, paramsType);
    assert(status);

    string renderType = RendererFactory::Instance()->GetRenderClassFromParamsClass(paramsType);

    return (_controlExec->GetRenderParams(winName, dataSetName, renderType, _instName));
}

DataMgr *RenderEventRouter::GetActiveDataMgr() const
{
    assert(!_instName.empty());

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    string winName, dataSetName, paramsType;

    bool status = paramsMgr->RenderParamsLookup(_instName, winName, dataSetName, paramsType);
    assert(status);

    DataStatus *dataStatus = _controlExec->getDataStatus();
    DataMgr *   dataMgr = dataStatus->GetDataMgr(dataSetName);
    assert(dataMgr);

    return (dataMgr);
}

//
// Obtain the current valid histogram.  if mustGet is false, don't build a
// new one.
// Boolean flag is only used by isoeventrouter version
//
Histo *RenderEventRouter::GetHistogram(bool mustGet, bool)
{
    RenderParams *rParams = GetActiveParams();

    if (_currentHistogram && !mustGet) return _currentHistogram;
    if (!mustGet) return 0;
    string          varname = rParams->GetVariableName();
    MapperFunction *mapFunc = rParams->MakeMapperFunc(varname);
    if (!mapFunc) return 0;
    if (_currentHistogram) delete _currentHistogram;

    _currentHistogram = new Histo(256, mapFunc->getMinMapValue(), mapFunc->getMaxMapValue());
    RefreshHistogram();
    return _currentHistogram;
}

void RenderEventRouter::RefreshHistogram()
{
    RenderParams *rParams = GetActiveParams();

    size_t timeStep = GetCurrentTimeStep();

    string varname = rParams->GetVariableName();
    if (varname.empty()) return;

#ifdef DEAD
    if (tParams->doBypass(timeStep)) return;
#endif
    float minRange = rParams->MakeMapperFunc(varname)->getMinMapValue();
    float maxRange = rParams->MakeMapperFunc(varname)->getMaxMapValue();
    if (!_currentHistogram)
        _currentHistogram = new Histo(256, minRange, maxRange);
    else
        _currentHistogram->reset(256, minRange, maxRange);
    StructuredGrid *histoGrid;
    int             actualRefLevel = rParams->GetRefinementLevel();
    int             lod = rParams->GetCompressionLevel();
    vector<double>  minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);

    DataMgr *dataMgr = GetActiveDataMgr();
    int      rc = DataMgrUtils::GetGrids(dataMgr, timeStep, varname, minExts, maxExts, true, &actualRefLevel, &lod, &histoGrid);

    if (rc) return;
    histoGrid->SetInterpolationOrder(0);
    float                    v;
    StructuredGrid *         rg_const = (StructuredGrid *)histoGrid;
    StructuredGrid::Iterator itr;

    for (itr = rg_const->begin(); itr != rg_const->end(); ++itr) {
        v = *itr;
        if (v == histoGrid->GetMissingValue()) continue;
        _currentHistogram->addToBin(v);
    }

    delete histoGrid;
}

// Calculate histogram for a planar slice of data, such as in
// the probe or the isolines.
//
void RenderEventRouter::CalcSliceHistogram(int ts, Histo *histo)
{
#ifdef DEAD

    #ifdef DEAD
    if (rParams->doBypass(ts)) return;
    #endif

    int            actualRefLevel = rParams->GetRefinementLevel();
    int            lod = rParams->GetCompressionLevel();
    vector<double> minExts, maxExts;
    _dataStatus->GetExtents((size_t)ts, minExts, maxExts);
    StructuredGrid *probeGrid;
    vector<string>  varnames;
    varnames.push_back(rParams->GetVariableName());
    double extents[6];

    rParams->GetBox()->calcContainingBoxExtents(extents, true);

    for (int i = 0; i < 3; i++) {
        extents[i] += minExts[i];
        extents[i + 3] += minExts[i];
    }

    int rc = Renderer::getGrids(_dataMgr, ts, varnames, extents, &actualRefLevel, &lod, &probeGrid);

    if (rc) { return; }

    probeGrid->SetInterpolationOrder(0);

    double transformMatrix[12];
    // Set up to transform from probe into volume:
    rParams->GetBox()->buildLocalCoordTransform(transformMatrix, 0., -1);

    // Get the data dimensions (at this resolution):
    size_t dataSize[3];
    // Start by initializing extents

    probeGrid->GetDimensions(dataSize);

    const double *fullSizes = _dataStatus->getFullSizes();
    // Now calculate the histogram
    //
    // For each voxel, map it into the volume.
    // We first map the coords in the probe to the volume.
    // Then we map the volume into the region provided by dataMgr

    double probeCoord[3];
    double dataCoord[3];

    float extExtents[6];    // Extend extents 1/2 voxel on each side so no bdry issues.
    for (int i = 0; i < 3; i++) {
        float mid = (fullSizes[i]) * 0.5;
        float halfExtendedSize = fullSizes[i] * 0.5 * (1.f + dataSize[i]) / (float)(dataSize[i]);
        extExtents[i] = mid - halfExtendedSize;
        extExtents[i + 3] = mid + halfExtendedSize;
    }

    // To determine the grid resolution to histogram, find out the change
    // in grid coordinate along each edge of the probe box.  Map each corner of the box to grid coordinates at current refinement level:
    // First map them to user coordinates, then convert these user coordinates to grid coordinates.

    // icor will contain the integer coordinates of each of the 8 corners of the probe box.
    int icor[8][3];
    for (int cornum = 0; cornum < 8; cornum++) {
        // coords relative to (-1,1)
        probeCoord[2] = -1.f + 2.f * (float)(cornum / 4);
        probeCoord[1] = -1.f + 2.f * (float)((cornum / 2) % 2);
        probeCoord[0] = -1.f + 2.f * (float)(cornum % 2);
        // Then transform to values in data
        vtransform(probeCoord, transformMatrix, dataCoord);
        // Then get array coords.
        // icor[k][dir] indicates the integer (data grid) coordinate of corner k along data grid axis dir
        for (int i = 0; i < 3; i++) { icor[cornum][i] = (size_t)(0.5f + (float)dataSize[i] * dataCoord[i] / fullSizes[i]); }
    }
    // Find the resolution along each axis of the probe
    // for each probe axis direction,  find the difference of each of the integer coordinates of the probe, across the probe in that direction.
    // Because the data is layered, try all 4 edges for each direction.  The various edges are identified by cornum increasing by 4,1,or 2 starting at
    //(0,1,2,3), (0,2,4,6), (0,1,4,5)
    // Once the fastest varying coordinate is known, subdivide that axis to match the resolution of that coordinate.
    // difference of a coordinate across a probe-axis direction is determined by the change in data-grid coordinates going from one face to the
    // opposite face of the probe.

    int gridRes[3] = {0, 0, 0};
    int difmax = -1;
    for (int dir = 0; dir < 3; dir++) {
        // four differences in the data-grid z direction, for data grid coordinate dir
        difmax = Max(difmax, abs(icor[0][dir] - icor[4][dir]));
        difmax = Max(difmax, abs(icor[1][dir] - icor[5][dir]));
        difmax = Max(difmax, abs(icor[2][dir] - icor[6][dir]));
        difmax = Max(difmax, abs(icor[3][dir] - icor[7][dir]));
    }
    gridRes[2] = difmax + 1;
    difmax = -1;
    for (int dir = 0; dir < 3; dir++) {
        // four differences in the data-grid y direction
        difmax = Max(difmax, abs(icor[0][dir] - icor[2][dir]));
        difmax = Max(difmax, abs(icor[1][dir] - icor[3][dir]));
        difmax = Max(difmax, abs(icor[4][dir] - icor[6][dir]));
        difmax = Max(difmax, abs(icor[5][dir] - icor[7][dir]));
    }
    gridRes[1] = difmax + 1;
    difmax = -1;
    for (int dir = 0; dir < 3; dir++) {
        // four differences in the data-grid x direction
        difmax = Max(difmax, abs(icor[0][dir] - icor[1][dir]));
        difmax = Max(difmax, abs(icor[2][dir] - icor[3][dir]));
        difmax = Max(difmax, abs(icor[4][dir] - icor[5][dir]));
        difmax = Max(difmax, abs(icor[6][dir] - icor[7][dir]));
    }
    gridRes[0] = difmax + 1;

    // Now gridRes represents the number of samples to take in each direction across the probe
    // Use the region reader to calculate coordinates in volume

    // Loop over pixels in texture.  Pixel centers map to edges of probe
    for (int iz = 0; iz < gridRes[2]; iz++) {
        if (gridRes[2] == 1)
            probeCoord[2] = 0.;
        else
            probeCoord[2] = -1. + 2. * iz / (float)(gridRes[2] - 1);
        for (int iy = 0; iy < gridRes[1]; iy++) {
            // Map iy to a value between -1 and 1
            if (gridRes[1] == 1)
                probeCoord[1] = 0.f;
            else
                probeCoord[1] = -1.f + 2.f * (float)iy / (float)(gridRes[1] - 1);
            for (int ix = 0; ix < gridRes[0]; ix++) {
                if (gridRes[0] == 1)
                    probeCoord[0] = 0.5f;
                else
                    probeCoord[0] = -1.f + 2.f * (float)ix / (float)(gridRes[0] - 1);
                vtransform(probeCoord, transformMatrix, dataCoord);
                // find the coords that the texture maps to
                // probeCoord is the coord in the probe, dataCoord is in data volume
                bool dataOK = true;
                for (int i = 0; i < 3; i++) {
                    if (dataCoord[i] < extExtents[i] || dataCoord[i] > extExtents[i + 3]) dataOK = false;
                    dataCoord[i] += minExts[i];    // Convert to user coordinates.
                }
                float varVal;
                if (dataOK) {    // find the coordinate in the data array

                    varVal = probeGrid->GetValue(dataCoord[0], dataCoord[1], dataCoord[2]);
                    if (varVal == probeGrid->GetMissingValue()) dataOK = false;
                }
                if (dataOK) {
                    // Add this sample to the histogram
                    histo->addToBin(varVal);
                }
                // otherwise ignore this sample...
            }    // End loop over ix
        }        // End loop over iy
    }            // End loop over iz;

    _dataMgr->UnlockGrid(probeGrid);
    delete probeGrid;
#endif
}

void RenderEventRouter::setEditorDirty()
{
    RenderParams *rParams = GetActiveParams();

    MappingFrame *mp = getMappingFrame();
    if (!mp) return;

    // mp->updateTab();
    mp->Update(rParams);

#ifdef DEAD
    if (rParams->GetMapperFunc()) p->GetMapperFunc()->setParams(p);
    getMappingFrame()->setMapperFunction(p->GetMapperFunc());
    if (getColorbarFrame()) getColorbarFrame()->setParams(p);
    getMappingFrame()->updateParams();
    if (_dataStatus->getNumActiveVariables()) {
        const std::string &varname = p->GetVariableName();
        getMappingFrame()->setVariableName(varname);
    } else {
        getMappingFrame()->setVariableName("N/A");
    }
    getMappingFrame()->update();
#endif
}

float RenderEventRouter::CalcCurrentValue(const double point[3])
{
    RenderParams *rParams = GetActiveParams();

    if (!rParams->IsEnabled()) return 0.f;

    size_t timeStep = GetCurrentTimeStep();

#ifdef DEAD
    if (rParams->doBypass(timeStep)) return _OUT_OF_BOUNDS;
#endif

    vector<double> minExts, maxExts;
    for (int i = 0; i < 3; i++) {
        minExts.push_back(point[i]);
        maxExts.push_back(point[i]);
    }

    string varname = rParams->GetVariableName();
    if (varname.empty()) return (0.0);

    vector<string> varnames;
    varnames.push_back(varname);

    StructuredGrid *grid;
    // Get the data dimensions (at current resolution):

    int numRefinements = rParams->GetRefinementLevel();
    int lod = rParams->GetCompressionLevel();

    DataStatus *dataStatus = _controlExec->getDataStatus();
    int         rc = dataStatus->getGrids(timeStep, varnames, minExts, maxExts, &numRefinements, &lod, &grid);

#ifdef DEAD
    if (rc < 0) return _OUT_OF_BOUNDS;
#endif
    float varVal = (grid)->GetValue(point[0], point[1], point[2]);

    delete grid;
    return varVal;
}

void RenderEventRouter::updateTab()
{
    RenderParams *rParams = GetActiveParams();

    // If the Params is not valid do not proceed.
    if (!rParams) return;

    DataMgr *dataMgr = _controlExec->GetDataMgr();
    if (!dataMgr) return;

    EventRouter::updateTab();
}

void RenderEventRouter::fileSaveTF()
{
    RenderParams *rParams = GetActiveParams();

    // Launch a file save dialog, open resulting file
    GUIStateParams *p = GetStateParams();
    string          path = p->GetCurrentTFPath();

    QString s = QFileDialog::getSaveFileName(0, "Choose a filename to save the transfer function", path.c_str(), "Vapor 3 Transfer Functions (*.tf3)");
    // Did the user cancel?
    if (s.length() == 0) return;
    // Force the name to end with .tf3
    if (!s.endsWith(".tf3")) { s += ".tf3"; }

    string varname = rParams->GetVariableName();

    TransferFunction *tf = rParams->GetTransferFunc(varname);
    if (!tf) {
        tf = rParams->MakeTransferFunc(varname);
        assert(tf);
    }

    int rc = tf->SaveToFile(s.toStdString());
    if (rc < 0) {
        QString str("Failed to write output file: \n");
        str += s;
        MessageReporter::errorMsg((const char *)str.toAscii());
        return;
    }
}

void RenderEventRouter::loadInstalledTF(string varname)
{
    // Get the path from the environment:
    vector<string> paths;
    paths.push_back("palettes");
    string palettes = GetAppPath("VAPOR", "share", paths);

    QString installPath = palettes.c_str();
    fileLoadTF(varname, (const char *)installPath.toAscii(), false);
}

void RenderEventRouter::loadTF(string varname)
{
    // Ignore TF's in session, for now.

    GUIStateParams *p = GetStateParams();
    string          path = p->GetCurrentTFPath();

    fileLoadTF(varname, p->GetCurrentTFPath().c_str(), true);
}

void RenderEventRouter::fileLoadTF(string varname, const char *startPath, bool savePath)
{
    RenderParams *rParams = GetActiveParams();

    // Open a file load dialog
    QString s = QFileDialog::getOpenFileName(0, "Choose a transfer function file to open", startPath, "Vapor 3 Transfer Functions (*.tf3)");
    // Null string indicates nothing selected.
    if (s.length() == 0) return;
    // Force the name to end with .tf3
    if (!s.endsWith(".tf3")) { s += ".tf3"; }

    // Start the history save:
    confirmText();
    TransferFunction *tf = rParams->GetTransferFunc(varname);
    if (!tf) {
        tf = rParams->MakeTransferFunc(varname);
        assert(tf);
    }

    int rc = tf->LoadFromFile(s.toStdString());
    if (rc < 0) {
        QString str("Error loading transfer function. /nFailed to convert input file: \n ");
        str += s;
        MessageReporter::errorMsg((const char *)str.toAscii());
    }

#ifdef DEAD
    // Remember the path to the file:
    if (savePath) PathParams::SetCurrentTFPath(s.toStdString());
    setEditorDirty(rParams);
#endif
}
