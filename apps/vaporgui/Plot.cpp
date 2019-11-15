//************************************************************************
//                                                                      *
//           Copyright (C)  2016                                        *
//     University Corporation for Atmospheric Research                  *
//           All Rights Reserved                                        *
//                                                                      *
//************************************************************************/
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       January 2018
//

#include <vapor/MyPython.h>
#include <vapor/DC.h>

#include <QLineEdit>
#include <QTemporaryFile>
#include "GUIStateParams.h"
#include <vapor/DataMgrUtils.h>
#include "ErrorReporter.h"
#include "Plot.h"
#include "Flags.h"

// Constructor
Plot::Plot(VAPoR::DataStatus *status, VAPoR::ParamsMgr *manager, QWidget *parent) : QDialog(parent), Ui_PlotWindow()
{
    _dataStatus = status;
    _paramsMgr = manager;

    // Get the active dataset name
    std::string              currentDatasetName;
    std::vector<std::string> dmNames = _dataStatus->GetDataMgrNames();
    if (dmNames.empty())
        MSG_ERR("No data set chosen yet. Plot shouldn't run into this condition.");
    else {
        GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
        currentDatasetName = guiParams->GetPlotDatasetName();
        if (currentDatasetName == "" || currentDatasetName == "NULL")    // not initialized yet
        {
            currentDatasetName = dmNames[0];
            guiParams->SetPlotDatasetName(currentDatasetName);
        }
    }

    // Do some static QT stuff
    setupUi(this);
    setWindowTitle("Plot Utility");
    myFidelityWidget->Reinit((VariableFlags)AUXILIARY);
    spaceTimeTab->setCurrentIndex(0);    // default to load space tab

    timeTabSinglePoint->SetMainLabel(QString::fromAscii("Select one data point in space:"));
    timeTabTimeRange->SetMainLabel(QString::fromAscii("Select the minimum and maximum time steps:"));
    timeTabTimeRange->SetIntType(true);

    spaceTabP1->SetMainLabel(QString::fromAscii("Select spatial location of Point 1"));
    spaceTabP2->SetMainLabel(QString::fromAscii("Select spatial location of Point 2"));
    spaceTabTimeSelector->SetLabel(QString::fromAscii("T"));
    spaceTabTimeSelector->SetIntType(true);

    // set widget extents
    _setInitialExtents();
    _validator = new QIntValidator(numOfSamplesLineEdit);
    numOfSamplesLineEdit->setValidator(_validator);

    // Connect signals with slots
    connect(newVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_newVarChanged(int)));
    connect(removeVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_removeVarChanged(int)));
    connect(dataMgrCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_dataSourceChanged(int)));
    connect(timeTabSinglePoint, SIGNAL(pointUpdated()), this, SLOT(_timeModePointChanged()));
    connect(timeTabTimeRange, SIGNAL(rangeChanged()), this, SLOT(_timeModeT1T2Changed()));
    connect(spaceTabTimeSelector, SIGNAL(valueChanged(int)), this, SLOT(_spaceModeTimeChanged(int)));
    connect(spaceTabP1, SIGNAL(pointUpdated()), this, SLOT(_spaceModeP1Changed()));
    connect(spaceTabP2, SIGNAL(pointUpdated()), this, SLOT(_spaceModeP2Changed()));
    connect(spaceTabPlotButton, SIGNAL(clicked()), this, SLOT(_spaceTabPlotClicked()));
    connect(timeTabPlotButton, SIGNAL(clicked()), this, SLOT(_timeTabPlotClicked()));
    connect(numOfSamplesLineEdit, SIGNAL(editingFinished()), this, SLOT(_numberOfSamplesChanged()));
    connect(xlock, SIGNAL(stateChanged(int)), this, SLOT(_axisLocksChanged(int)));
    connect(ylock, SIGNAL(stateChanged(int)), this, SLOT(_axisLocksChanged(int)));
    connect(zlock, SIGNAL(stateChanged(int)), this, SLOT(_axisLocksChanged(int)));

    // Create widgets for the plot window
    _plotDialog = new QDialog(this);
    _plotLabel = new QLabel(this);
    _plotLabel->setText(QString::fromAscii("  Plot is on disk:  "));
    _plotPathEdit = new QLineEdit(this);
    _plotPathEdit->setReadOnly(true);
    _plotPathEdit->setTextMargins(6, 0, 6, 0);
    _plotImageLabel = new QLabel(this);
    _plotLayout = new QVBoxLayout();
    _plotDialog->setLayout(_plotLayout);
    _plotLayout->addWidget(_plotLabel);
    _plotLayout->addWidget(_plotPathEdit);
    _plotLayout->addWidget(_plotImageLabel);

    // Put the current window on top
    show();
    raise();
    activateWindow();
}

