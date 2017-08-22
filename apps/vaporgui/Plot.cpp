//************************************************************************
//																	  *
//		   Copyright (C)  2016										*
//	 University Corporation for Atmospheric Research				  *
//		   All Rights Reserved										*
//																	  *
//************************************************************************/
//
//  File:	   Plot.cpp
//
//  Author:	 Scott Pearse
//		  National Center for Atmospheric Research
//		  PO 3000, Boulder, Colorado
//
//  Date:	   September 2016
//
//  Description:	Implements the matPlotLib Plot class.
//

#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#define FIXED

//#include <Python.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <limits>
#include <algorithm>
#include <QCheckBox>
#include <QFileDialog>
#include <vapor/GetAppPath.h>
#include <vapor/DataMgr.h>
#include "Plot.h"
#include <vapor/MyPython.h>
#include "RangeController.h"

#define NPY_NO_DEPRECATED_API NPY_1_8_API_VERSION
#include <numpy/ndarrayobject.h>

using namespace VAPoR;
using namespace Wasp;

namespace {

//
// Commented Python Functions:
//
//  PyObject *buildNumpyArray(const vector <float> &vec) const;
//  void getSliders(
//  QObject*& sender, QComboBox*& qcb, SpaceSSC*& x, SpaceSSC*& y,
//  SpaceSSC*& z
//);
//

//
// Params functions needed:
//
// 1061: error: no member named 'getActiveProbeParams' in 'VAPoR::VizWinMgr'
// 1062: error: no member named 'getActiveTwoDDataParams' in 'VAPoR::VizWinMgr'
// 1063: error: no member named 'getActiveIsolineParams' in 'VAPoR::VizWinMgr'
// 1070: error: member access into incomplete type 'VAPoR::RenderParams'
// 1075: error: member access into incomplete type 'VAPoR::RenderParams'

//
// Changes to DataMgr in 3.0
// GetNumTimeSteps() now takes a varname argument.  We will need to modify statistics
// to show data on a more dynamic time range.  See function initTimes().
//
// Need access to a new grid class instead of RegularGrid that provides GetValue()
// and GetMissingValue() function calls.

// Get path to write temporary image file. File path will be in user's home
// directory. Better to use cross-platform tmp directory :-(
//
string tmpImgFilePath() {
    QString qhome = QDir::homePath();
    assert(!qhome.isEmpty()); // Qt API guarantees an absolute home path

    qhome.append("/.vaporguitmp.png");
    qhome = QDir::toNativeSeparators(qhome);

    return (qhome.toStdString());
}

bool fexists(string filename) {
    ifstream ifs(filename.c_str());
    return ifs.good();
}

}; // namespace

// Static member variable used to ensure Python only gets initialized
// once. Python doesn't seem to be capable of proper cleanup :-(
//
bool Plot::_isInitializedPython = false;

// Slurp the python script that will be used to plot the data
//
// TODO: Search for user-defined script in home directory
//
string Plot::readPlotScript() const {

    // Get path to python scripts
    //

    //
    // First look in users home directory for .plot1D.py
    //
    QString qhome = QDir::homePath();
    assert(!qhome.isEmpty()); // Qt API guarantees an absolute home path
    qhome.append("/.plot1D.py");
    qhome = QDir::toNativeSeparators(qhome); // Convert to native OS path

    string path = qhome.toStdString();
    if (!fexists(path))
        path = "";

    // If no script in home directory get the system provided script
    //
    if (path.empty()) {
        vector<string> paths;
        paths.push_back("python");
        paths.push_back("plot1D.py");
        path = GetAppPath("VAPOR", "share", paths);
    }

    //
    // Read the file
    //
    ifstream in;
    in.open(path.c_str());
    if (!in) {
        errReport("Failed to open file " + path);
        return ("");
    }

    std::ostringstream contents;
    contents << in.rdbuf();
    if (!in) {
        errReport("Failed to open file " + path);
        return ("");
    }

    in.close();

    return (contents.str());
}

