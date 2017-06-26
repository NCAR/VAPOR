//************************************************************************
//															*
//		     Copyright (C)  2006										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		regioneventrouter.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the RegionEventRouter class.
//		This class supports routing messages from the gui to the params
//		This is derived from the region tab
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100 4996)
#endif
#include <qdesktopwidget.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qlabel.h>
#include <QFileDialog>
#include <qlistwidget.h>
#include <QListWidgetItem>
#include "GL/glew.h"
#include "vapor/regionparams.h"
#include "ui_regionTab.h"

#include "MessageReporter.h"
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qlabel.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "TabManager.h"

#include "regioneventrouter.h"
#include "vapor/ControlExecutive.h"
#include "EventRouter.h"

using namespace VAPoR;

RegionEventRouter::RegionEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), Ui_RegionTab(), EventRouter(ce, RegionParams::GetClassType())
{
    setupUi(this);

    setIgnoreBoxSliderEvents(false);
}

RegionEventRouter::~RegionEventRouter() {}
/**********************************************************
 * Whenever a new regiontab is created it must be hooked up here
 ************************************************************/
void RegionEventRouter::hookUpTab()
{
    connect(boxSliderFrame, SIGNAL(extentsChanged()), this, SLOT(changeExtents()));
    connect(resetDomainButton, SIGNAL(clicked()), this, SLOT(setDomainVars()));
    connect(addVariableCombo, SIGNAL(activated(int)), this, SLOT(addDomainVar(int)));
    connect(removeVariableButton, SIGNAL(clicked()), this, SLOT(removeDomainVar()));

    connect(setFullRegionButton, SIGNAL(clicked()), this, SLOT(setMaxSize()));
    connect(copyBoxButton, SIGNAL(clicked()), this, SLOT(CopyBox()));

    // connect (loadRegionsButton, SIGNAL(clicked()), this, SLOT(LoadRegionExtents()));
    connect(saveRegionsButton, SIGNAL(clicked()), this, SLOT(saveRegionExtents()));
    // connect (adjustExtentsButton, SIGNAL(clicked()), this, SLOT(AdjustExtents()));
    // connect (refinementCombo, SIGNAL(activated(int)), this, SLOT(setNumRefinements(int)));
    // connect (variableCombo, SIGNAL(activated(int)), this, SLOT(setVarNum(int)));
    // connect (timestepSpin, SIGNAL(valueChanged(int)), this, SLOT(setTimeStep(int)));

    connect(LocalGlobal, SIGNAL(activated(int)), VizWinMgr::getInstance(), SLOT(setRgLocalGlobal(int)));
    connect(VizWinMgr::getInstance(), SIGNAL(enableMultiViz(bool)), LocalGlobal, SLOT(setEnabled(bool)));
}

void RegionEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Overview of the Region tab", "http://www.vapor.ucar.edu/docs/vapor-gui-help/region-tab#RegionOverview"));

    help.push_back(make_pair("Controlling the region extents", "http://www.vapor.ucar.edu/docs/vapor-gui-help/region-tab#RegionControl"));

    help.push_back(make_pair("Time-varying region extents", "http://www.vapor.ucar.edu/docs/vapor-gui-help/region-tab#TimeVaryingRegionExtents"));

    help.push_back(make_pair("Displaying information about current data in the region", "http://www.vapor.ucar.edu/docs/vapor-gui-help/region-tab#RegionInfoLoadedData"));

    help.push_back(make_pair("Scene extents and boxes", "http://www.vapor.ucar.edu/docs/vapor-gui-help/scene-extents-and-boxes"));

    help.push_back(make_pair("Copying box extents", "http://www.vapor.ucar.edu/docs/vapor-gui-help/copying-box-extents"));

    help.push_back(make_pair("Mouse control of region extents", "http://www.vapor.ucar.edu/docs/vapor-gui-help/mouse-modes"));

    help.push_back(make_pair("Tips for controlling region extents in the tab", "http://www.vapor.ucar.edu/docs/vapor-gui-help/control-box-extents-tab"));
}

/*********************************************************************************
 * Slots associated with RegionTab:
 *********************************************************************************/

void RegionEventRouter::setRegionTabTextChanged(const QString &) { SetTextChanged(true); }
void RegionEventRouter::_confirmText() {}

void RegionEventRouter::regionReturnPressed(void) { confirmText(); }