// Destructor
Plot::~Plot()
{
    _dataStatus = NULL;
    _paramsMgr = NULL;
}

void Plot::Update()
{
    // Doesn't do anything if the current window isn't visible.
    if (!(this->isVisible())) return;

    // Initialize pointers
    std::vector<std::string> dmNames = _dataStatus->GetDataMgrNames();
    if (dmNames.empty()) {
        this->close();
        return;
    }
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    std::string     currentDatasetName = guiParams->GetPlotDatasetName();
    VAssert(currentDatasetName != "" && currentDatasetName != "NULL");
    int currentIdx = -1;
    for (int i = 0; i < dmNames.size(); i++)
        if (currentDatasetName == dmNames[i]) {
            currentIdx = i;
            break;
        }
    if (currentIdx == -1)    // currentDatasetName is closed!!!
    {
        currentDatasetName = dmNames[0];
        currentIdx = 0;
        guiParams->SetPlotDatasetName(currentDatasetName);
        _setInitialExtents();
    }
    VAPoR::DataMgr *         currentDmgr = _dataStatus->GetDataMgr(currentDatasetName);
    VAPoR::PlotParams *      plotParams = dynamic_cast<VAPoR::PlotParams *>(_paramsMgr->GetAppRenderParams(currentDatasetName, VAPoR::PlotParams::GetClassType()));
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();

    // Update DataMgrCombo
    dataMgrCombo->blockSignals(true);
    dataMgrCombo->clear();
    for (int i = 0; i < dmNames.size(); i++) dataMgrCombo->addItem(QString::fromStdString(dmNames[i]));
    dataMgrCombo->setCurrentIndex(currentIdx);
    dataMgrCombo->blockSignals(false);

    // Update "Add a Variable"
    std::vector<std::string> availVars = currentDmgr->GetDataVarNames();
    for (int i = 0; i < enabledVars.size(); i++)
        for (int rmIdx = 0; rmIdx < availVars.size(); rmIdx++)
            if (availVars[rmIdx] == enabledVars[i]) {
                availVars.erase(availVars.begin() + rmIdx);
                break;
            }
    std::sort(availVars.begin(), availVars.end());
    newVarCombo->blockSignals(true);
    newVarCombo->clear();
    newVarCombo->blockSignals(false);
    newVarCombo->addItem(QString::fromAscii("Add a Variable"));
    for (std::vector<std::string>::iterator it = availVars.begin(); it != availVars.end(); ++it) newVarCombo->addItem(QString::fromStdString(*it));
    newVarCombo->setCurrentIndex(0);

    // Update "Remove a Variable"
    std::sort(enabledVars.begin(), enabledVars.end());
    removeVarCombo->blockSignals(true);
    removeVarCombo->clear();
    removeVarCombo->blockSignals(false);
    removeVarCombo->addItem(QString::fromAscii("Remove a Variable"));
    for (int i = 0; i < enabledVars.size(); i++) removeVarCombo->addItem(QString::fromStdString(enabledVars[i]));
    removeVarCombo->setCurrentIndex(0);

    // Update "Variable Table"
    variablesTable->clear();    // This also deletes the items properly.
    QStringList header;         // Start from the header
    header << "Enabled Variables";
    variablesTable->setColumnCount(header.size());
    variablesTable->setHorizontalHeaderLabels(header);
    variablesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    variablesTable->horizontalHeader()->setFixedHeight(30);
    variablesTable->verticalHeader()->setFixedWidth(30);

    variablesTable->setRowCount(enabledVars.size());    // Then work on the cells
    for (int row = 0; row < enabledVars.size(); row++) {
        QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(enabledVars[row]));
        item->setFlags(Qt::NoItemFlags);
        item->setTextAlignment(Qt::AlignCenter);
        variablesTable->setItem(row, 0, item);
    }
    variablesTable->update();
    variablesTable->repaint();
    variablesTable->viewport()->update();

    // If there are variables selected, update the extents based on the selected variables.
    if (enabledVars.size() > 0) {
        std::vector<double> min, max;
        std::vector<int>    axes;

        // First update the space tab
        min = plotParams->GetMinExtents();
        max = plotParams->GetMaxExtents();
        size_t dimensionality = min.size();

        spaceTabP1->SetDimensionality(dimensionality);
        spaceTabP2->SetDimensionality(dimensionality);
        spaceTabP1->SetExtents(min, max);
        spaceTabP2->SetExtents(min, max);

        std::vector<double> pt = plotParams->GetPoint1();
        if (pt.size() == 0)    // 1st time open Plot
            pt = min;
        else if (pt.size() == 2 && dimensionality == 3)
            pt.push_back(min.at(2));
        else if (pt.size() == 3 && dimensionality == 2)
            pt.pop_back();

        spaceTabP1->SetValue(pt);
        plotParams->SetPoint1(pt);

        pt = plotParams->GetPoint2();
        if (pt.size() == 0)
            pt = max;
        else if (pt.size() == 2 && dimensionality == 3)
            pt.push_back(max.at(2));
        else if (pt.size() == 3 && dimensionality == 2)
            pt.pop_back();

        spaceTabP2->SetValue(pt);
        plotParams->SetPoint2(pt);

        // Second update the time tab
        timeTabSinglePoint->SetDimensionality(dimensionality);
        timeTabSinglePoint->SetExtents(min, max);

        pt = plotParams->GetSinglePoint();
        if (pt.size() == 0) {
            for (size_t i = 0; i < min.size(); i++) pt.push_back(min.at(i) + 0.5 * (max.at(i) - min.at(i)));
        } else if (pt.size() == 2 && dimensionality == 3)
            pt.push_back(min.at(2) + 0.5 * (max.at(2) - min.at(2)));
        else if (pt.size() == 3 && dimensionality == 2)
            pt.pop_back();
        timeTabSinglePoint->SetValue(pt);
        plotParams->SetSinglePoint(pt);
    }

    // Update LOD, Refinement
    myFidelityWidget->Update(currentDmgr, _paramsMgr, plotParams);

    // Update time dimension
    spaceTabTimeSelector->SetValue(plotParams->GetCurrentTimestep());
    std::vector<long> range = plotParams->GetMinMaxTS();
    VAssert(range.size() > 0);
    timeTabTimeRange->SetValue((double)range[0], (double)range[1]);

    // Update number of samples
    numOfSamplesLineEdit->setText(QString::number(plotParams->GetNumOfSamples(), 10));

    // Update axis locks
    std::vector<bool> locks = plotParams->GetAxisLocks();
    xlock->setChecked(locks[0]);
    ylock->setChecked(locks[1]);
    zlock->setChecked(locks[2]);
    if (spaceTabP1->GetDimensionality() == 3)
        zlock->setVisible(true);
    else
        zlock->setVisible(false);
    _spaceModeP1Changed();    // if any axis is locked, this function syncs them
}