Plot::Plot(QWidget *parent) : QDialog(parent), Ui_PlotWindow() {

    _dm = NULL;
    _errMsg = NULL;
    _vwm = NULL;
    _plotDialog = NULL;
    _plotLabel = NULL;
    _plotLayout = NULL;
    _plotButton = NULL;
    _plotImage = NULL;

    _cRatio = -1;
    _refLevel = -1;

    _spaceTimeRange = NULL;
    _timeTimeRange = NULL;
    _spaceXRange = NULL;
    _spaceYRange = NULL;
    _spaceZRange = NULL;
    _timeXRange = NULL;
    _timeYRange = NULL;
    _timeZRange = NULL;

    _timeTimeMinSlider = NULL;
    _timeTimeMaxSlider = NULL;
    _timeXSlider = NULL;
    _timeYSlider = NULL;
    _timeZSlider = NULL;

    _spaceTimeSlider = NULL;
    _spaceP1XSlider = NULL;
    _spaceP2XSlider = NULL;
    _spaceP1YSlider = NULL;
    _spaceP2YSlider = NULL;
    _spaceP1ZSlider = NULL;
    _spaceP2ZSlider = NULL;

    _spaceP1XLineEdit = NULL;
    _spaceP1YLineEdit = NULL;
    _spaceP1ZLineEdit = NULL;
    _spaceP2XLineEdit = NULL;
    _spaceP2YLineEdit = NULL;
    _spaceP2ZLineEdit = NULL;

    _spaceP1XCell = NULL;
    _spaceP1YCell = NULL;
    _spaceP1ZCell = NULL;
    _spaceP2XCell = NULL;
    _spaceP2YCell = NULL;
    _spaceP2ZCell = NULL;

    _spaceP1XMinLabel = NULL;
    _spaceP1XMaxLabel = NULL;
    _spaceP2XMinLabel = NULL;
    _spaceP2XMaxLabel = NULL;
    _spaceP1YMinLabel = NULL;
    _spaceP1YMaxLabel = NULL;
    _spaceP2YMinLabel = NULL;
    _spaceP2YMaxLabel = NULL;
    _spaceP1ZMinLabel = NULL;
    _spaceP1ZMaxLabel = NULL;
    _spaceP2ZMinLabel = NULL;
    _spaceP2ZMaxLabel = NULL;

    _timeXMinLabel = NULL;
    _timeXMaxLabel = NULL;
    _timeYMinLabel = NULL;
    _timeYMaxLabel = NULL;
    _timeZMinLabel = NULL;
    _timeZMaxLabel = NULL;
    _timeTimeMinLabel = NULL;
    _timeTimeMaxLabel = NULL;

    _triggeredByFriend = false;
    _isInitialized = false;

    //
    // Install event filter on edits/sliders
    // to set params when they're adjusted
    //
    _spaceP1XSlider->installEventFilter(this);
    _spaceP2XSlider->installEventFilter(this);
    _spaceP1YSlider->installEventFilter(this);
    _spaceP2YSlider->installEventFilter(this);
    _spaceP1ZSlider->installEventFilter(this);
    _spaceP2ZSlider->installEventFilter(this);

    _spaceP1XLineEdit->installEventFilter(this);
    _spaceP2XLineEdit->installEventFilter(this);
    _spaceP1YLineEdit->installEventFilter(this);
    _spaceP2YLineEdit->installEventFilter(this);
    _spaceP1ZLineEdit->installEventFilter(this);
    _spaceP2ZLineEdit->installEventFilter(this);

    _spaceTimeSlider->installEventFilter(this);
    _spaceTimeLineEdit->installEventFilter(this);

    _timeXSlider->installEventFilter(this);
    _timeYSlider->installEventFilter(this);
    _timeZSlider->installEventFilter(this);
    _timeXLineEdit->installEventFilter(this);
    _timeYLineEdit->installEventFilter(this);
    _timeZLineEdit->installEventFilter(this);
    _timeTimeMinSlider->installEventFilter(this);
    _timeTimeMaxLineEdit->installEventFilter(this);
    _timeTimeMinLineEdit->installEventFilter(this);
    _timeTimeMaxSlider->installEventFilter(this);

    //
    // Construct the GUI
    //
    setupUi(this);
    initTables();
    initConstCheckboxes();
    initPlotDlg();
    setWindowTitle("Plot");

    plotButton->setAutoDefault(false);
    plotButton->setDefault(false);

    connect(plotButton, SIGNAL(pressed()), this, SLOT(go()));
    connect(addVarCombo, SIGNAL(activated(int)), this, SLOT(newVarAdded(int)));
    connect(removeVarCombo, SIGNAL(activated(int)), this, SLOT(removeVar(int)));
#ifdef DEAD
    connect(timeCopyCombo, SIGNAL(activated(int)), this, SLOT(getPointFromRenderer()));
    connect(spaceP1CopyCombo, SIGNAL(activated(int)), this, SLOT(getPointFromRenderer()));
    connect(spaceP2CopyCombo, SIGNAL(activated(int)), this, SLOT(getPointFromRenderer()));
#endif
    connect(refCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(refinementChanged(int)));
    connect(cRatioCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(cRatioChanged(int)));
    for (int i = 0; i < 4; i++) {
        connect(_spaceCheckBoxes[i], SIGNAL(stateChanged(int)), this, SLOT(constCheckboxChanged(int)));
        if (i < 3) {
            connect(_timeCheckBoxes[i], SIGNAL(stateChanged(int)), this, SLOT(constCheckboxChanged(int)));
        }
    }
}

Plot::~Plot() {
    if (_errMsg)
        delete _errMsg;

    if (_plotImage)
        delete _plotImage;

    if (_spaceXRange) {
        _spaceXRange->deleteObservers();
        delete _spaceXRange;
        _spaceXRange = NULL;
    }

    if (_spaceYRange) {
        _spaceYRange->deleteObservers();
        delete _spaceYRange;
        _spaceYRange = NULL;
    }

    if (_spaceZRange) {
        _spaceZRange->deleteObservers();
        delete _spaceZRange;
        _spaceZRange = NULL;
    }

    if (_timeTimeRange) {
        _timeTimeRange->deleteObservers();
        delete _timeTimeRange;
        _timeTimeRange = NULL;
    }

    if (_timeXRange) {
        _timeXRange->deleteObservers();
        delete _timeXRange;
        _timeXRange = NULL;
    }

    if (_timeYRange) {
        _timeYRange->deleteObservers();
        delete _timeYRange;
        _timeYRange = NULL;
    }

    if (_timeZRange) {
        _timeZRange->deleteObservers();
        delete _timeZRange;
        _timeZRange = NULL;
    }
}

void Plot::reject() {
    if (_plotDialog) {
        _plotDialog->close();
    }
    QDialog::reject();
}

// One-time initialization. Stuff that can't go in constructor because
// it's not no-fail
//
int Plot::init() {

    // Strictly one-time
    //
    if (_isInitialized)
        return (0);

    // For error popups
    //
    _errMsg = new pErrMsg;

    // Path for writing temporary image files
    //
    // TODO: Remove tmp image file when done
    //
    _defaultImageLocation = tmpImgFilePath();

    // One-time python initialization
    //
    int rc = initPython();
    if (rc < 0)
        return (-1);

    _isInitialized = true;
    return (0);
}

// NOT One-time initialization!!! This method is called every time the
// plotting window is launched by vaporgui.
//
// TODO: Handle changes to dm and possibly vwm. I.e. What happens when
// a new data set is loaded? Probably need to make Initialize two
// methods: one to launch the window, and one to be called *only* when
// dm and vwm change
//
//void Plot::Initialize(DataMgr* dm, VizWinMgr* vwm) {
void Plot::Initialize(ControlExec *ce, VizWinMgr *vwm) {
    assert(vwm != NULL);
    _vwm = vwm;

    assert(ce != NULL);
    _controlExec = ce;
    DataStatus *ds = _controlExec->getDataStatus();
    string dm = ds->GetDataMgrNames()[0];
    _dm = ds->GetDataMgr(dm);
    assert(_dm != NULL);

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    _params = (PlotParams *)paramsMgr->GetParams("PlotParams");

    // Do one-time initialization. The Initialize() method is called
    // every time Plot is launched.
    //
    int rc = init();
    if (rc < 0)
        return;

    // Initialize GUI with data-dependent variables
    //
    initVariables();
    initTimes();
    initExtents(0, _extents);
    initCRatios();
    initRefinement();

    // More GUI construction. Should probably go in constructor but it
    // is data dependent.
    //
    initSSCs();

    applyParams();

    show();
    raise();
    activateWindow();
}

void Plot::applyParams() {
    vector<double> minSpace, maxSpace;
    minSpace = _params->GetSpaceMinExtents();
    maxSpace = _params->GetSpaceMaxExtents();

    // If minSpace is empty, then we have an empty set of params.  Just return.
    if (minSpace.empty()) {
        return;
    }

    _spaceXRange->setUserMin(minSpace[0]);
    _spaceYRange->setUserMin(minSpace[1]);
    _spaceZRange->setUserMin(minSpace[2]);
    _spaceXRange->setUserMax(maxSpace[0]);
    _spaceYRange->setUserMax(maxSpace[1]);
    _spaceZRange->setUserMax(maxSpace[2]);

    int timeMinTS, timeMaxTS;
    timeMinTS = _params->GetTimeMinTS();
    timeMaxTS = _params->GetTimeMaxTS();
    _timeTimeRange->setUserMin(timeMinTS);
    _timeTimeRange->setUserMax(timeMaxTS);
}