// Insert values from params into tab panel
//
void RegionEventRouter::_updateTab()
{
#ifdef DEAD
    cout << "RegionEventRouter::_updateTab() BLOCKED" << endl;
    return;
#endif

    size_t ts = GetCurrentTimeStep();

    RegionParams *rParams = (RegionParams *)GetActiveParams();

#ifdef DEAD
    domainVariableList->clear();
    vector<string> domainVars = RegionParams::GetDomainVariables();
    for (int i = 0; i < domainVars.size(); i++) { domainVariableList->addItem(QString::fromStdString(domainVars[i])); }

    DataStatus *dataStatus = _controlExec->getDataStatus();

    vector<double> minExts, maxExts;
    dataStatus->GetExtents(ts, minExts, maxExts);

    // setup the boxSlider frame
    vector<double> minBoxExts, maxBoxExts;
    rParams->GetBox()->GetExtents(minBoxExts, maxBoxExts);

    boxSliderFrame->setFullDomain(minExts, maxExts);
    boxSliderFrame->setBoxExtents(minBoxExts, maxBoxExts);
#endif
#ifdef DEAD
    string varname = rParams->GetDomainVariables()[0];
    int    maxreflevel = dataStatus->maxXFormPresent(varname, ts);
    boxSliderFrame->setNumRefinements(maxreflevel);
    boxSliderFrame->setVariableName(varname);
#endif
    boxSliderFrame->setNumRefinements(1);
    boxSliderFrame->setVariableName("N/A");

#ifdef DEAD
    // Calculate extents of the containing box
    double corners[8][3];
    rParams->GetBox()->calcLocalBoxCorners(corners, 0.f, -1);
    double dboxmin[3], dboxmax[3];
    size_t gridExts[6];
    for (int i = 0; i < 3; i++) {
        float mincrd = corners[0][i];
        float maxcrd = mincrd;
        for (int j = 0; j < 8; j++) {
            if (mincrd > corners[j][i]) mincrd = corners[j][i];
            if (maxcrd < corners[j][i]) maxcrd = corners[j][i];
        }
        if (mincrd < 0.) mincrd = 0.;
        if (maxcrd > fullSizes[i]) maxcrd = fullSizes[i];
        dboxmin[i] = mincrd;
        dboxmax[i] = maxcrd;
    }
    // Now convert to user coordinates

    minUserXLabel->setText(QString::number(minExts[0] + dboxmin[0]));
    minUserYLabel->setText(QString::number(minExts[1] + dboxmin[1]));
    minUserZLabel->setText(QString::number(minExts[2] + dboxmin[2]));
    maxUserXLabel->setText(QString::number(minExts[0] + dboxmax[0]));
    maxUserYLabel->setText(QString::number(minExts[1] + dboxmax[1]));
    maxUserZLabel->setText(QString::number(minExts[2] + dboxmax[2]));
    minUserXLabel->setText(QString::number(minExts[0]));
    minUserYLabel->setText(QString::number(minExts[1]));
    minUserZLabel->setText(QString::number(minExts[2]));
    maxUserXLabel->setText(QString::number(minExts[0]));
    maxUserYLabel->setText(QString::number(minExts[1]));
    maxUserZLabel->setText(QString::number(minExts[2]));
#endif

#ifdef DEAD
    // And convert these to grid coordinates.
    // BUG:  Use maxreflevel = 0 temporarily because of vdc performance bug on Windows:
    dataStatus->mapBoxToVox(rParams->GetBox(), varname, 0, 0, ts, gridExts);

    minGridXLabel->setText(QString::number(gridExts[0]));
    minGridYLabel->setText(QString::number(gridExts[1]));
    minGridZLabel->setText(QString::number(gridExts[2]));
    maxGridXLabel->setText(QString::number(gridExts[3]));
    maxGridYLabel->setText(QString::number(gridExts[4]));
    maxGridZLabel->setText(QString::number(gridExts[5]));
#endif
    minGridXLabel->setText(QString::number(-1));
    minGridYLabel->setText(QString::number(-1));
    minGridZLabel->setText(QString::number(-1));
    maxGridXLabel->setText(QString::number(-1));
    maxGridYLabel->setText(QString::number(-1));
    maxGridZLabel->setText(QString::number(-1));

    // Provide latlon box extents if available:
#ifdef DEAD
    if (_controlExec->GetDataMgr()->GetMapProjection().size() == 0) {
#endif
        minMaxLonLatFrame->hide();
#ifdef DEAD
    } else {
#endif
#ifdef DEAD
        double boxLatLon[4];

        boxLatLon[0] = minBoxExts[0];
        boxLatLon[1] = minBoxExts[1];
        boxLatLon[2] = maxBoxExts[0];
        boxLatLon[3] = maxBoxExts[1];

        if (dataStatus->convertLocalToLonLat((int)ts, boxLatLon, 2)) {
            minLonLabel->setText(QString::number(boxLatLon[0]));
            minLatLabel->setText(QString::number(boxLatLon[1]));
            maxLonLabel->setText(QString::number(boxLatLon[2]));
            maxLatLabel->setText(QString::number(boxLatLon[3]));
            minMaxLonLatFrame->show();
        } else {
            minMaxLonLatFrame->hide();
        }
    }
#endif

#ifdef DEAD
    if (rParams->IsLocal())
        LocalGlobal->setCurrentIndex(1);
    else
#endif
        LocalGlobal->setCurrentIndex(0);

    minMaxLonLatFrame->hide();

    relabel();
    setIgnoreBoxSliderEvents(false);
}