void Plot::_newVarChanged(int index)
{
    if (index == 0)    // not selecting any variable
        return;

    std::string varName = newVarCombo->itemText(index).toStdString();

    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *   dataMgr = this->_getCurrentDataMgr();
    int                refinementLevel = plotParams->GetRefinementLevel();
    int                compressLevel = plotParams->GetCompressionLevel();
    int                currentTS = (int)plotParams->GetCurrentTimestep();

    // Test if the selected variable available at the specific time step,
    //   compression level, etc.
    if (!dataMgr->VariableExists(currentTS, varName, refinementLevel, compressLevel)) {
        MSG_WARN("Selected variable not available at this settings!");
        newVarCombo->setCurrentIndex(0);
        return;
    } else    // Add this variable to parameter
    {
        std::vector<std::string> vars = plotParams->GetAuxVariableNames();
        vars.push_back(varName);
        plotParams->SetAuxVariableNames(vars);
        _updateExtents();
    }
}

void Plot::_removeVarChanged(int index)
{
    if (index == 0) return;

    std::string        varName = removeVarCombo->itemText(index).toStdString();
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();

    // Remove this variable from parameter
    std::vector<std::string> vars = plotParams->GetAuxVariableNames();
    int                      rmIdx = -1;
    for (int i = 0; i < vars.size(); i++)
        if (vars[i] == varName) {
            rmIdx = i;
            break;
        }
    VAssert(rmIdx != -1);
    vars.erase(vars.begin() + rmIdx);
    plotParams->SetAuxVariableNames(vars);
}