void Plot::initCRatios() {
    _cRatios = _dm->GetCRatios(_defaultVar);
    cRatioCombo->clear();

    for (std::vector<size_t>::iterator it = _cRatios.begin(); it != _cRatios.end(); ++it) {
        cRatioCombo->addItem("1:" + QString::number(*it));
    }

    _cRatio = _cRatios.size() - 1;
    cRatioCombo->setCurrentIndex(_cRatio);
}

void Plot::initRefinement() {
    refCombo->clear();
    _refLevel = _dm->GetNumRefLevels(_defaultVar);
    for (int i = 0; i <= _refLevel; i++) {
        refCombo->blockSignals(true);
        refCombo->addItem(QString::number(i));
        refCombo->blockSignals(false);
    }

    refCombo->setCurrentIndex(_refLevel);
}

void Plot::initSSCs() {

    _spaceTimeRange = new Range(_timeExtents[0], _timeExtents[1]);
    _spaceTimeRange->setConst(true);
    _spaceTimeSlider = new TimeSlider(_spaceTimeRange, spaceTimeSlider);
    _spaceTimeLineEdit = new SinglePointLineEdit(_spaceTimeRange, spaceTimeEdit, 0);
    _spaceTimeCell = new MinMaxTableCell(_spaceTimeRange, spaceTable, 0, 3, 0);
    _spaceTimeMinLabel = new MinMaxLabel(_spaceTimeRange, spaceTimeMinLabel, 0);
    _spaceTimeMinLabel = new MinMaxLabel(_spaceTimeRange, spaceTimeMaxLabel, 1);
    _spaceTimeRange->addObserver(_spaceTimeCell);
    _spaceTimeRange->addObserver(_spaceTimeSlider);
    _spaceTimeRange->addObserver(_spaceTimeLineEdit);

    _timeTimeRange = new Range(_timeExtents[0], _timeExtents[1]);
    _timeTimeMinSlider = new TimeSlider(_timeTimeRange, timeTimeMinSlider, 0);
    _timeTimeMaxSlider = new TimeSlider(_timeTimeRange, timeTimeMaxSlider, 1);
    _timeTimeMinLineEdit = new MinMaxLineEdit(_timeTimeRange, timeTimeMinEdit, 0);
    _timeTimeMaxLineEdit = new MinMaxLineEdit(_timeTimeRange, timeTimeMaxEdit, 1);
    _timeTimeMinCell = new MinMaxTableCell(_timeTimeRange, timeTable, 0, 3, 0);
    _timeTimeMaxCell = new MinMaxTableCell(_timeTimeRange, timeTable, 1, 3, 1);
    _timeTimeRange->addObserver(_timeTimeMinCell);
    _timeTimeRange->addObserver(_timeTimeMaxCell);
    _timeTimeRange->addObserver(_timeTimeMinSlider);
    _timeTimeRange->addObserver(_timeTimeMaxSlider);
    _timeTimeRange->addObserver(_timeTimeMinLineEdit);
    _timeTimeRange->addObserver(_timeTimeMaxLineEdit);

    _spaceXRange = new Range(_extents[0], _extents[3]);
    _spaceP1XSlider = new MinMaxSlider(_spaceXRange, spaceP1XSlider, 0);
    _spaceP2XSlider = new MinMaxSlider(_spaceXRange, spaceP2XSlider, 1);
    _spaceP1XLineEdit = new MinMaxLineEdit(_spaceXRange, spaceP1XEdit, 0);
    _spaceP2XLineEdit = new MinMaxLineEdit(_spaceXRange, spaceP2XEdit, 1);
    _spaceP1XCell = new MinMaxTableCell(_spaceXRange, spaceTable, 0, 0, 0);
    _spaceP2XCell = new MinMaxTableCell(_spaceXRange, spaceTable, 1, 0, 1);
    _spaceP1XMinLabel = new MinMaxLabel(_spaceXRange, spaceP1XMin, 0);
    _spaceP1XMaxLabel = new MinMaxLabel(_spaceXRange, spaceP1XMax, 1);
    _spaceP2XMinLabel = new MinMaxLabel(_spaceXRange, spaceP2XMin, 0);
    _spaceP2XMaxLabel = new MinMaxLabel(_spaceXRange, spaceP2XMax, 1);
    _spaceXRange->addObserver(_spaceP1XCell);
    _spaceXRange->addObserver(_spaceP2XCell);
    _spaceXRange->addObserver(_spaceP1XSlider);
    _spaceXRange->addObserver(_spaceP2XSlider);
    _spaceXRange->addObserver(_spaceP1XLineEdit);
    _spaceXRange->addObserver(_spaceP2XLineEdit);

    _spaceYRange = new Range(_extents[1], _extents[4]);
    _spaceP1YSlider = new MinMaxSlider(_spaceYRange, spaceP1YSlider, 0);
    _spaceP2YSlider = new MinMaxSlider(_spaceYRange, spaceP2YSlider, 1);
    _spaceP1YLineEdit = new MinMaxLineEdit(_spaceYRange, spaceP1YEdit, 0);
    _spaceP2YLineEdit = new MinMaxLineEdit(_spaceYRange, spaceP2YEdit, 1);
    _spaceP1YCell = new MinMaxTableCell(_spaceYRange, spaceTable, 0, 1, 0);
    _spaceP2YCell = new MinMaxTableCell(_spaceYRange, spaceTable, 1, 1, 1);
    _spaceP1YMinLabel = new MinMaxLabel(_spaceYRange, spaceP1YMin, 0);
    _spaceP1YMaxLabel = new MinMaxLabel(_spaceYRange, spaceP1YMax, 1);
    _spaceP2YMinLabel = new MinMaxLabel(_spaceYRange, spaceP2YMin, 0);
    _spaceP2YMaxLabel = new MinMaxLabel(_spaceYRange, spaceP2YMax, 1);
    _spaceYRange->addObserver(_spaceP1YCell);
    _spaceYRange->addObserver(_spaceP2YCell);
    _spaceYRange->addObserver(_spaceP1YSlider);
    _spaceYRange->addObserver(_spaceP2YSlider);
    _spaceYRange->addObserver(_spaceP1YLineEdit);
    _spaceYRange->addObserver(_spaceP2YLineEdit);

    _spaceZRange = new Range(_extents[2], _extents[5]);
    _spaceP1ZSlider = new MinMaxSlider(_spaceZRange, spaceP1ZSlider, 0);
    _spaceP2ZSlider = new MinMaxSlider(_spaceZRange, spaceP2ZSlider, 1);
    _spaceP1ZLineEdit = new MinMaxLineEdit(_spaceZRange, spaceP1ZEdit, 0);
    _spaceP2ZLineEdit = new MinMaxLineEdit(_spaceZRange, spaceP2ZEdit, 1);
    _spaceP1ZCell = new MinMaxTableCell(_spaceZRange, spaceTable, 0, 2, 0);
    _spaceP2ZCell = new MinMaxTableCell(_spaceZRange, spaceTable, 1, 2, 1);
    _spaceP1ZMinLabel = new MinMaxLabel(_spaceZRange, spaceP1ZMin, 0);
    _spaceP1ZMaxLabel = new MinMaxLabel(_spaceZRange, spaceP1ZMax, 1);
    _spaceP2ZMinLabel = new MinMaxLabel(_spaceZRange, spaceP2ZMin, 0);
    _spaceP2ZMaxLabel = new MinMaxLabel(_spaceZRange, spaceP2ZMax, 1);
    _spaceZRange->addObserver(_spaceP1ZCell);
    _spaceZRange->addObserver(_spaceP2ZCell);
    _spaceZRange->addObserver(_spaceP1ZSlider);
    _spaceZRange->addObserver(_spaceP2ZSlider);
    _spaceZRange->addObserver(_spaceP1ZLineEdit);
    _spaceZRange->addObserver(_spaceP2ZLineEdit);

    _timeXRange = new Range(_extents[0], _extents[3]);
    _timeXRange->setConst(true);
    _timeXSlider = new MinMaxSlider(_timeXRange, timeXSlider);
    _timeXLineEdit = new SinglePointLineEdit(_timeXRange, timeXEdit, _extents[0]);
    _timeXCell = new MinMaxTableCell(_timeXRange, timeTable, 0, 0, 0);
    _timeXMinLabel = new MinMaxLabel(_timeXRange, timeXMin, 0);
    _timeXMaxLabel = new MinMaxLabel(_timeXRange, timeXMax, 1);
    _timeXRange->addObserver(_timeXCell);
    _timeXRange->addObserver(_timeXSlider);
    _timeXRange->addObserver(_timeXLineEdit);

    _timeYRange = new Range(_extents[1], _extents[4]);
    _timeYRange->setConst(true);
    _timeYSlider = new MinMaxSlider(_timeYRange, timeYSlider);
    _timeYLineEdit = new SinglePointLineEdit(_timeYRange, timeYEdit, _extents[1]);
    _timeYCell = new MinMaxTableCell(_timeYRange, timeTable, 0, 1, 0);
    _timeYMinLabel = new MinMaxLabel(_timeYRange, timeYMin, 0);
    _timeYMaxLabel = new MinMaxLabel(_timeYRange, timeYMax, 1);
    _timeYRange->addObserver(_timeYCell);
    _timeYRange->addObserver(_timeYSlider);
    _timeYRange->addObserver(_timeYLineEdit);

    _timeZRange = new Range(_extents[2], _extents[5]);
    _timeZRange->setConst(true);
    _timeZSlider = new MinMaxSlider(_timeZRange, timeZSlider);
    _timeZLineEdit = new SinglePointLineEdit(_timeZRange, timeZEdit, _extents[2]);
    _timeZCell = new MinMaxTableCell(_timeZRange, timeTable, 0, 2, 0);
    _timeZMinLabel = new MinMaxLabel(_timeZRange, timeZMin, 0);
    _timeZMaxLabel = new MinMaxLabel(_timeZRange, timeZMax, 1);
    _timeZRange->addObserver(_timeZCell);
    _timeZRange->addObserver(_timeZSlider);
    _timeZRange->addObserver(_timeZLineEdit);
}

