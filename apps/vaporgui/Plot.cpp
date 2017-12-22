//************************************************************************
//																	  *
//		   Copyright (C)  2016										*
//	 University Corporation for Atmospheric Research				  *
//		   All Rights Reserved										*
//																	  *
//************************************************************************/
//
//  File:	   plot.cpp
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

#include <Python.h>
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
#include <QMessageBox>
#include <QKeyEvent>
#include <vapor/MyPython.h>
#include <vapor/GetAppPath.h>
#include <vapor/DataMgr.h>
#include "Plot.h"
#include "RangeController.h"

#define NPY_NO_DEPRECATED_API NPY_1_8_API_VERSION
#include <numpy/ndarrayobject.h>

using namespace VAPoR;
using namespace Wasp;

namespace {

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
        path = Wasp::GetAppPath("VAPOR", "share", paths);
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
    _spaceOrTime = "space";

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
    connect(addVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(newVarAdded(int)));
    connect(removeVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(removeVar(int)));
    connect(dataMgrCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(reinitDataMgr()));
#ifdef GETPOINTFROMRENDERER
    connect(timeCopyCombo, SIGNAL(activated(int)),
            this, SLOT(getPointFromRenderer()));
    connect(spaceP1CopyCombo, SIGNAL(activated(int)),
            this, SLOT(getPointFromRenderer()));
    connect(spaceP2CopyCombo, SIGNAL(activated(int)),
            this, SLOT(getPointFromRenderer()));
#endif
    connect(refCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(refinementChanged(int)));
    connect(cRatioCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(cRatioChanged(int)));
    for (int i = 0; i < 4; i++) {
        connect(_spaceCheckBoxes[i], SIGNAL(stateChanged(int)),
                this, SLOT(constCheckboxChanged(int)));
        if (i > 2) {
            connect(_timeCheckBoxes[i], SIGNAL(stateChanged(int)),
                    this, SLOT(constCheckboxChanged(int)));
        }
    }
    connect(spaceTimeTab, SIGNAL(currentChanged(int)), this,
            SLOT(tabChanged(int)));

    spaceP1CopyLabel->hide();
    spaceP1CopyCombo->hide();
    spaceP2CopyLabel->hide();
    spaceP2CopyCombo->hide();
    timeCopyLabel->hide();
    timeCopyCombo->hide();
}

Plot::~Plot() {
    destroyControllers();
    if (_errMsg)
        delete _errMsg;
    if (_plotImage)
        delete _plotImage;
}