// Update the axes labels
//
void RegionEventRouter::relabel()
{
    {
        xUserLabel->setText("X");
        yUserLabel->setText("Y");
        zUserLabel->setText("Z");

        xVoxelLabel->setText("VoxX");
        yVoxelLabel->setText("VoxY");
        zVoxelLabel->setText("VoxZ");
    }
}

void RegionEventRouter::setMaxSize()
{
    confirmText();
    RegionParams *rParams = (RegionParams *)GetActiveParams();

#ifdef DEAD
    vector<double> minExt, maxExt;
    DataStatus *   dataStatus = _controlExec->getDataStatus();
    dataStatus->GetExtents(minExt, maxExt);

    rParams->GetBox()->SetExtents(minExt, maxExt);
    updateTab();
#endif
}

#ifdef DEAD
// Reinitialize region tab settings, session has changed
// Need to force the regionMin, regionMax to be OK.
void RegionEventRouter::_reinitTab(bool doOverride)
{
    int i;
    setIgnoreBoxSliderEvents(false);

    // Set up the combo boxes in the gui based on info in the session:
    #ifdef DEAD
    const vector<string> &varNames = _dataMgr->GetDataVarNames();
    #endif
    vector<string> varNames(1, "N/A");

    variableCombo->clear();
    addVariableCombo->clear();
    addVariableCombo->addItem("       Add Variable");
    for (i = 0; i < (int)varNames.size(); i++) {
        variableCombo->addItem(varNames[i].c_str());
        addVariableCombo->addItem(varNames[i].c_str());
    }

    #ifdef DEAD
    DataStatus *  dataStatus = _controlExec->getDataStatus();
    const double *fullDataExtents = dataStatus->getExtents(minExt, maxExt);
    int           mints = _dataStatus->getMinTimestep();
    int           maxts = _dataStatus->getMaxTimestep();

    timestepSpin->setMinimum(mints);
    timestepSpin->setMaximum(maxts);
    timestepSpin->setValue(mints);
    #endif
    /* set up refinement combo when we know the variable
    int numRefinements = currentDataMgr->GetNumTransforms();
    refinementCombo->setMaxCount(numRefinements+1);
    refinementCombo->clear();
    for (i = 0; i<= numRefinements; i++){
        refinementCombo->addItem(QString::number(i));
    }
    */
    if (VizWinMgr::getInstance()->getNumVisualizers() > 1)
        LocalGlobal->setEnabled(true);
    else
        LocalGlobal->setEnabled(false);
    // Set up the copy combos
    copyBoxFromCombo->clear();
    copyBoxToCombo->clear();
    _boxMapping.clear();

    #ifdef DEAD
    for (int i = 1; i <= _paramsMgr->GetNumParamsClasses(); i++) {
        string tag = _paramsMgr->GetTagFromType(i);
        assert(tag != "");
        Params *p = _paramsMgr->GetDefaultParams(tag);
        assert(p);
        if (!p->GetBox()) continue;
        QString pname = QString(p->getShortName().c_str());
        copyBoxFromCombo->addItem(pname);
        copyBoxToCombo->addItem(pname);
        _boxMapping.push_back(tag);
    }
    #endif
    setEnabled(true);
}
#endif

// Save undo/redo state when user grabs a rake handle
//
void RegionEventRouter::captureMouseDown(int)
{
    // If text has changed, will ignore it-- don't call confirmText()!
    //
    RegionParams *rParams = (RegionParams *)GetActiveParams();
    SetTextChanged(false);

#ifdef DEAD
    // Force a rerender, so we will see the selected face:
    VizWinMgr::getInstance()->refreshRegion(rParams);
#endif
}
void RegionEventRouter::captureMouseUp()
{
    // Update the tab:
    RegionParams *rParams = (RegionParams *)GetActiveParams();
    updateTab();

#ifdef DEAD
    // Force a rerender:
    VizWinMgr::getInstance()->refreshRegion(rParams);
#endif
}