bool Plot::eventFilter(QObject *target, QEvent *event) {
    vector<double> spaceMinExts, spaceMaxExts, timeExts;
    int spaceTS, minTimeTS, maxTimeTS;

    spaceMinExts.push_back(_spaceXRange->getUserMin());
    spaceMinExts.push_back(_spaceYRange->getUserMin());
    spaceMinExts.push_back(_spaceZRange->getUserMin());
    spaceMaxExts.push_back(_spaceXRange->getUserMax());
    spaceMaxExts.push_back(_spaceYRange->getUserMax());
    spaceMaxExts.push_back(_spaceZRange->getUserMax());
    timeExts.push_back(_timeXRange->getUserMin());
    timeExts.push_back(_timeYRange->getUserMin());
    timeExts.push_back(_timeZRange->getUserMin());

    spaceTS = _spaceTimeRange->getUserMin();
    maxTimeTS = _timeTimeRange->getUserMin();
    minTimeTS = _timeTimeRange->getUserMax();

    return false;
}

void Plot::initPlotDlg() {
    if (_plotDialog == NULL)
        _plotDialog = new QDialog(this);
    if (_plotLabel == NULL)
        _plotLabel = new QLabel(this);
    if (_plotButton == NULL)
        _plotButton = new QPushButton(this);
    if (_plotLayout == NULL)
        _plotLayout = new QVBoxLayout();

    _plotButton->setText("Save to file");
    _plotLayout->addWidget(_plotLabel);
    _plotLayout->addWidget(_plotButton);
    _plotDialog->setLayout(_plotLayout);

    connect(_plotButton, SIGNAL(pressed()), this, SLOT(savePlotToFile()));
}

void Plot::savePlotToFile() {
    QFileDialog *fd = new QFileDialog;
    fd->setDefaultSuffix(".png");
    string file = fd->getSaveFileName(this, "Select Output File Name",
                                      "/", tr("(*.png)"))
                      .toStdString();
    ;
    delete fd;

    std::ifstream src(_defaultImageLocation.c_str(), std::ios::binary);
    std::ofstream dest(file.c_str(), std::ios::binary);
    dest << src.rdbuf();
}