void Plot::_spaceModeP1Changed()
{
    std::vector<double> pt, pt2;
    spaceTabP1->GetCurrentPoint(pt);
    VAssert(pt.size() == 2 || pt.size() == 3);

    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    plotParams->SetPoint1(pt);

    spaceTabP2->GetCurrentPoint(pt2);
    std::vector<bool> locks = plotParams->GetAxisLocks();
    bool              lockFlag = false;
    if (locks[0]) {
        pt2[0] = pt[0];
        lockFlag = true;
    }
    if (locks[1]) {
        pt2[1] = pt[1];
        lockFlag = true;
    }
    if (spaceTabP1->GetDimensionality() == 3 && locks[2]) {
        pt2[2] = pt[2];
        lockFlag = true;
    }
    if (lockFlag) {
        spaceTabP2->SetValue(pt2);
        plotParams->SetPoint2(pt2);
    }
}

void Plot::_spaceModeP2Changed()
{
    std::vector<double> pt, pt1;
    spaceTabP2->GetCurrentPoint(pt);
    VAssert(pt.size() == 2 || pt.size() == 3);

    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    plotParams->SetPoint2(pt);

    spaceTabP1->GetCurrentPoint(pt1);
    std::vector<bool> locks = plotParams->GetAxisLocks();
    bool              lockFlag = false;
    if (locks[0]) {
        pt1[0] = pt[0];
        lockFlag = true;
    }
    if (locks[1]) {
        pt1[1] = pt[1];
        lockFlag = true;
    }
    if (spaceTabP2->GetDimensionality() == 3 && locks[2]) {
        pt1[2] = pt[2];
        lockFlag = true;
    }
    if (lockFlag) {
        spaceTabP1->SetValue(pt1);
        plotParams->SetPoint1(pt1);
    }
}

void Plot::_spaceModeTimeChanged(int val)
{
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    plotParams->SetCurrentTimestep(val);
    std::string emptyStr;
    _updateExtents();
}

void Plot::_timeModePointChanged()
{
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();

    std::vector<double> currentPoint;
    timeTabSinglePoint->GetCurrentPoint(currentPoint);
    VAssert(currentPoint.size() == 2 || currentPoint.size() == 3);

    plotParams->SetSinglePoint(currentPoint);
}

void Plot::_timeModeT1T2Changed()
{
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();

    double smallVal, bigVal;
    timeTabTimeRange->GetValue(smallVal, bigVal);
    std::vector<long int> rangeInt;
    rangeInt.push_back((long int)smallVal);
    rangeInt.push_back((long int)bigVal);

    plotParams->SetMinMaxTS(rangeInt);
    std::string emptyStr;
    _updateExtents();
}