void RegionEventRouter::loadRegionExtents()
{
#ifdef DEAD
    // load a list of
    // region extents.   The previous default box is used if the
    // list does not specify the extents at a timestep.
    confirmText();
    // Launch a file-open dialog
    QString filename = QFileDialog::getOpenFileName(this, "Specify file name for loading list of time-varying Region extents", ".", "Text files (*.txt)");
    // Check that user did specify a file:
    if (filename.isNull()) { return; }

    // Open the file:
    FILE *regionFile = fopen((const char *)filename.toLocal8Bit(), "r");
    if (!regionFile) { return; }
    // File is OK, save the state in command queue
    DataStatus *  dataStatus = _controlExec->getDataStatus();
    const double *fullDataExtents = dataStatus->getExtents(minExt, maxExt);
    RegionParams *rParams = (RegionParams *)GetActiveParams();

    const double *fullExtents = dataStatus->getLocalExtents();
    // Read the file

    int numregions = 0;

    while (1) {
        int   ts;
        float exts[6];
        int   numVals = fscanf(regionFile, "%d %g %g %g %g %g %g", &ts, exts, exts + 1, exts + 2, exts + 3, exts + 4, exts + 5);

        if (numVals < 7) {
            if (numVals > 0 && numregions > 0) {}
            break;
        }
        numregions++;
        bool ok = true;
        // Force it to be valid:
        for (int i = 0; i < 3; i++) {
            if (exts[i] < 0.) {
                ok = false;
                exts[i] = 0.;
            }
            if (exts[i + 3] > fullExtents[i + 3] - fullExtents[i]) {
                ok = false;
                exts[i + 3] = fullExtents[i + 3] - fullExtents[i];
            }
            if (exts[i] > exts[i + 3]) {
                ok = false;
                exts[i + 3] = exts[i];
            }
        }
        if (!ok) {}
        rParams->insertTime(ts);
        double dbexts[6];
        for (int i = 0; i < 6; i++) dbexts[i] = exts[i];
        rParams->GetBox()->SetLocalExtents(dbexts, rParams, ts);
    }
    if (numregions == 0) {
        fclose(regionFile);
        return;
    }

    fclose(regionFile);
    updateTab();

#endif
}
void RegionEventRouter::saveRegionExtents()
{
#ifdef DEAD
    // save the list of
    // region extents.
    // Timesteps that have not been modified do not get saved.
    confirmText();
    RegionParams *rParams = (RegionParams *)GetActiveParams();
    // Are there any extents to write?
    if (!rParams->extentsAreVarying()) { return; }
    // Launch a file-open dialog
    QString filename = QFileDialog::getSaveFileName(this, "Specify file name for saving list of current time-varying Local Region extents", ".", "Text files (*.txt)");
    // Check that user did specify a file:
    if (filename.isNull()) { return; }

    // Open the file:
    FILE *regionFile = fopen((const char *)filename.toLocal8Bit(), "w");
    if (!regionFile) { return; }
    const vector<double> extents = rParams->GetAllExtents();
    const vector<long>   times = rParams->GetTimes();
    for (int i = 1; i < times.size(); i++) {
        int   timestep = times[i];
        float exts[6];
        for (int j = 0; j < 6; j++) exts[j] = extents[6 * i + j];
        int rc = fprintf(regionFile, "%d %g %g %g %g %g %g\n", timestep, exts[0], exts[1], exts[2], exts[3], exts[4], exts[5]);
        if (rc < 7) {
            fclose(regionFile);
            return;
        }
    }

    fclose(regionFile);
#endif
}
void RegionEventRouter::adjustExtents()
{
    confirmText();

#ifdef DEAD
    // Get bounds from DataStatus:
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    size_t     ts = GetCurrentTimeStep();

    if (!rParams->insertTime(ts)) {    // no change

        return;
    }
#endif
}
// Make region match probe.  Responds to button in region panel
void RegionEventRouter::CopyBox()
{
#ifdef DEAD
    confirmText();

    int viznum = GetActiveVizIndex();
    if (viznum < 0) return;
    string fromParamsTag = _boxMapping[copyBoxFromCombo->currentIndex()];
    string toParamsTag = _boxMapping[copyBoxToCombo->currentIndex()];
    if (toParamsTag == fromParamsTag) {
        MessageReporter::errorMsg("Source and Target of extents copy cannot be the same");
        return;
    }

    Params *pFrom = _paramsMgr->GetCurrentParams(viznum, fromParamsTag);
    Params *pTo = _paramsMgr->GetCurrentParams(viznum, toParamsTag);
    assert(pFrom && pTo);
    Command *cmd = Command::CaptureStart(pTo, "copy box extents");
    double   toExtents[6], fromExtents[6];
    pFrom->GetBox()->GetLocalExtents(fromExtents);
    pTo->GetBox()->GetLocalExtents(toExtents);
    // Check if the source is 2D;  If the target is not 2D then don't alter its vertical extents
    if (fromExtents[2] == fromExtents[5] && toExtents[2] != toExtents[5]) {
        fromExtents[2] = toExtents[2];
        fromExtents[5] = toExtents[5];
    }
    // Check if target is 2D, and source is not 2D don't change its extents
    else if (toExtents[2] == toExtents[5] && fromExtents[2] != fromExtents[5]) {
        fromExtents[2] = toExtents[2];
        fromExtents[5] = toExtents[5];
    }
    pTo->GetBox()->SetLocalExtents(fromExtents, pTo);
    Command::CaptureEnd(cmd, pTo);

#endif
}
void RegionEventRouter::setDomainVars()
{
#ifdef DEAD
    // Construct a vector from the entries in the domain variable list
    vector<string> varnames;
    for (int i = 0; i < domainVariableList->count(); i++) {
        QListWidgetItem *item = domainVariableList->item(i);
        string           s = item->text().toStdString();
        varnames.push_back(s);
    }
    RegionParams::SetDomainVariables(varnames);
#endif
}
void RegionEventRouter::addDomainVar(int indx)
{
#ifdef DEAD
    if (indx <= 0) return;
    QString varname = addVariableCombo->itemText(indx);
    // Check for duplication
    for (int i = 0; i < domainVariableList->count(); i++) {
        QListWidgetItem *item = domainVariableList->item(i);
        QString          var = item->text();
        if (varname != var) continue;
        addVariableCombo->setCurrentIndex(0);
        return;
    }
    domainVariableList->addItem(varname);
    addVariableCombo->setCurrentIndex(0);
#endif
}
void RegionEventRouter::removeDomainVar()
{
#ifdef DEAD
    int row = domainVariableList->currentRow();
    domainVariableList->takeItem(row);
#endif
}
void RegionEventRouter::changeExtents()
{
#ifdef DEAD
    confirmText();

    RegionParams *rParams = (RegionParams *)GetActiveParams();
    double        newExts[6];
    boxSliderFrame->getBoxExtents(newExts);
    Box *bx = rParams->GetBox();

    // Get bounds from DataStatus:
    size_t timeStep = GetCurrentTimeStep();

    // convert newExts (in user coords) to local extents, by subtracting time-varying extents origin
    DataStatus *dataStatus = _controlExec->getDataStatus();

    vector<double> minExts, maxExts;
    dataStatus->GetExtents(timeStep, minExts, maxExts);
    bx->SetExtents(minExts, maxExts);
    updateTab();
#endif
}
void RegionEventRouter::setCenter(const double newCenter[3])
{
#ifdef DEAD
    RegionParams *rParams = (RegionParams *)GetActiveParams();

    DataStatus *dataStatus = _controlExec->getDataStatus();

    vector<double> minExts, maxExts;
    dataStatus->GetExtents(ts, minExts, maxExts);

    vector<double> minBoxExts, maxBoxExts;
    rParams->GetBox()->GetExtents(minBoxExts, maxBoxExts));

    double newSize[3];
    // Check the new region extents, make sure they fit inside the domain
    for (int i = 0; i < 3; i++) {
        newSize[i] = (locExtents[i + 3] - locExtents[i]);
        double maxSize = abs(domainExtents[i] - newCenter[i]);
        if (maxSize > abs(domainExtents[i + 3] - newCenter[i])) maxSize = abs(domainExtents[i + 3] - newCenter[i]);
        if (newSize[i] > maxSize) newSize[i] = maxSize;
        // Move the extents to the new center:
        locExtents[i] = newCenter[i] - newSize[i];
        locExtents[i + 3] = newCenter[i] + newSize[i];
    }
    rParams->GetBox()->SetLocalExtents(locExtents, rParams);
#endif
}