// Our sampling rate is based on how many voxels are crossed
// between points a and b, multiplied by 2
//
int Plot::findNyquist(
    const vector<size_t> minv, const vector<size_t> maxv,
    const vector<double> minu, const vector<double> maxu,
    double &dX, double &dY, double &dZ) const {

    //int s1 = maxv[0]>minv[0] ? maxv[0]-minv[0] : minv[0]-maxv[0];
    //int s2 = maxv[1]>minv[1] ? maxv[1]-minv[1] : minv[1]-maxv[1];
    //int s3 = maxv[2]>minv[2] ? maxv[2]-minv[2] : minv[2]-maxv[2];
    int s1 = abs((int)maxv[0] - (int)minv[0]);
    int s2 = abs((int)maxv[1] - (int)minv[1]);
    int s3 = abs((int)maxv[2] - (int)minv[2]);
    int nsamples = 2 * ceil(sqrt((double)(s1 * s1 + s2 * s2 + s3 * s3)));

    if (nsamples < 2)
        nsamples = 2;

    dX = ((maxu[0] - minu[0]) / (double)(nsamples - 1));
    dY = ((maxu[1] - minu[1]) / (double)(nsamples - 1));
    dZ = ((maxu[2] - minu[2]) / (double)(nsamples - 1));

    return nsamples;
}

// Fetch an error message genereated by Python API.  Python API doesn't
// provide a means for getting error message  as a string (all formatted
// error messages are written to stderr), so we have to redirect Python's
// stderr to a memory buffer.
//
string Plot::pyErr() const {
    PyObject *pMain = PyImport_AddModule("__main__");

    PyObject *catcher = NULL;
    if (pMain && PyObject_HasAttrString(pMain, "catchErr")) {
        catcher = PyObject_GetAttrString(pMain, "catchErr");
    }

    // If catcher is NULL the Python message will be written
    // to stderr. Otherwise it is writter to the catchErr object.
    //
    PyErr_Print();

    if (!catcher) {
        cerr << "CATCHER NULL" << endl;
        return ("No Python error catcher");
    }

    PyObject *output = PyObject_GetAttrString(catcher, "value");
    if (!output) {
        return ("Can't get err msg : PyObject_GetAttrString(catcher,'value')");
    }
    return (PyString_AsString(output));
    return "";
}

// Initialize embedded python environment.
//
// TODO: restructure this code so it can be used by SeedMe and move
// to MyPython helper class.
//

int Plot::initPython() {
    if (_isInitializedPython)
        return (0); // Static one time initialization!

    // Ugh. Have to define a python object to enable capturing of
    // stderr to a string. Python API doesn't support a version of
    // PyErr_Print() that fetches the error to a C++ string. Give me
    // a break!
    //
    std::string stdErr =
        "import sys\n"
        "class CatchErr:\n"
        "	def __init__(self):\n"
        "		self.value = 'Plot: '\n"
        "	def write(self, txt):\n"
        "		self.value += txt\n"
        "catchErr = CatchErr()\n"
        "sys.stderr = catchErr\n";

    // Use MyPython singleton class to initialize Python interpeter to
    // ensure it only gets initialized once.
    //
    Wasp::MyPython::Instance()->Initialize();

    // see http://docs.scipy.org/doc/numpy/reference/
    //  c-api.array.html#importing-the-api
    //
    import_array1(1)

        // Catch stderr from Python to a string.
        //
        int rc = PyRun_SimpleString(stdErr.c_str());
    if (rc < 0) {
        errReport(pyErr());
        return (-1);
    }

    PyObject *pMain = PyImport_AddModule("__main__");
    if (!pMain) {
        errReport(pyErr());
        return (-1);
    }

    // Create a new module object
    //
    PyObject *pModule = PyModule_New("plotModule");
    if (!pModule) {
        errReport(pyErr());
        return (-1);
    }
    rc = PyModule_AddStringConstant(pModule, "__file__", "");
    if (rc < 0) {
        errReport(pyErr());
        return (-1);
    }

    Py_DECREF(pModule);

    _isInitializedPython = true;

    return (0);
}

vector<string> Plot::getEnabledVars() const {
    vector<string> enabledVars;
    int rows = variablesTable->rowCount();

    for (int i = 0; i < rows; i++) {
        QWidget *wid = variablesTable->cellWidget(i, 0);
        QCheckBox *box = wid->findChild<QCheckBox *>();
        if (box->checkState()) {
            string varName;
            varName = variablesTable->verticalHeaderItem(i)->text().toStdString();
            enabledVars.push_back(varName);
        }
    }
    return enabledVars;
}

void Plot::go() {
    vector<string> enabledVars;
    enabledVars = getEnabledVars();

    if (enabledVars.empty()) {
        errReport("No variables selected for plotting");
        return;
    }

    // Build a dictionary of varName:valueList pairs (string:list)
    // according to whether the user is generating a plot through
    // space or through time.
    //
    map<string, vector<float>> data;
    map<string, vector<float>> iData;
    if (spaceTimeTab->currentIndex() == 0) {

        // Get samples
        //
        int rc = getSpatialVectors(enabledVars, data, iData);
        if (rc < 0)
            return;

    } else {

        // Get samples
        //
        int rc = getTemporalVectors(enabledVars, data, iData);
        if (rc < 0)
            return;
    }

    // Marshal data to python
    //
    PyObject *pyDict = buildPyDict(data);
    if (!pyDict) {
        errReport(pyErr());
        return;
    }

    PyObject *pyDictKey = buildPyDict(iData);
    if (!pyDictKey) {
        errReport(pyErr());
        return;
    }

    // Get upload script. Fetch it new each time to allow for the script
    // to be changed.
    //
    string plotScript = readPlotScript();
    if (plotScript.empty())
        return;

    PyObject *pFunc = MyPython::CreatePyFunc(
        "plotModule", "plot1D", plotScript);
    if (!pFunc) {
        errReport(pyErr());
        return;
    }

    PyObject *pArgs = PyTuple_New(2);
    if (!pArgs) {
        errReport(pyErr());
        return;
    }

    int rc = PyTuple_SetItem(pArgs, 0, pyDict);
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    rc = PyTuple_SetItem(
        pArgs, 1, PyUnicode_FromString(_defaultImageLocation.c_str()));
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    PyObject *pValue = PyObject_Call(pFunc, pArgs, pyDictKey);
    Py_DECREF(pArgs);
    if (!pValue) {
        errReport(pyErr());
        return;
    }

    Py_DECREF(pyDict);
    Py_DECREF(pyDictKey);
    Py_DECREF(pFunc);

    // Output of pFunc is a raster image. Slurp it in and display it
    //
    QImage image(QString::fromStdString(_defaultImageLocation));
    _plotLabel->setPixmap(QPixmap::fromImage(image));
    _plotDialog->show();
    _plotDialog->raise();
    _plotDialog->activateWindow();
}

void Plot::refinementChanged(int i) {
    _refLevel = i;
    _params->SetRefinement(i);
}

void Plot::cRatioChanged(int i) {
    _cRatio = i;
    _params->SetCRatio(i);
}