void Plot::_dataSourceChanged(int index)
{
    std::string newDataSourceName = dataMgrCombo->itemText(index).toStdString();

    // Inform GUIStateParams the change of data source.
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));

    guiParams->SetPlotDatasetName(newDataSourceName);

    _setInitialExtents();
}

VAPoR::PlotParams *Plot::_getCurrentPlotParams() const
{
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    std::string     dsName = guiParams->GetPlotDatasetName();
    return (dynamic_cast<VAPoR::PlotParams *>(_paramsMgr->GetAppRenderParams(dsName, VAPoR::PlotParams::GetClassType())));
}

VAPoR::DataMgr *Plot::_getCurrentDataMgr() const
{
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    std::string     currentDatasetName = guiParams->GetPlotDatasetName();
    VAssert(currentDatasetName != "" && currentDatasetName != "NULL");

    return (_dataStatus->GetDataMgr(currentDatasetName));
}

void Plot::_setInitialExtents()
{
    // Set spatial extents
    std::vector<double> minActiveExtents, maxActiveExtents;
    _dataStatus->GetActiveExtents(_paramsMgr, 0, minActiveExtents, maxActiveExtents);
    int dimensionality = minActiveExtents.size();
    if (dimensionality < 2) return;
    _getCurrentPlotParams()->SetMinExtents(minActiveExtents);
    _getCurrentPlotParams()->SetMaxExtents(maxActiveExtents);

    int                   numOfTimeSteps = this->_getCurrentDataMgr()->GetNumTimeSteps();
    std::vector<long int> rangeInt;
    rangeInt.push_back((long int)0);
    rangeInt.push_back((long int)(numOfTimeSteps - 1));
    _getCurrentPlotParams()->SetMinMaxTS(rangeInt);

    // Set space tab extents
    spaceTabP1->SetDimensionality(dimensionality);
    spaceTabP2->SetDimensionality(dimensionality);
    spaceTabP1->SetExtents(minActiveExtents, maxActiveExtents);
    spaceTabP2->SetExtents(minActiveExtents, maxActiveExtents);
    spaceTabP1->SetValue(minActiveExtents);
    spaceTabP2->SetValue(maxActiveExtents);
    spaceTabTimeSelector->SetExtents(0.0, (double)(numOfTimeSteps - 1));
    spaceTabTimeSelector->SetValue(0.0);

    // Set time tab extents
    timeTabSinglePoint->SetDimensionality(dimensionality);
    timeTabSinglePoint->SetExtents(minActiveExtents, maxActiveExtents);
    timeTabSinglePoint->SetValue(minActiveExtents);
    timeTabTimeRange->SetExtents(0.0, (double)(numOfTimeSteps - 1));
}