void Plot::destroyControllers() {
    if (_spaceTimeRange)
        delete _spaceTimeRange;
    if (_spaceTimeLineEdit)
        delete _spaceTimeLineEdit;
    if (_spaceTimeSlider)
        delete _spaceTimeSlider;
    if (_spaceTimeCell)
        delete _spaceTimeCell;
    if (_spaceTimeMinLabel)
        delete _spaceTimeMinLabel;
    if (_spaceTimeMaxLabel)
        delete _spaceTimeMaxLabel;

    if (_spaceXRange)
        delete _spaceXRange;
    if (_spaceP1XSlider)
        delete _spaceP1XSlider;
    if (_spaceP2XSlider)
        delete _spaceP2XSlider;
    if (_spaceP1XLineEdit)
        delete _spaceP1XLineEdit;
    if (_spaceP2XLineEdit)
        delete _spaceP2XLineEdit;
    if (_spaceP1XCell)
        delete _spaceP1XCell;
    if (_spaceP2XCell)
        delete _spaceP2XCell;
    if (_spaceP1XMinLabel)
        delete _spaceP1XMinLabel;
    if (_spaceP1XMaxLabel)
        delete _spaceP1XMaxLabel;
    if (_spaceP2XMinLabel)
        delete _spaceP2XMinLabel;
    if (_spaceP2XMaxLabel)
        delete _spaceP2XMaxLabel;

    if (_spaceYRange)
        delete _spaceYRange;
    if (_spaceP1YSlider)
        delete _spaceP1YSlider;
    if (_spaceP2YSlider)
        delete _spaceP2YSlider;
    if (_spaceP1YLineEdit)
        delete _spaceP1YLineEdit;
    if (_spaceP2YLineEdit)
        delete _spaceP2YLineEdit;
    if (_spaceP1YCell)
        delete _spaceP1YCell;
    if (_spaceP2YCell)
        delete _spaceP2YCell;
    if (_spaceP1YMinLabel)
        delete _spaceP1YMinLabel;
    if (_spaceP1YMaxLabel)
        delete _spaceP1YMaxLabel;
    if (_spaceP2YMinLabel)
        delete _spaceP2YMinLabel;
    if (_spaceP2YMaxLabel)
        delete _spaceP2YMaxLabel;

    if (_spaceZRange)
        delete _spaceZRange;
    if (_spaceP1ZSlider)
        delete _spaceP1ZSlider;
    if (_spaceP2ZSlider)
        delete _spaceP2ZSlider;
    if (_spaceP1ZLineEdit)
        delete _spaceP1ZLineEdit;
    if (_spaceP2ZLineEdit)
        delete _spaceP2ZLineEdit;
    if (_spaceP1ZCell)
        delete _spaceP1ZCell;
    if (_spaceP2ZCell)
        delete _spaceP2ZCell;
    if (_spaceP1ZMinLabel)
        delete _spaceP1ZMinLabel;
    if (_spaceP1ZMaxLabel)
        delete _spaceP1ZMaxLabel;
    if (_spaceP2ZMinLabel)
        delete _spaceP2ZMinLabel;
    if (_spaceP2ZMaxLabel)
        delete _spaceP2ZMaxLabel;

    if (_timeTimeRange)
        delete _timeTimeRange;
    if (_timeTimeMinSlider)
        delete _timeTimeMinSlider;
    if (_timeTimeMaxSlider)
        delete _timeTimeMaxSlider;
    if (_timeTimeMinLineEdit)
        delete _timeTimeMinLineEdit;
    if (_timeTimeMaxLineEdit)
        delete _timeTimeMaxLineEdit;
    if (_timeTimeMinCell)
        delete _timeTimeMinCell;
    if (_timeTimeMaxCell)
        delete _timeTimeMaxCell;
    if (_timeTimeMinLabel)
        delete _timeTimeMinLabel;
    if (_timeTimeMaxLabel)
        delete _timeTimeMaxLabel;

    if (_timeXRange)
        delete _timeXRange;
    if (_timeXSlider)
        delete _timeXSlider;
    if (_timeXMinLabel)
        delete _timeXMinLabel;
    if (_timeXMaxLabel)
        delete _timeXMaxLabel;
    if (_timeXLineEdit)
        delete _timeXLineEdit;
    if (_timeXCell)
        delete _timeXCell;

    if (_timeYRange)
        delete _timeYRange;
    if (_timeYSlider)
        delete _timeYSlider;
    if (_timeYMinLabel)
        delete _timeYMinLabel;
    if (_timeYMaxLabel)
        delete _timeYMaxLabel;
    if (_timeYLineEdit)
        delete _timeYLineEdit;
    if (_timeYCell)
        delete _timeYCell;

    if (_timeZRange)
        delete _timeZRange;
    if (_timeZSlider)
        delete _timeZSlider;
    if (_timeZMinLabel)
        delete _timeZMinLabel;
    if (_timeZMaxLabel)
        delete _timeZMaxLabel;
    if (_timeZLineEdit)
        delete _timeZLineEdit;
    if (_timeZCell)
        delete _timeZCell;
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
bool Plot::init() {

    // Strictly one-time
    //
    if (_isInitialized)
        return (false);

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
        return (false);

    _isInitialized = true;
    return (true);
}

// NOT One-time initialization!!! This method is called every time the
// plotting window is launched by vaporgui.
//
// TODO: Handle changes to dm and possibly vwm. I.e. What happens when
// a new data set is loaded? Probably need to make Initialize two
// methods: one to launch the window, and one to be called *only* when
// dm and vwm change
//
void Plot::Initialize(ControlExec *ce, VizWinMgr *vwm) {
    assert(vwm != NULL);
    _vwm = vwm;

    assert(ce != NULL);
    _controlExec = ce;
    DataStatus *ds = _controlExec->getDataStatus();
    vector<string> dataMgrs = ds->GetDataMgrNames();
    _dm = ds->GetDataMgr(dataMgrs[0]);
    assert(_dm != NULL);

    dataMgrCombo->clear();
    for (int i = 0; i < dataMgrs.size(); i++) {
        dataMgrCombo->addItem(QString::fromStdString(dataMgrs[i]));
    }

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
    initExtents(0);
    initCRatios();
    initRefinement();

    // More GUI construction. Should probably go in constructor but it
    // is data dependent.
    //
    initSSCs();

    _params->SetXConst(0);
    _params->SetYConst(0);
    _params->SetZConst(0);
    _params->SetTimeConst(0);

    Update(_params);

    showMe();
}

void Plot::reinitDataMgr() {
    DataStatus *ds = _controlExec->getDataStatus();
    string dmName = dataMgrCombo->currentText().toStdString();
    _dm = ds->GetDataMgr(dmName);

    if (_dm == NULL) {
        string err = "Could not find DataMgr named " + dmName;
        errReport(err);
    } else {
        variablesTable->clearContents();
        int numOfRows = variablesTable->rowCount();
        for (int i = 0; i < numOfRows; i++)
            variablesTable->removeRow(0);
        initVariables();
    }
}

void Plot::showMe() {
    show();
    raise();
    activateWindow();
}

void Plot::tabChanged(int tab) {
    if (tab == 0) {
        _spaceOrTime = "space";
    } else {
        _spaceOrTime = "time";
    }
    _params->SetSpaceOrTime(_spaceOrTime);
}

void Plot::initCRatios() {
    _cRatios = _dm->GetCRatios(_vars[0]);

    cRatioCombo->blockSignals(true);
    cRatioCombo->clear();

    for (std::vector<size_t>::iterator it = _cRatios.begin(); it != _cRatios.end(); ++it) {
        cRatioCombo->addItem("1:" + QString::number(*it));
    }

    if (_params->GetCRatio() == -1) {
        _cRatio = _cRatios.size() - 1;
        cRatioCombo->setCurrentIndex(_cRatio);
        _params->SetCRatio(_cRatio);
    }
    cRatioCombo->blockSignals(false);
}

void Plot::initRefinement() {
    refCombo->blockSignals(true);

    refCombo->clear();
    _refLevel = _dm->GetNumRefLevels(_vars[0]);
    for (int i = 0; i <= _refLevel; i++) {
        refCombo->addItem(QString::number(i));
    }

    if (_params->GetRefinement() == -1) {
        refCombo->setCurrentIndex(_refLevel);
        _params->SetRefinement(_refLevel);
    }

    refCombo->blockSignals(false);
}

void Plot::initSSCs() {
    _spaceTimeRange = new Range(_timeExtents[0], _timeExtents[1]);
    _spaceTimeRange->setConst(true);
    _spaceTimeSlider = new TimeSlider(_spaceTimeRange, spaceTimeSlider);
    _spaceTimeLineEdit = new SinglePointLineEdit(_spaceTimeRange, spaceTimeEdit, 0);
    _spaceTimeCell = new MinMaxTableCell(_spaceTimeRange, spaceTable, 0, 3, 0);
    _spaceTimeMinLabel = new MinMaxLabel(_spaceTimeRange, spaceTimeMinLabel, 0);
    _spaceTimeMaxLabel = new MinMaxLabel(_spaceTimeRange, spaceTimeMaxLabel, 1);
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
    _timeXRange->setUserMin((_extents[0] + _extents[3]) / 2.f);

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
    _timeYRange->setUserMin((_extents[1] + _extents[4]) / 2.f);

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
    _timeZRange->setUserMin((_extents[2] + _extents[5]) / 2.f);

    connect(_spaceXRange, SIGNAL(valueChanged()), this, SLOT(spaceRangeChanged()));
    connect(_spaceYRange, SIGNAL(valueChanged()), this, SLOT(spaceRangeChanged()));
    connect(_spaceZRange, SIGNAL(valueChanged()), this, SLOT(spaceRangeChanged()));
    connect(_timeXRange, SIGNAL(valueChanged()), this, SLOT(timeRangeChanged()));
    connect(_timeYRange, SIGNAL(valueChanged()), this, SLOT(timeRangeChanged()));
    connect(_timeZRange, SIGNAL(valueChanged()), this, SLOT(timeRangeChanged()));
    connect(_timeTimeRange, SIGNAL(valueChanged()), this, SLOT(timeTimesChanged()));
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

    connect(_plotButton, SIGNAL(clicked()), this, SLOT(savePlotToFile()));
}

void Plot::savePlotToFile() {
    QFileDialog *fd = new QFileDialog;
    QString defaultOpenLocation = QDir::homePath();
    QString f = fd->getSaveFileName(this, "Select Output File Name",
                                    defaultOpenLocation, tr("*.png"));
    string fileName = f.toStdString();
    delete fd;

    //See if it ends with "png":
    //
    QFileInfo *fileInfo = new QFileInfo(f);
    if (fileInfo->suffix() != "png") {
        fileName.append(".png");

        // Verify if we're overwriting existing files
        //
        if (std::ifstream(fileName.c_str())) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Are you sure?");
            msgBox.setText("Target output file already exists. Do you want to overwrite?");
            msgBox.setStandardButtons(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            if (msgBox.exec() == QMessageBox::No) {
                return;
            }
        }
    }
    if (fileInfo)
        delete fileInfo;

    // Check to see if file is writable
    //
    if (!fileName.empty()) {
        std::ifstream src(_defaultImageLocation.c_str(), std::ios::binary);
        std::ofstream dest(fileName.c_str(), std::ios::binary);
        if (dest.fail()) {
            std::ostringstream ss;
            ss << "Failed to save file " << fileName << " for writing.";
            string myErr = ss.str();
            errReport(myErr);
        } else
            dest << src.rdbuf();

        dest.close();
        src.close();
    }
}

// Our sampling rate is based on how many voxels are crossed
// between points a and b, multiplied by 2
//
int Plot::findNyquist(VAPoR::Grid *sg,
                      const double minu[3], const double maxu[3],
                      double &dX, double &dY, double &dZ) const {

    vector<size_t> dims = sg->GetDimensions();

    int s1, s2, s3;
    s1 = (int)dims[0];
    ;
    s2 = (int)dims[1];
    if (dims[2]) {
        s3 = (int)dims[2];
    } else {
        s3 = 0;
    }
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
    // For some reasons it returns empty strings.
    // Detect and erase them!
    for (vector<string>::iterator it = enabledVars.begin(); it != enabledVars.end(); ++it)
        if (it->empty())
            enabledVars.erase(it);
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
    if (_spaceOrTime == "space") {

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

    PyObject *pFunc = Wasp::MyPython::CreatePyFunc(
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

#ifdef DEAD
// Grow selected voxel region by one voxel if possible. This is a
// a workaround for poor design choice in DataMgr
//
void Plot::fudgeVoxBounds(
    size_t minv[3], size_t maxv[3]) const {
    size_t dims[3];
    _dm->GetDim(dims, _refLevel);

    for (int i = 0; i < 3; i++) {
        if (minv[i] > 0)
            minv[i]--;
        if (maxv[i] < dims[i] - 1)
            maxv[i]++;
    }
}
#endif

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
    double minu[3], double maxu[3], size_t &ts) const {

    minu[0] = _spaceXRange->getUserMin();
    maxu[0] = _spaceXRange->getUserMax();
    minu[1] = _spaceYRange->getUserMin();
    maxu[1] = _spaceYRange->getUserMax();
    minu[2] = _spaceZRange->getUserMin();
    maxu[2] = _spaceZRange->getUserMax();

    // Swap if needed
    //
    for (int i = 0; i < 3; i++) {
        if (minu[i] > maxu[i]) {
            double tmp = minu[i];
            minu[i] = maxu[i];
            maxu[i] = tmp;
        }
    }

    ts = _spaceTimeRange->getUserMin();
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
    double minu[3], maxu[3];
    size_t ts;
    getSpatialExtents(minu, maxu, ts);

    // Get bounding box for data in voxel coordiantes
    //
    size_t minv[3], maxv[3];

#ifdef DEAD
    _dm->MapUserToVox(ts, minu, minv, _refLevel, _cRatio);
    _dm->MapUserToVox(ts, maxu, maxv, _refLevel, _cRatio);

    // Ugh. Need to try to grow the voxel bounds otherwise boundary
    // points may not be inside of the returned RegularGrid. Sigh :-(
    //
    fudgeVoxBounds(minv, maxv);
#endif

    double dX, dY, dZ;
    int nsamples = 0;
    for (int i = 0; i < vars.size(); i++) {
        string var = vars[i];
        vector<float> vec;

        vector<double> minUVec, maxUVec;
        minUVec.push_back(minu[0]);
        minUVec.push_back(minu[1]);
        minUVec.push_back(minu[2]);
        maxUVec.push_back(maxu[0]);
        maxUVec.push_back(maxu[1]);
        maxUVec.push_back(maxu[2]);

        Grid *sg = _dm->GetVariable(
            ts, var, _refLevel, _cRatio, minUVec, maxUVec, false);
        if (!sg) {
            // N.B. In VAPOR 2.x libvdc functions post their own
            // error popups!
            //
            return (-1);
        }

        // Use first RG to figure out how many samples we need,
        // and step size. Only do this with first variable. All
        // variables sampled same number of times
        //
        if (!nsamples) {
            //nsamples = findNyquist(minv, maxv, minu, maxu, dX, dY, dZ);
            nsamples = findNyquist(sg, minu, maxu, dX, dY, dZ);
        }

        float mv = sg->GetMissingValue();
        for (int j = 0; j < nsamples; ++j) {
            double xCoord = j * dX + minu[0];
            double yCoord = j * dY + minu[1];
            double zCoord = j * dZ + minu[2];
            float val = sg->GetValue(xCoord, yCoord, zCoord);
            // Map missing values to NaNs. matplotlib won't plot
            // these. May need to using a numpy masked array in future.
            //
            if (val == mv) {
                val = std::numeric_limits<double>::quiet_NaN();
            }

            vec.push_back(val);
        }

        if (sg)
            delete sg;

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
    double xyz[3], size_t &ts0, size_t &ts1) const {
    ts0 = _timeTimeRange->getUserMin();
    ts1 = _timeTimeRange->getUserMax();

    if (ts0 > ts1) {
        size_t tmp = ts0;
        ts0 = ts1;
        ts1 = tmp;
    }

    xyz[0] = _timeXRange->getUserMin();
    xyz[1] = _timeYRange->getUserMin();
    xyz[2] = _timeZRange->getUserMin();
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
    double xyz[3];
    size_t ts0, ts1;
    getTemporalExtents(xyz, ts0, ts1);

    for (int i = 0; i < vars.size(); i++) {
        string var = vars[i];
        vector<float> vec;

        for (size_t ts = ts0; ts <= ts1; ts++) {

            size_t ijk[3];
#ifdef DEAD
            _dm->MapUserToVox(ts, xyz, ijk, _refLevel, _cRatio);

            // Ugh. Need to try to grow the voxel bounds otherwise boundary
            // points may not be inside of the returned RegularGrid. Sigh :-(
            //
            //			size_t minv[3] = {ijk[0], ijk[1],ijk[2]};
            //			size_t maxv[3] = {ijk[0], ijk[1],ijk[2]};
            fudgeVoxBounds(minv, maxv);
#endif
            size_t minv[3] = {ijk[0], ijk[1], ijk[2]};
            size_t maxv[3] = {ijk[0], ijk[1], ijk[2]};

            //			Grid *sg = _dm->GetVariable(
            //				ts, var, _refLevel, _cRatio, minv, maxv, false
            //			);
            vector<double> uPoint;
            uPoint.push_back(xyz[0]);
            uPoint.push_back(xyz[1]);
            uPoint.push_back(xyz[2]);
            Grid *sg = _dm->GetVariable(
                ts, var, _refLevel, _cRatio, uPoint, uPoint, false);
            if (!sg) {
                // N.B. In VAPOR 2.x libvdc functions post their own
                // error popups!
                //
                return (-1);
            }

            float val = sg->GetValue(xyz[0], xyz[1], xyz[2]);

            // TODO: Handle missing values
            //
            vec.push_back(val);

            delete sg;
        }
        data[var] = vec;
    }

    vector<float> vec;
    vector<double> timeCoords;
    _dm->GetTimeCoordinates(timeCoords);
    for (size_t ts = ts0; ts <= ts1; ts++) {
        vec.push_back(timeCoords[ts]);
    }

    iData["X"] = vec;
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

#ifdef GETPOINTFROMRENDERER
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

void Plot::updateSpaceTimeTabs() {
    _spaceOrTime = _params->GetSpaceOrTime();
    spaceTimeTab->blockSignals(true);
    if (_spaceOrTime == "space") {
        spaceTimeTab->setCurrentIndex(0);
    } else {
        spaceTimeTab->setCurrentIndex(1);
    }
    spaceTimeTab->blockSignals(false);
}

void Plot::updateRanges() {
    vector<double> minSpace, maxSpace, timeExts;
    minSpace = _params->GetSpaceMinExtents();
    maxSpace = _params->GetSpaceMaxExtents();

    _spaceXRange->blockSignals(true);
    _spaceYRange->blockSignals(true);
    _spaceZRange->blockSignals(true);
    _spaceXRange->setUserMin(minSpace[0]);
    _spaceYRange->setUserMin(minSpace[1]);
    _spaceZRange->setUserMin(minSpace[2]);
    _spaceXRange->setUserMax(maxSpace[0]);
    _spaceYRange->setUserMax(maxSpace[1]);
    _spaceZRange->setUserMax(maxSpace[2]);
    _spaceXRange->blockSignals(false);
    _spaceYRange->blockSignals(false);
    _spaceZRange->blockSignals(false);

    _timeXRange->blockSignals(true);
    _timeYRange->blockSignals(true);
    _timeZRange->blockSignals(true);
    timeExts = _params->GetTimePoint();
    _timeXRange->setUserMin(timeExts[0]);
    _timeYRange->setUserMin(timeExts[1]);
    _timeZRange->setUserMin(timeExts[2]);
    _timeXRange->blockSignals(false);
    _timeYRange->blockSignals(false);
    _timeZRange->blockSignals(false);
}

void Plot::updateTimes() {
    int timeMinTS, timeMaxTS;
    timeMinTS = _params->GetTimeMinTS();
    timeMaxTS = _params->GetTimeMaxTS();
    _timeTimeRange->blockSignals(true);
    _timeTimeRange->setUserMin(timeMinTS);
    _timeTimeRange->setUserMax(timeMaxTS);
    _timeTimeRange->blockSignals(false);

    int spaceTS = _params->GetSpaceTS();
    _spaceTimeRange->blockSignals(true);
    _spaceTimeRange->setUserMin(spaceTS);
    _spaceTimeRange->blockSignals(false);
}

void Plot::updateVariables() {
    // Clear and regenerate the variable table,
    // and its associated combo boxes
    //
    vector<string> vars = _params->GetVarNames();
    removeVarCombo->clear();
    removeVarCombo->addItem("Remove Variable:");

    for (int i = 0; i < _uVars.size(); i++) {
        variablesTable->removeRow(0);
    }
    _uVars.clear();
    for (int i = 0; i < vars.size(); i++) {
        string var = vars[i];
        int index = addVarCombo->findText(QString::fromStdString(var));
        newVarAdded(index);
    }
}

void Plot::updateRefCRatio() {
    int refIndex = _params->GetRefinement();
    refCombo->setCurrentIndex(refIndex);

    int cRatioIndex = _params->GetCRatio();
    cRatioCombo->setCurrentIndex(cRatioIndex);
}

void Plot::updateConstCheckboxes() {
    for (int i = 0; i < 3; i++) {
        _spaceCheckBoxes[i]->blockSignals(true);
    }
    _timeCheckBoxes[3]->blockSignals(true);

    bool xConst = _params->GetXConst();
    _spaceXRange->setConst(xConst);
    if (xConst)
        _spaceCheckBoxes[0]->setCheckState(Qt::Checked);
    else
        _spaceCheckBoxes[0]->setCheckState(Qt::Unchecked);

    bool yConst = _params->GetYConst();
    constCheckboxChanged(yConst);
    if (yConst)
        _spaceCheckBoxes[1]->setCheckState(Qt::Checked);
    else
        _spaceCheckBoxes[1]->setCheckState(Qt::Unchecked);

    bool zConst = _params->GetZConst();
    constCheckboxChanged(zConst);
    if (zConst)
        _spaceCheckBoxes[2]->setCheckState(Qt::Checked);
    else
        _spaceCheckBoxes[2]->setCheckState(Qt::Unchecked);

    for (int i = 0; i < 3; i++) {
        _spaceCheckBoxes[i]->blockSignals(false);
    }
    _timeCheckBoxes[3]->blockSignals(false);

    bool tConst = _params->GetTimeConst();
    constCheckboxChanged(tConst);
    if (tConst)
        _timeCheckBoxes[3]->setCheckState(Qt::Checked);
    else
        _timeCheckBoxes[3]->setCheckState(Qt::Unchecked);
}

void Plot::Update(VAPoR::PlotParams *pParams) {
    _params = pParams;

    updateSpaceTimeTabs();
    updateRanges();
    updateTimes();
    updateVariables();
    updateRefCRatio();
    updateConstCheckboxes();
}

void Plot::spaceRangeChanged() {
    vector<double> spaceMinExts, spaceMaxExts;
    //	_params = (PlotParams*)pMgr->GetParams("PlotParams");
    spaceMinExts.push_back(_spaceXRange->getUserMin());
    spaceMinExts.push_back(_spaceYRange->getUserMin());
    spaceMinExts.push_back(_spaceZRange->getUserMin());
    spaceMaxExts.push_back(_spaceXRange->getUserMax());
    spaceMaxExts.push_back(_spaceYRange->getUserMax());
    spaceMaxExts.push_back(_spaceZRange->getUserMax());

    ParamsMgr *pMgr = _controlExec->GetParamsMgr();
    pMgr->BeginSaveStateGroup("Plot space range changing");
    _params->SetSpaceMaxExtents(spaceMaxExts);
    _params->SetSpaceMinExtents(spaceMinExts);
    pMgr->EndSaveStateGroup();
}

void Plot::timeRangeChanged() {
    vector<double> timeExtents;
    timeExtents.push_back(_timeXRange->getUserMin());
    timeExtents.push_back(_timeYRange->getUserMin());
    timeExtents.push_back(_timeZRange->getUserMin());
    _params->SetTimePoint(timeExtents);
}

void Plot::timeTimesChanged() {
    vector<int> timeTimes;
    int min = _timeTimeRange->getUserMin();
    int max = _timeTimeRange->getUserMax();
    ParamsMgr *pMgr = _controlExec->GetParamsMgr();
    pMgr->BeginSaveStateGroup("Set temporal extents for time plotting");
    _params->SetTimeMinTS(min);
    _params->SetTimeMaxTS(max);
    pMgr->EndSaveStateGroup();
}

void Plot::constCheckboxChanged(int state) {
    QObject *sender = QObject::sender();
    if (sender == _spaceCheckBoxes[0]) {
        _spaceXRange->setConst(state);
        _params->SetXConst((bool)state);
    } else if (sender == _spaceCheckBoxes[1]) {
        _spaceYRange->setConst(state);
        _params->SetYConst((bool)state);
    } else if (sender == _spaceCheckBoxes[2]) {
        _spaceZRange->setConst(state);
        _params->SetZConst((bool)state);
    } else if (sender == _timeCheckBoxes[3]) {
        _timeTimeRange->setConst(state);
        _params->SetTimeConst((bool)state);
    }
}

void Plot::print(bool doSpace) const {
    vector<string>::const_iterator it;

    double minu[3], maxu[3];
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
    _timeExtents.push_back(_dm->GetNumTimeSteps() - 1);
}

void Plot::initExtents(int ts) {
    vector<double> minExts, maxExts;

    int rc = -1;
    if (!_vars3d.empty())
        rc = _dm->GetVariableExtents(ts, _vars3d[0], _refLevel, minExts, maxExts);
    else if (!_vars.empty())
        rc = _dm->GetVariableExtents(ts, _vars[0], _refLevel, minExts, maxExts);
    else {
        // No Valid Variable from this DataMgr!!!
    }

    if (rc < 0) {
        string myErr;
        myErr = "Plot could not find minimum and maximum extents"
                " in current data set.";
    }
    if (_extents.size() < 6) {
        _extents.push_back(minExts[0]);
        _extents.push_back(minExts[1]);
        _extents.push_back(minExts[2]);
        _extents.push_back(maxExts[0]);
        _extents.push_back(maxExts[1]);
        _extents.push_back(maxExts[2]);
    } else {
        _extents[0] = minExts[0];
        _extents[1] = minExts[1];
        _extents[2] = minExts[2];
        _extents[3] = maxExts[0];
        _extents[4] = maxExts[1];
        _extents[5] = maxExts[2];
    }

    ParamsMgr *pMgr = _controlExec->GetParamsMgr();
    pMgr->BeginSaveStateGroup("Initializing plot extent selectors");
    _params->SetSpaceMaxExtents(maxExts);
    _params->SetSpaceMinExtents(minExts);
    _params->SetTimePoint(minExts);
    pMgr->EndSaveStateGroup();
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
    addVarCombo->blockSignals(true);
    _vars.clear();
    _vars3d.clear();
    addVarCombo->clear(); // Clear old variables
    addVarCombo->addItem("Add Variable:");
    vector<string> vars;

    vars = _dm->GetDataVarNames(3);
    for (std::vector<string>::iterator it = vars.begin(); it != vars.end(); ++it) {
        _vars.push_back(*it);
        _vars3d.push_back(*it);
    }
    vars = _dm->GetDataVarNames(2);
    for (std::vector<string>::iterator it = vars.begin(); it != vars.end(); ++it) {
        _vars.push_back(*it);
    }

    sort(_vars.begin(), _vars.end());

    // Add variables to combo box
    for (std::vector<string>::iterator it = _vars.begin(); it != _vars.end(); ++it) {
        addVarCombo->addItem(QString::fromStdString(*it));
    }
    addVarCombo->blockSignals(false);
}

void Plot::newVarAdded(int index) {

    if (index == 0)
        return;
    QString varName = addVarCombo->itemText(index);

    if (std::find(_uVars.begin(), _uVars.end(), varName.toStdString()) != _uVars.end())
        return;

    _uVars.push_back(varName.toStdString());
    int rowCount = variablesTable->rowCount();
    variablesTable->insertRow(rowCount);
    variablesTable->setVerticalHeaderItem(rowCount, new QTableWidgetItem(varName));

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

    if (removeVarCombo->findText(varName) == -1) {
        removeVarCombo->addItem(varName);
    }

    // If all variables are 2D, disable Z axis controllers
    //
    bool varsAre2D = true;
    for (int i = 0; i < _uVars.size(); i++) {
        size_t nDims = _dm->GetVarTopologyDim(_uVars[i]);
        if (nDims == 3) {
            varsAre2D = false;
        }
    }
    varsAre2D ? enableZControllers(false) : enableZControllers(true);

    // If we are adding a var through the gui, set params.
    // Otherwise we are issuing an Update() and should not
    // touch params.
    if (sender()) {
        _params->SetVarNames(_uVars);
    }
}

void Plot::buildVarTable() {}

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
        timeZEdit->setText(QString::number((_extents[2] + _extents[5]) / 2.f));
        _timeZRange->setUserMin((_extents[2] + _extents[5]) / 2.f);
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
    if (index < 1)
        return;
    removeVarCombo->blockSignals(true);
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
    removeVarCombo->blockSignals(false);
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