// Get extents of line in user coordinates. Ensure min[i] <= max[i]
//
void Plot::getSpatialExtents(
    vector<double> &minu, vector<double> &maxu, size_t &ts) const {

    if (minu.size() > 0) {
        minu[0] = _spaceXRange->getUserMin();
        maxu[0] = _spaceXRange->getUserMax();
        minu[1] = _spaceYRange->getUserMin();
        maxu[1] = _spaceYRange->getUserMax();
        minu[2] = _spaceZRange->getUserMin();
        maxu[2] = _spaceZRange->getUserMax();
    } else {
        minu.push_back(_spaceXRange->getUserMin());
        minu.push_back(_spaceYRange->getUserMin());
        minu.push_back(_spaceZRange->getUserMin());
        maxu.push_back(_spaceXRange->getUserMax());
        maxu.push_back(_spaceYRange->getUserMax());
        maxu.push_back(_spaceZRange->getUserMax());
    }

    // Swap if needed
    //
    for (int i = 0; i < 3; i++) {
        if (minu[i] > maxu[i]) {
            double tmp = minu[i];
            minu[i] = maxu[i];
            maxu[i] = tmp;
        }
    }

    ts = _timeTimeRange->getUserMin();
}

// Sample selected variables over a line through space
//
int Plot::getSpatialVectors(
    const vector<string> vars,
    map<string, vector<float>> &data,
    map<string, vector<float>> &iData) const {

    data.clear();
    iData.clear();

    // Get bounding box for data (line extents) in user coordinates
    //
    size_t ts;
    vector<double> minu, maxu;
    getSpatialExtents(minu, maxu, ts);

    int nsamples = 0;
    double dX, dY, dZ;
    vector<size_t> minv, maxv;

    for (int i = 0; i < vars.size(); i++) {
        string var = vars[i];
        vector<float> vec;

        StructuredGrid *sg = _dm->GetVariable(ts, var, _refLevel, _cRatio, minu, maxu);
        if (!sg) {
            return (-1);
        }

        sg->GetEnclosingRegion(minu, maxu, minv, maxv);

        // Use first SG to figure out how many samples we need,
        // and step size. Only do this with first variable. All
        // variables sampled same number of times
        //
        if (!nsamples) {
            nsamples = findNyquist(minv, maxv, minu, maxu, dX, dY, dZ);
        }

        float mv = 0.f;
        for (int j = 0; j < nsamples; ++j) {
            float val = 0.f;

            // Map missing values to NaNs. matplotlib won't plot
            // these. May need to using a numpy masked array in future.
            //
            if (val == mv) {
                val = std::numeric_limits<double>::quiet_NaN();
            }

            vec.push_back(val);
        }

        //		if (sg) delete sg;

        data[var] = vec;
    }

    // Compute array for independent variable
    //
    double delta = nsamples > 1 ? 1.0 / (double)(nsamples - 1) : 0.0;
    double x0 = 0.0;
    if (_spaceYRange->getConst() && _spaceZRange->getConst()) {
        delta = dX;
        x0 = minu[0];
    } else if (_spaceXRange->getConst() && _spaceZRange->getConst()) {
        delta = dY;
        x0 = minu[1];
    } else if (_spaceXRange->getConst() && _spaceYRange->getConst()) {
        delta = dZ;
        x0 = minu[2];
    } else if (_spaceXRange->getConst() &&
               _spaceYRange->getConst() &&
               _spaceZRange->getConst()) {
        delta = 1;
        x0 = 0.0;
    }

    vector<float> vec;
    for (int i = 0; i < nsamples; i++) {
        vec.push_back(i * delta + x0);
    }
    iData["X"] = vec;

    return (0);
}

// Get extents of line in user coordinates. Ensure ts0 <= ts1
//
void Plot::getTemporalExtents(
    vector<double> &xyz, size_t &ts0, size_t &ts1) const {
    ts0 = _timeTimeRange->getUserMin();
    ts1 = _timeTimeRange->getUserMax();

    if (ts0 > ts1) {
        size_t tmp = ts0;
        ts0 = ts1;
        ts1 = tmp;
    }

    if (xyz.size() < 3) {
        xyz.clear();
        xyz.push_back(_timeXRange->getUserMin());
        xyz.push_back(_timeYRange->getUserMin());
        xyz.push_back(_timeZRange->getUserMin());
    } else {
        xyz[0] = _timeXRange->getUserMin();
        xyz[1] = _timeYRange->getUserMin();
        xyz[2] = _timeZRange->getUserMin();
    }
}

int Plot::getTemporalVectors(
    const vector<string> vars,
    map<string, vector<float>> &data,
    map<string, vector<float>> &iData) const {

    data.clear();
    iData.clear();

    // Get spatial coordinates of selected  point in user coordinates,
    // and temporal range in time steps
    //
    vector<double> xyz;
    size_t ts0, ts1;
    getTemporalExtents(xyz, ts0, ts1);

    for (int i = 0; i < vars.size(); i++) {
        string var = vars[i];
        vector<float> vec;

        for (size_t ts = ts0; ts <= ts1; ts++) {

            StructuredGrid *sg = _dm->GetVariable(
                ts, var, _refLevel, _cRatio);
            if (!sg) {
                return (-1);
            }

            float val = sg->GetValue(xyz[0], xyz[1], xyz[2]);

            // TODO: Handle missing values
            //
            vec.push_back(val);
        }
        data[var] = vec;
    }

    vector<double> allTimes;
    vector<float> requestedTimes;
    _dm->GetTimeCoordinates(allTimes);

    vector<float> vec;
    for (size_t ts = ts0; ts <= ts1; ts++) {
        requestedTimes.push_back(allTimes[ts]);
    }
    iData["X"] = requestedTimes;
    return (0);
}

PyObject *Plot::buildNumpyArray(const vector<float> &vec) const {

    npy_intp dims[] = {static_cast<npy_intp>(vec.size())};
    PyObject *myArray = (PyObject *)PyArray_SimpleNew(
        1, dims, NPY_FLOAT);
    if (!myArray)
        return (NULL);

    // create the iterators
    //
    NpyIter *iter = NpyIter_New(
        (PyArrayObject *)myArray, NPY_ITER_READWRITE, NPY_CORDER,
        NPY_NO_CASTING, NULL);
    if (!iter)
        return (NULL);

    NpyIter_IterNextFunc *iternext = NpyIter_GetIterNext(iter, NULL);
    if (!iternext)
        return (NULL);

    float **dataptr = (float **)NpyIter_GetDataPtrArray(iter);

    //  iterate over the arrays
    //
    int i = 0;
    do {
        assert(i < vec.size());
        **dataptr = vec[i];
        i++;
    } while (iternext(iter));

    //  clean up and return the result
    //
    NpyIter_Deallocate(iter);
    Py_INCREF(myArray);
    return myArray;
}