void Plot::_spaceTabPlotClicked()
{
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *   dataMgr = this->_getCurrentDataMgr();

    int                      refinementLevel = plotParams->GetRefinementLevel();
    int                      compressLevel = plotParams->GetCompressionLevel();
    int                      currentTS = (int)plotParams->GetCurrentTimestep();
    std::vector<double>      point1 = plotParams->GetPoint1();
    std::vector<double>      point2 = plotParams->GetPoint2();
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();
    int                      numOfSamples = plotParams->GetNumOfSamples();

    // Do nothing if no variable is enabled
    if (enabledVars.size() == 0) return;

    std::vector<double> p1p2span;
    for (int i = 0; i < point1.size(); i++) p1p2span.push_back(point2[i] - point1[i]);

    std::vector<std::vector<float>> sequences;
    for (int v = 0; v < enabledVars.size(); v++) {
        std::vector<float> seq(numOfSamples, 0.0);
        VAPoR::Grid *      grid = dataMgr->GetVariable(currentTS, enabledVars[v], refinementLevel, compressLevel);
        if (grid) {
            float missingVal = grid->GetMissingValue();
            for (int i = 0; i < numOfSamples; i++) {
                std::vector<double> sample;
                if (i == 0)
                    sample = point1;
                else if (i == numOfSamples - 1)
                    sample = point2;
                else {
                    for (int j = 0; j < point1.size(); j++) sample.push_back((double)i / (double)(numOfSamples - 1) * p1p2span[j] + point1[j]);
                }
                float fieldVal = grid->GetValue(sample);
                if (fieldVal == missingVal)
                    seq[i] = std::nanf("1");
                else
                    seq[i] = fieldVal;
            }
            sequences.push_back(seq);
        }

        delete grid;
    }

    // Decide X label and values
    std::string        xLabel = _getXLabel();
    std::vector<float> xValues;
    if (!xLabel.empty())    // If xLabel isn't empty, we calculate the actual distances
    {
        float dist = 0.0f;
        for (int i = 0; i < p1p2span.size(); i++) dist += p1p2span[i] * p1p2span[i];
        dist = std::sqrt(dist);
        float stepsize = dist / (float)(numOfSamples - 1);
        for (int i = 0; i < numOfSamples; i++) xValues.push_back((float)i * stepsize);
    } else {
        for (int i = 0; i < numOfSamples; i++) xValues.push_back((float)i);
    }

    // Decide Y label and values
    std::string yLabel = _getYLabel();

    // Call python routines.
    QTemporaryFile file;
    if (file.open()) {
        QString filename = file.fileName() + QString::fromAscii(".png");

        _invokePython(filename, enabledVars, sequences, xValues, xLabel, yLabel);

        QImage plot(filename);
        _plotPathEdit->setText(filename);
        _plotImageLabel->setPixmap(QPixmap::fromImage(plot));
        _plotDialog->show();
        _plotDialog->raise();
        _plotDialog->activateWindow();

        file.close();
    } else
        MSG_ERR("QT temporary file not able to open");
}

void Plot::_timeTabPlotClicked()
{
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *   dataMgr = this->_getCurrentDataMgr();

    int                      refinementLevel = plotParams->GetRefinementLevel();
    int                      compressLevel = plotParams->GetCompressionLevel();
    std::vector<double>      singlePt = plotParams->GetSinglePoint();
    std::vector<long int>    minMaxTS = plotParams->GetMinMaxTS();
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();

    // Do nothing if no variable is enabled
    if (enabledVars.size() == 0) return;

    std::vector<std::vector<float>> sequences;
    for (int v = 0; v < enabledVars.size(); v++) {
        std::vector<float> seq;
        for (int t = minMaxTS[0]; t <= minMaxTS[1]; t++) {
            VAPoR::Grid *grid = dataMgr->GetVariable(t, enabledVars[v], refinementLevel, compressLevel);
            if (grid) {
                float fieldVal = grid->GetValue(singlePt);
                if (fieldVal != grid->GetMissingValue())
                    seq.push_back(fieldVal);
                else
                    seq.push_back(std::nanf("1"));
            }

            delete grid;
        }
        sequences.push_back(seq);
    }

    std::vector<float> xValues;
    if (minMaxTS.size() > 0)
        for (int i = minMaxTS[0]; i <= minMaxTS[1]; i++) xValues.push_back((float)i);

    // Decide Y label and values
    std::string yLabel = _getYLabel();

    // Call python routines.
    QTemporaryFile file;
    if (file.open()) {
        QString filename = file.fileName() + QString::fromAscii(".png");

        const std::string xLabel = "Time Steps";
        _invokePython(filename, enabledVars, sequences, xValues, xLabel, yLabel);

        QImage plot(filename);
        _plotPathEdit->setText(filename);
        _plotImageLabel->setPixmap(QPixmap::fromImage(plot));
        _plotDialog->show();
        _plotDialog->raise();
        _plotDialog->activateWindow();

        file.close();
    } else
        MSG_ERR("QT temporary file not able to open");
}