PyObject *Plot::buildPyDict(
    const map<string, vector<float>> &data) {
    PyObject *pyDict = PyDict_New();

    map<string, vector<float>>::const_iterator itr;
    for (itr = data.begin(); itr != data.end(); ++itr) {

        string var = itr->first;
        const vector<float> &vref = itr->second;

        PyObject *pyArray = buildNumpyArray(vref);

        PyObject *key = Py_BuildValue("s", var.c_str());
        if (!key) {
            Py_DECREF(pyArray);
            errReport(pyErr());
            return (NULL);
        }

        int rc = PyDict_SetItem(pyDict, key, pyArray);
        if (rc < 0) {
            errReport(pyErr());
            Py_DECREF(pyDict);
            return (NULL);
        }

        Py_DECREF(pyArray);
    }
    return pyDict;
}
//#endif

void Plot::getRanges(QObject *&sender, QComboBox *&qcb, Range *&x, Range *&y, Range *&z) {
    if (sender == timeCopyCombo) {
        qcb = timeCopyCombo;
        x = _timeXRange;
        y = _timeYRange;
        z = _timeZRange;
    }
    if (sender == spaceP1CopyCombo) {
        qcb = spaceP1CopyCombo;
        x = _spaceXRange;
        y = _spaceYRange;
        z = _spaceZRange;
    }
    if (sender == spaceP2CopyCombo) {
        qcb = spaceP2CopyCombo;
        x = _spaceXRange;
        y = _spaceYRange;
        z = _spaceZRange;
    }
}

#ifdef DEAD
void Plot::getPointFromRenderer() {
    QComboBox *qcb = NULL;
    Range *x = NULL;
    Range *y = NULL;
    Range *z = NULL;
    QObject *sender = QObject::sender();
    getRanges(sender, qcb, x, y, z);

    RenderParams *p = NULL;
    string renderer = qcb->currentText().toStdString();
    qcb->setCurrentIndex(0);

    if (renderer == "Probe")
        p = _vwm->getActiveProbeParams();
    else if (renderer == "2D")
        p = _vwm->getActiveTwoDDataParams();
    else if (renderer == "Isoline")
        p = _vwm->getActiveIsolineParams();

    if (p == NULL) {
        errReport("RenderParams is NULL in getPointFromRenderer()");
        return;
    }

    if (!p->isEnabled()) {
        errReport("Selected renderer must be enabled to copy its point.");
        return;
    }

    const float *selectedPoint = p->getSelectedPointLocal();
    float selectedCoord[3];
    for (int i = 0; i < 3; i++) {
        selectedCoord[i] = selectedPoint[i] + _extents[i];
    }

    if (sender == spaceP2CopyCombo) {
        x->setUserMax((double)selectedCoord[0]);
        y->setUserMax((double)selectedCoord[1]);
        z->setUserMax((double)selectedCoord[2]);
    } else if (sender == spaceP1CopyCombo) {
        x->setUserMin((double)selectedCoord[0]);
        y->setUserMin((double)selectedCoord[1]);
        z->setUserMin((double)selectedCoord[2]);
    } else if (sender == timeCopyCombo) {
        x->setUserMin((double)selectedCoord[0]);
        y->setUserMin((double)selectedCoord[1]);
        z->setUserMin((double)selectedCoord[2]);
    }
}
#endif

void Plot::constCheckboxChanged(int state) {
    QObject *sender = QObject::sender();
    if (sender == _spaceCheckBoxes[0]) {
        _spaceXRange->setConst(state);
        _params->SetXConst((bool)state);
    } else if (sender == _spaceCheckBoxes[1]) {
        _spaceYRange->setConst(state);
        _params->SetXConst((bool)state);
    } else if (sender == _spaceCheckBoxes[2]) {
        _spaceZRange->setConst(state);
        _params->SetXConst((bool)state);
    } else if (sender == _timeCheckBoxes[0]) {
        _timeTimeRange->setConst(state);
        _params->SetTimeConst((bool)state);
    }
}

void Plot::print(bool doSpace) const {
    vector<string>::const_iterator it;

    vector<double> minu, maxu;
    size_t ts0, ts1;
    if (doSpace) {
        getSpatialExtents(minu, maxu, ts0);
        ts1 = ts0;
    } else {
        getTemporalExtents(minu, ts0, ts1);
        maxu[0] = minu[0];
        maxu[1] = minu[1];
        maxu[2] = minu[2];
    }

    cout << "Vars: " << endl;
    for (it = _uVars.begin(); it != _uVars.end(); ++it)
        cout << *it << endl;
    cout << "T Coords: " << ts0 << " " << ts1 << endl;
    cout << "X Coords: " << minu[0] << " " << maxu[0] << endl;
    cout << "Y Coords: " << minu[1] << " " << maxu[1] << endl;
    cout << "Z Coords: " << minu[2] << " " << maxu[2] << endl;
}

void Plot::initTimes() {
    _timeExtents.clear();

    _timeExtents.push_back(0);
    _timeExtents.push_back(1);
    //_timeExtents.push_back(_dm->GetNumTimeSteps()-1);
}

int Plot::initExtents(int ts, vector<double> &extents) {
    vector<double> minExtents, maxExtents;
    int rc = _dm->GetVariableExtents(ts, _defaultVar, _refLevel,
                                     minExtents, maxExtents);
    if (rc < 0) {
        string myErr;
        myErr = "Could not find minimum and maximun extents in current data set.";
        errReport(myErr);
        return -1;
    }
    if (extents.size() < 6) {
        extents.push_back(minExtents[0]);
        extents.push_back(minExtents[1]);
        extents.push_back(minExtents[2]);
        extents.push_back(maxExtents[0]);
        extents.push_back(maxExtents[1]);
        extents.push_back(maxExtents[2]);

#ifdef DEAD
        // TYPO???
        extents.push_back(minExtents[0]);
        extents.push_back(minExtents[1]);
        extents.push_back(minExtents[2]);
        extents.push_back(maxExtents[0]);
        extents.push_back(maxExtents[1]);
        extents.push_back(maxExtents[2]);
#endif
    } else {
        extents[0] = minExtents[0];
        extents[1] = minExtents[1];
        extents[2] = minExtents[2];
        extents[3] = maxExtents[0];
        extents[4] = maxExtents[1];
        extents[5] = maxExtents[2];
    }
    return 1;
}

void Plot::initConstCheckboxes() {
    for (int i = 0; i < 4; i++) {
        QWidget *pWidget = new QWidget();
        _spaceCheckBoxes.push_back(new QCheckBox());
        if (i == 3) {
            _spaceCheckBoxes[i]->setCheckState(Qt::Checked);
            _spaceCheckBoxes[i]->setEnabled(false);
        }
        QHBoxLayout *pLayout = new QHBoxLayout(pWidget);
        pLayout->addWidget(_spaceCheckBoxes[i]);
        pLayout->setAlignment(Qt::AlignCenter);
        pLayout->setContentsMargins(0, 0, 0, 0);
        pWidget->setLayout(pLayout);
        spaceTable->setCellWidget(i, 2, pWidget);

        pWidget = new QWidget();
        _timeCheckBoxes.push_back(new QCheckBox());
        if (i < 3) {
            _timeCheckBoxes[i]->setCheckState(Qt::Checked);
            _timeCheckBoxes[i]->setEnabled(false);
        }
        pLayout = new QHBoxLayout(pWidget);
        pLayout->addWidget(_timeCheckBoxes[i]);
        pLayout->setAlignment(Qt::AlignCenter);
        pLayout->setContentsMargins(0, 0, 0, 0);
        pWidget->setLayout(pLayout);
        timeTable->setCellWidget(i, 2, pWidget);
    }
}

// Called whenever list of variables changes
//
void Plot::initVariables() {
    _vars.clear();
    addVarCombo->clear(); // Clear old variables
    addVarCombo->addItem("Add Variable:");
    vector<string> vars;

    vars = _dm->GetDataVarNames(3, true);
    for (std::vector<string>::iterator it = vars.begin(); it != vars.end(); ++it) {
        _vars.push_back(*it);
        _vars3d.push_back(*it);
    }

    vars = _dm->GetDataVarNames(2, true);
    for (std::vector<string>::iterator it = vars.begin(); it != vars.end(); ++it) {
        _vars.push_back(*it);
    }

    _defaultVar = _vars3d[0];

    sort(_vars.begin(), _vars.end());

    // Add variables to combo box
    for (std::vector<string>::iterator it = _vars.begin(); it != _vars.end(); ++it) {
        addVarCombo->addItem(QString::fromStdString(*it));
    }
}

void Plot::newVarAdded(int index) {

    if (index == 0)
        return;
    string varName = addVarCombo->currentText().toStdString();

    if (std::find(_uVars.begin(), _uVars.end(), varName) != _uVars.end())
        return;

    _uVars.push_back(varName);

    int rowCount = variablesTable->rowCount();
    variablesTable->insertRow(rowCount);
    variablesTable->setVerticalHeaderItem(rowCount, new QTableWidgetItem(QString::fromStdString(varName)));

    QHeaderView *verticalHeader = variablesTable->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(20);

    // Add checkboxes to each variable/row
    int colCount = variablesTable->columnCount();
    for (int j = 0; j < colCount; j++) {

        QWidget *cell = new QWidget();
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setCheckState(Qt::Checked);
        QHBoxLayout *layout = new QHBoxLayout(cell);
        layout->addWidget(checkBox);
        layout->setAlignment(Qt::AlignHCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        cell->setLayout(layout);
        variablesTable->setCellWidget(rowCount, j, cell);
    }
    variablesTable->resizeRowsToContents();
    addVarCombo->setCurrentIndex(0);

    removeVarCombo->addItem(QString::fromStdString(varName));
    addVarCombo->removeItem(index);

    // If all variables are 2D, disable Z axis controllers
    //
    bool varsAre2D = true;
    for (int i = 0; i < _uVars.size(); i++) {
#ifdef FIXED
//		if (_dm->GetVarType(_uVars[i]) == DataMgr::VAR3D) {
//			varsAre2D=false;
//		}
#endif
    }
    varsAre2D ? enableZControllers(false) : enableZControllers(true);

    _params->SetVarNames(_uVars);
}

void Plot::enableZControllers(bool s) {
    spaceP1ZSlider->setEnabled(s);
    spaceP2ZSlider->setEnabled(s);
    spaceP1ZEdit->setEnabled(s);
    spaceP2ZEdit->setEnabled(s);

    timeZSlider->setEnabled(s);
    timeZEdit->setEnabled(s);

    spaceP1ZEdit->blockSignals(true);
    spaceP2ZEdit->blockSignals(true);
    timeZEdit->blockSignals(true);
    if (s) {
        spaceP1ZEdit->setText(QString::number(_extents[2]));
        spaceP2ZEdit->setText(QString::number(_extents[5]));
        timeZEdit->setText(QString::number(_extents[2]));
    } else {
        spaceP1ZEdit->clear();
        spaceP2ZEdit->clear();
        timeZEdit->clear();
    }
    spaceP1ZEdit->blockSignals(false);
    spaceP2ZEdit->blockSignals(false);
    timeZEdit->blockSignals(false);
}

void Plot::removeVar(int index) {
    if (index == 0)
        return;
    string varName = removeVarCombo->currentText().toStdString();

    _uVars.erase(std::remove(_uVars.begin(), _uVars.end(), varName), _uVars.end());

    addVarCombo->addItem(QString::fromStdString(varName));
    removeVarCombo->removeItem(index);

    string rowName;
    int rowCount = variablesTable->rowCount();
    for (int i = 0; i < rowCount; i++) {
        rowName = variablesTable->verticalHeaderItem(i)->text().toStdString();
        if (varName == rowName) {
            variablesTable->removeRow(i);
            break;
        }
    }

    removeVarCombo->setCurrentIndex(0);

    _params->SetVarNames(_uVars);
}

void Plot::initTables() {
    variablesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    variablesTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
    variablesTable->resizeRowsToContents();
    variablesTable->resizeColumnsToContents();

    spaceTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    spaceTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
    spaceTable->resizeRowsToContents();
    spaceTable->resizeColumnsToContents();

    timeTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    timeTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
    timeTable->resizeRowsToContents();
    timeTable->resizeColumnsToContents();

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 4; i++) {
            QTableWidgetItem *scell = new QTableWidgetItem;
            Qt::ItemFlags flags = scell->flags();
            flags &= !Qt::ItemIsEditable;
            scell->setFlags(flags);
            scell->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            spaceTable->setItem(i, j, scell);

            QTableWidgetItem *tcell = new QTableWidgetItem;
            flags = tcell->flags();
            flags &= !Qt::ItemIsEditable;
            tcell->setFlags(flags);
            tcell->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            timeTable->setItem(i, j, tcell);
        }
    }
}

void Plot::errReport(string msg) const {
    _errMsg->errorList->setText(QString::fromStdString(msg));
    _errMsg->show();
    _errMsg->raise();
    _errMsg->activateWindow();
}