void Plot::_invokePython(const QString &outFile, const std::vector<std::string> &enabledVars, const std::vector<std::vector<float>> &sequences, const std::vector<float> &xValues,
                         const std::string &xLabel, const std::string &yLabel)
{
    /* Adopted from documentation: https://docs.python.org/2/extending/embedding.html */
    PyObject *pName = NULL;
    PyObject *pModule = NULL;
    PyObject *pFunc = NULL;
    PyObject *pArgs = NULL;
    PyObject *pValue = NULL;
    Wasp::MyPython::Instance()->Initialize();
    VAssert(Py_IsInitialized());

    pName = PyString_FromString("plot");
    pModule = PyImport_Import(pName);

    if (pModule == NULL) {
        MSG_ERR("pModule NULL!!");
        PyErr_Print();
        return;
    }
    pFunc = PyObject_GetAttrString(pModule, "plotSequences");
    if (pFunc && PyCallable_Check(pFunc)) {
        pArgs = PyTuple_New(6);

        // Set the 1st argument: output file name
        pValue = PyString_FromString(outFile.toAscii());
        PyTuple_SetItem(pArgs, 0, pValue);    // pValue is stolen!

        // Set the 2nd argument: variable names
        PyObject *pListOfStrings = PyList_New(enabledVars.size());
        VAssert(pListOfStrings);
        for (int i = 0; i < enabledVars.size(); i++) {
            pValue = PyString_FromString(enabledVars[i].c_str());
            int rt = PyList_SetItem(pListOfStrings, i, pValue);    // pValue is stolen!
            VAssert(rt == 0);
        }
        PyTuple_SetItem(pArgs, 1, pListOfStrings);    // pListOfStrings is stolen!

        // Set the 3rd argument: sequence values (Y axis)
        PyObject *pListOfLists = PyList_New(sequences.size());
        VAssert(pListOfLists);
        for (int i = 0; i < sequences.size(); i++)    // for each sequence
        {
            PyObject *pList = PyList_New(sequences[i].size());
            VAssert(pList);
            for (int j = 0; j < sequences[i].size(); j++) {
                int rt = PyList_SetItem(pList, j, PyFloat_FromDouble(sequences[i][j]));
                VAssert(rt == 0);
            }
            PyList_SetItem(pListOfLists, i, pList);
        }
        PyTuple_SetItem(pArgs, 2, pListOfLists);

        // Set the 4th argument: X axis values
        PyObject *pListOfFloats = PyList_New(xValues.size());
        VAssert(pListOfFloats);
        for (int i = 0; i < xValues.size(); i++) {
            int rt = PyList_SetItem(pListOfFloats, i, PyFloat_FromDouble(xValues[i]));
            VAssert(rt == 0);
        }
        PyTuple_SetItem(pArgs, 3, pListOfFloats);

        // Set the 5th argument: X axis label
        pValue = PyString_FromString(xLabel.c_str());
        PyTuple_SetItem(pArgs, 4, pValue);

        // Set the 6th argument: Y axis label
        pValue = PyString_FromString(yLabel.c_str());
        PyTuple_SetItem(pArgs, 5, pValue);

        pValue = PyObject_CallObject(pFunc, pArgs);
        if (pValue == NULL) {
            MSG_ERR("pFunc failed to execute");
            PyErr_Print();
        }
    } else {
        MSG_ERR("pFunc NULL");
        PyErr_Print();
    }

    Py_XDECREF(pName);
    Py_XDECREF(pArgs);
    Py_XDECREF(pValue);
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
}

void Plot::_numberOfSamplesChanged()
{
    long val = numOfSamplesLineEdit->text().toLong();
    long minSamples = 50;
    if (val < minSamples) {
        val = minSamples;
        numOfSamplesLineEdit->setText(QString::number(val, 10));
    }
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    plotParams->SetNumOfSamples(val);
}

std::string Plot::_getXLabel()
{
    VAPoR::PlotParams *      plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *         dataMgr = this->_getCurrentDataMgr();
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();
    std::vector<std::string> units;

    for (int i = 0; i < enabledVars.size(); i++) {
        VAPoR::DC::DataVar dataVar;
        dataMgr->GetDataVarInfo(enabledVars[i], dataVar);
        std::string     meshName = dataVar.GetMeshName();
        VAPoR::DC::Mesh mesh;
        dataMgr->GetMesh(meshName, mesh);
        std::vector<std::string> coordVarNames = mesh.GetCoordVars();
        for (int j = 0; j < coordVarNames.size(); j++) {
            VAPoR::DC::CoordVar coordVar;
            dataMgr->GetCoordVarInfo(coordVarNames[j], coordVar);
            units.push_back(coordVar.GetUnits());
        }
    }

    std::string empty;
    if (units.size() == 0)
        return empty;
    else {
        std::string label = units[0];
        for (int i = 1; i < units.size(); i++)
            if (units[i] != label) return empty;

        return label;
    }
}

std::string Plot::_getYLabel()
{
    VAPoR::PlotParams *      plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *         dataMgr = this->_getCurrentDataMgr();
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();
    std::vector<std::string> units;

    for (int i = 0; i < enabledVars.size(); i++) {
        VAPoR::DC::DataVar dataVar;
        dataMgr->GetDataVarInfo(enabledVars[i], dataVar);
        units.push_back(dataVar.GetUnits());
    }

    std::string empty;
    if (units.size() == 0)
        return empty;
    else {
        std::string label = units[0];
        for (int i = 1; i < units.size(); i++)
            if (units[i] != label) return empty;

        return label;
    }
}

void Plot::_axisLocksChanged(int val)
{
    std::vector<bool> locks(3, false);
    locks[0] = (bool)xlock->isChecked();
    locks[1] = (bool)ylock->isChecked();
    locks[2] = (bool)zlock->isChecked();

    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    plotParams->SetAxisLocks(locks);
    _spaceModeP1Changed();
}

void Plot::_updateExtents()
{
    VAPoR::DataMgr *         currentDmgr = this->_getCurrentDataMgr();
    VAPoR::PlotParams *      plotParams = this->_getCurrentPlotParams();
    int                      refinementLevel = plotParams->GetRefinementLevel();
    int                      compressLevel = plotParams->GetCompressionLevel();
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();

    // Retrieve extents of all variables at 3 different time steps.
    std::vector<double> min, max, minT1, maxT1, minT2, maxT2;
    std::vector<int>    axes;
    std::vector<long>   TSToExamine;
    TSToExamine.push_back(plotParams->GetCurrentTimestep());
    TSToExamine.push_back(plotParams->GetMinMaxTS().at(0));
    TSToExamine.push_back(plotParams->GetMinMaxTS().at(1));

    // TSToExamine[0] definitely needs to be evaluated.
    VAPoR::DataMgrUtils::GetExtents(currentDmgr, TSToExamine[0], enabledVars, refinementLevel, compressLevel, min, max, axes);

    // TSToExamine[1] and TSToExamine[2] are evaluated only when not duplicate
    if (TSToExamine[1] != TSToExamine[0]) {
        VAPoR::DataMgrUtils::GetExtents(currentDmgr, TSToExamine[1], enabledVars, refinementLevel, compressLevel, minT1, maxT1, axes);
    } else {
        minT1 = min;
        maxT1 = max;
    }
    if ((TSToExamine[2] != TSToExamine[1]) && (TSToExamine[2] != TSToExamine[0])) {
        VAPoR::DataMgrUtils::GetExtents(currentDmgr, TSToExamine[2], enabledVars, refinementLevel, compressLevel, minT2, maxT2, axes);
    } else {
        minT2 = min;
        maxT2 = max;
    }

    // Find the union of the 3 extents
    for (int i = 0; i < min.size(); i++) {
        if (minT1[i] < min[i]) min[i] = minT1[i];
        if (maxT1[i] > max[i]) max[i] = maxT1[i];
        if (minT2[i] < min[i]) min[i] = minT2[i];
        if (maxT2[i] > max[i]) max[i] = maxT2[i];
    }
    plotParams->SetMinExtents(min);
    plotParams->SetMaxExtents(max);
}
