#include "FlowSubtabs.h"
#include "vapor/DataMgrUtils.h"

QVaporSubtab::QVaporSubtab(QWidget *parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->insertSpacing(-1, 20);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}

//
//================================
//
FlowVariablesSubtab::FlowVariablesSubtab(QWidget *parent) : QVaporSubtab(parent)
{
    _variablesWidget = new VariablesWidget(this);
    _variablesWidget->Reinit((VariableFlags)(VECTOR | COLOR), (DimFlags)(THREED));
    _layout->addWidget(_variablesWidget, 0, 0);

    _velocityMltp = new VLineEdit(this, "Field Scale Factor", "1.0");
    _layout->addWidget(_velocityMltp);

    _periodicX = new VCheckBox(this, "Particles periodic in X");
    _layout->addWidget(_periodicX);
    _periodicY = new VCheckBox(this, "Particles periodic in Y");
    _layout->addWidget(_periodicY);
    _periodicZ = new VCheckBox(this, "Particles periodic in Z");
    _layout->addWidget(_periodicZ);

    connect(_velocityMltp, SIGNAL(_editingFinished()), this, SLOT(_velocityMultiplierChanged()));
    connect(_periodicX, SIGNAL(_checkboxClicked()), this, SLOT(_periodicClicked()));
    connect(_periodicY, SIGNAL(_checkboxClicked()), this, SLOT(_periodicClicked()));
    connect(_periodicZ, SIGNAL(_checkboxClicked()), this, SLOT(_periodicClicked()));
}

void FlowVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(rParams);
    assert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);

    // Update custom widgets
    auto mltp = _params->GetVelocityMultiplier();
    _velocityMltp->SetEditText(QString::number(mltp, 'f', 3));

    auto bools = _params->GetPeriodic();
    _periodicX->SetCheckState(bools[0]);
    _periodicY->SetCheckState(bools[1]);
    _periodicZ->SetCheckState(bools[2]);
}

void FlowVariablesSubtab::_periodicClicked()
{
    std::vector<bool> bools(3, false);
    bools[0] = _periodicX->GetCheckState();
    bools[1] = _periodicY->GetCheckState();
    bools[2] = _periodicZ->GetCheckState();
    _params->SetPeriodic(bools);
}

void FlowVariablesSubtab::_velocityMultiplierChanged()
{
    double newval, oldval;
    oldval = _params->GetVelocityMultiplier();
    try {
        newval = std::stod(_velocityMltp->GetEditText());
    } catch (const std::invalid_argument &e) {
        std::cerr << "bad input: " << _velocityMltp->GetEditText() << std::endl;
        _velocityMltp->SetEditText(QString::number(oldval, 'f', 3));
        return;
    }

    if (newval >= 0.001 && newval <= 1000.0)    // in the valid range
    {
        /* std::stod() would convert "3.83aaa" without throwing an exception.
           We set the correct text based on the number identified.        */
        _velocityMltp->SetEditText(QString::number(newval, 'f', 3));
        /* Only write back to _params if newval is different from oldval */
        if (newval != oldval) _params->SetVelocityMultiplier(newval);
    } else
        _velocityMltp->SetEditText(QString::number(oldval, 'f', 3));
}

//
//================================
//
FlowAppearanceSubtab::FlowAppearanceSubtab(QWidget *parent) : QVaporSubtab(parent)
{
    _TFEditor = new TFEditor;

    _layout->addWidget(_TFEditor, 0, 0);

    _params = NULL;
}

void FlowAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(rParams);
    assert(_params);

    _TFEditor->Update(dataMgr, paramsMgr, rParams);
}

//
//================================
//
FlowSeedingSubtab::FlowSeedingSubtab(QWidget *parent) : QVaporSubtab(parent)
{
    _steady = new VCheckBox(this, "Use Steady Flow");
    _layout->addWidget(_steady);
    connect(_steady, SIGNAL(_checkboxClicked()), this, SLOT(_steadyGotClicked()));
    _steadyNumOfSteps = new VLineEdit(this, "Steady Integration Steps", "100");
    _layout->addWidget(_steadyNumOfSteps);
    connect(_steadyNumOfSteps, SIGNAL(_editingFinished()), this, SLOT(_steadyNumOfStepsChanged()));
    _pastNumOfTimeSteps = new VIntSlider(this, "Display Past Num. of Time Steps", 1, 2);
    _layout->addWidget(_pastNumOfTimeSteps);
    connect(_pastNumOfTimeSteps, SIGNAL(_valueChanged(int)), this, SLOT(_pastNumOfTimeStepsChanged(int)));
    _seedInjInterval = new VIntSlider(this, "Seed Injection Interval", 0, 1);
    _layout->addWidget(_seedInjInterval);

    connect(_seedInjInterval, SIGNAL(_valueChanged(int)), this, SLOT(_seedInjIntervalChanged(int)));

    /* Index numbers are in agreement with what's in FlowRenderer.h */
    _flowDirection = new VComboBox(this, "Steady Flow Direction");
    _flowDirection->AddOption("Forward", static_cast<int>(FlowDir::FORWARD));
    _flowDirection->AddOption("Backward", static_cast<int>(FlowDir::BACKWARD));
    _flowDirection->AddOption("Bi-Directional", static_cast<int>(FlowDir::BI_DIR));
    _layout->addWidget(_flowDirection);
    connect(_flowDirection, SIGNAL(_indexChanged(int)), this, SLOT(_flowDirectionChanged(int)));
    _seedGenMode = new VComboBox(this, "Seed Generation Mode");

    _hline1 = new QFrame(this);
    _hline1->setFrameShape(QFrame::HLine);
    _layout->addWidget(_hline1);

    /* The following two widgets deal with flow line output and seed point input */
    _fileWriter = new VFileWriter(this, "Output Flow Lines");
    _fileWriter->SetFileFilter(QString::fromAscii("*.txt"));
    _layout->addWidget(_fileWriter);
    connect(_fileWriter, SIGNAL(_pathChanged()), this, SLOT(_fileWriterChanged()));

    _fileReader = new VFileReader(this, "Input Seed File");
    _fileReader->SetFileFilter(QString::fromAscii("*.txt"));
    _layout->addWidget(_fileReader);
    connect(_fileReader, SIGNAL(_pathChanged()), this, SLOT(_fileReaderChanged()));

    _hline2 = new QFrame(this);
    _hline2->setFrameShape(QFrame::HLine);
    _layout->addWidget(_hline2);

    /* Index numbers are in agreement with what's in FlowRenderer.h */
    _seedGenMode->AddOption("From a Rake, Uniformly", static_cast<int>(FlowSeedMode::UNIFORM));
    _seedGenMode->AddOption("From a Rake, Randomly", static_cast<int>(FlowSeedMode::RANDOM));
    _seedGenMode->AddOption("From a Rake, Randomly with Bias", static_cast<int>(FlowSeedMode::RANDOM_BIAS));
    _seedGenMode->AddOption("From a List", static_cast<int>(FlowSeedMode::LIST));
    _layout->addWidget(_seedGenMode);
    connect(_seedGenMode, SIGNAL(_indexChanged(int)), this, SLOT(_seedGenModeChanged(int)));

    /* Set up the Rake selector */
    std::vector<float> geoRange = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0};    // temporary range
    _rake = new VGeometry(this, 3, geoRange);
    _layout->addWidget(_rake);
    connect(_rake, SIGNAL(_geometryChanged()), this, SLOT(_rakeGeometryChanged()));

    /* Set up rake seed number controls */
    _rakeXNum = new VLineEdit(this, "Num. of Seeds in X", "1");
    _rakeYNum = new VLineEdit(this, "Num. of Seeds in Y", "1");
    _rakeZNum = new VLineEdit(this, "Num. of Seeds in Z", "1");
    _rakeTotalNum = new VLineEdit(this, "Total Num. of Seeds", "1");
    _layout->addWidget(_rakeXNum);
    _layout->addWidget(_rakeYNum);
    _layout->addWidget(_rakeZNum);
    _layout->addWidget(_rakeTotalNum);
    connect(_rakeXNum, SIGNAL(_editingFinished()), this, SLOT(_rakeNumOfSeedsChanged()));
    connect(_rakeYNum, SIGNAL(_editingFinished()), this, SLOT(_rakeNumOfSeedsChanged()));
    connect(_rakeZNum, SIGNAL(_editingFinished()), this, SLOT(_rakeNumOfSeedsChanged()));
    connect(_rakeTotalNum, SIGNAL(_editingFinished()), this, SLOT(_rakeNumOfSeedsChanged()));

    _rakeBiasVariable = new VComboBox(this, "Random Bias Variable");
    _rakeBiasStrength = new VSlider(this, "Random Bias Strength", -5.0f, 5.0f);
    _layout->addWidget(_rakeBiasVariable);
    _layout->addWidget(_rakeBiasStrength);
    connect(_rakeBiasVariable, SIGNAL(_indexChanged(int)), this, SLOT(_rakeBiasVariableChanged(int)));
    connect(_rakeBiasStrength, SIGNAL(_valueChanged()), this, SLOT(_rakeBiasStrengthChanged()));

    VAssert(parent);
    connect(parent, SIGNAL(currentChanged(int)), this, SLOT(_selectedTabChanged(int)));
}

void FlowSeedingSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(params);
    _paramsMgr = paramsMgr;
    VAssert(_params);

    bool isSteady = _params->GetIsSteady();
    _steady->SetCheckState(isSteady);
    int steadyNumOfSteps = _params->GetSteadyNumOfSteps();
    _steadyNumOfSteps->SetEditText(QString::number(steadyNumOfSteps));

    /* Update the past num of steps widget */
    int totalNumOfTimeSteps = dataMgr->GetNumTimeSteps();
    _pastNumOfTimeSteps->SetRange(0, totalNumOfTimeSteps - 1);
    int valParams = _params->GetPastNumOfTimeSteps();
    if (valParams < 0)    // initial value, we need to set it to all time steps!
    {
        _pastNumOfTimeSteps->SetCurrentValue(totalNumOfTimeSteps - 1);
        _params->SetPastNumOfTimeSteps(totalNumOfTimeSteps - 1);
    } else {
        _pastNumOfTimeSteps->SetCurrentValue(valParams);
    }

    /* Update the seed injection interval widget */
    _seedInjInterval->SetRange(0, totalNumOfTimeSteps - 1);
    int injIntv = _params->GetSeedInjInterval();
    if (injIntv < 0)    // initial value, we set it to 0
    {
        _seedInjInterval->SetCurrentValue(0);
        _params->SetSeedInjInterval(0);
    } else {
        _seedInjInterval->SetCurrentValue(injIntv);
    }

    /* Update flow direction combo */
    auto dir = _params->GetFlowDirection();
    if (dir >= 0 && dir < _flowDirection->GetNumOfItems())
        _flowDirection->SetIndex(dir);
    else {
        _flowDirection->SetIndex(0);
        _params->SetFlowDirection(0);    // use 0 as the default option
    }

    /* Update seed generation mode combo */
    auto genMod = _params->GetSeedGenMode();
    if (genMod >= 0 && genMod < _seedGenMode->GetNumOfItems())
        _seedGenMode->SetIndex(genMod);
    else {
        _seedGenMode->SetIndex(0);
        _params->SetSeedGenMode(0);    // use 0 as the default option
    }

    _hideShowWidgets();

    /* Update rake range */
    std::vector<double> minExt, maxExt;
    std::vector<int>    axes;
    VAPoR::DataMgrUtils::GetExtents(dataMgr, _params->GetCurrentTimestep(), _params->GetFieldVariableNames(), minExt, maxExt, axes);
    VAssert(minExt.size() == 3 && maxExt.size() == 3);
    std::vector<float> range;
    for (int i = 0; i < 3; i++) {
        range.push_back(float(minExt[i]));
        range.push_back(float(maxExt[i]));
    }
    _rake->SetDimAndRange(3, range);

    /* Update rake values */
    auto rakeVals = _params->GetRake();
    /* In case the user hasn't set the rake, set the current value to be the rake extents,
       plus update the params.  Otherwise, apply the actual rake values. */
    if (std::isnan(rakeVals[0])) {
        _rake->SetCurrentValues(range);
        _params->SetRake(range);
    } else {
        _rake->SetCurrentValues(rakeVals);
    }

    /* Update rake num. of seeds */
    auto rakeNumOfSeeds = _params->GetRakeNumOfSeeds();
    _rakeXNum->SetEditText(QString::number(rakeNumOfSeeds[0]));
    _rakeYNum->SetEditText(QString::number(rakeNumOfSeeds[1]));
    _rakeZNum->SetEditText(QString::number(rakeNumOfSeeds[2]));
    _rakeTotalNum->SetEditText(QString::number(rakeNumOfSeeds[3]));

    /* Update rake random bias variable and strength */
    if (_rakeBiasVariable->GetNumOfItems() < 1)    // Not filled with variables yet
    {
        auto varNames3d = dataMgr->GetDataVarNames(3);
        for (int i = 0; i < varNames3d.size(); i++) _rakeBiasVariable->AddOption(varNames3d[i], i);
        _rakeBiasVariable->SetIndex(0);    // Set the 1st variable name
    }
    auto varParams = _params->GetRakeBiasVariable();
    if (varParams.empty())    // The variable isn't set by the user yet. Let's set it!
    {
        auto varDefault = _rakeBiasVariable->GetCurrentText();
        _params->SetRakeBiasVariable(varDefault);
    } else    // Find the variable and set it in the GUI!
    {
        for (int i = 0; i < _rakeBiasVariable->GetNumOfItems(); i++) {
            auto varName = _rakeBiasVariable->GetItemText(i);
            if (varName.compare(varParams) == 0) {
                _rakeBiasVariable->SetIndex(i);
                break;
            }
        }
    }

    _rakeBiasStrength->SetCurrentValue(_params->GetRakeBiasStrength());

    /* Update input and output file names */
    if (!_params->GetSeedInputFilename().empty()) _fileReader->SetPath(_params->GetSeedInputFilename());
    if (!_params->GetFlowlineOutputFilename().empty()) _fileWriter->SetPath(_params->GetFlowlineOutputFilename());
}

void FlowSeedingSubtab::_pastNumOfTimeStepsChanged(int newVal)
{
    if (newVal != _params->GetPastNumOfTimeSteps()) { _params->SetPastNumOfTimeSteps(newVal); }
}

void FlowSeedingSubtab::_seedInjIntervalChanged(int newVal)
{
    if (newVal != _params->GetSeedInjInterval()) { _params->SetSeedInjInterval(newVal); }
}

#include <QScrollArea>
void FlowSeedingSubtab::_selectedTabChanged(int index)
{
    if (!_paramsMgr) return;

    const QTabWidget *parent = dynamic_cast<QTabWidget *>(sender());
    VAssert(parent);
    const QScrollArea *area = dynamic_cast<QScrollArea *>(parent->widget(index));
    VAssert(area);
    const QWidget *widget = area->widget();

    GUIStateParams *gp = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());

    gp->SetFlowSeedTabActive(widget == this);
}

void FlowSeedingSubtab::_steadyNumOfStepsChanged()
{
    int newval, oldval;
    oldval = (int)_params->GetSteadyNumOfSteps();
    try {
        newval = std::stoi(_steadyNumOfSteps->GetEditText());
    } catch (const std::invalid_argument &e) {
        std::cerr << "bad input: " << _steadyNumOfSteps->GetEditText() << std::endl;
        _steadyNumOfSteps->SetEditText(QString::number(oldval));
        return;
    }

    if (newval >= 0)    // in the valid range
    {
        /* std::stoi() would convert "383aaa" without throwing an exception.
           We set the correct text based on the number identified.        */
        _steadyNumOfSteps->SetEditText(QString::number(newval));
        /* Only write back to _params if newval is different from oldval */
        if (newval != oldval) _params->SetSteadyNumOfSteps(newval);
    } else
        _steadyNumOfSteps->SetEditText(QString::number(oldval));
}

void FlowSeedingSubtab::_steadyGotClicked()
{
    bool userInput = _steady->GetCheckState();
    _params->SetIsSteady(userInput);
}

void FlowSeedingSubtab::_hideShowWidgets()
{
    bool isSteady = _params->GetIsSteady();
    if (isSteady) {
        _steadyNumOfSteps->show();
        _flowDirection->show();
        _seedInjInterval->hide();
        _pastNumOfTimeSteps->hide();
    } else {
        _steadyNumOfSteps->hide();
        _flowDirection->hide();
        _seedInjInterval->show();
        _pastNumOfTimeSteps->show();
    }

    int genMod = _params->GetSeedGenMode();    // genMod must be valid at this point
    if (genMod == static_cast<int>(FlowSeedMode::UNIFORM)) {
        _rake->show();
        _rakeXNum->show();
        _rakeYNum->show();
        _rakeZNum->show();
        _rakeTotalNum->hide();
        _rakeBiasVariable->hide();
        _rakeBiasStrength->hide();
    } else if (genMod == static_cast<int>(FlowSeedMode::RANDOM)) {
        _rake->show();
        _rakeXNum->hide();
        _rakeYNum->hide();
        _rakeZNum->hide();
        _rakeTotalNum->show();
        _rakeBiasVariable->hide();
        _rakeBiasStrength->hide();
    } else if (genMod == static_cast<int>(FlowSeedMode::RANDOM_BIAS)) {
        _rake->show();
        _rakeXNum->hide();
        _rakeYNum->hide();
        _rakeZNum->hide();
        _rakeTotalNum->show();
        _rakeBiasVariable->show();
        _rakeBiasStrength->show();
    } else if (genMod == static_cast<int>(FlowSeedMode::LIST)) {
        _rake->hide();
        _rakeXNum->hide();
        _rakeYNum->hide();
        _rakeZNum->hide();
        _rakeTotalNum->hide();
        _rakeBiasVariable->hide();
        _rakeBiasStrength->hide();
    }
}

void FlowSeedingSubtab::_rakeBiasVariableChanged(int idx)
{
    // idx is always a valid value, since it's returned by the GUI
    auto varGUI = _rakeBiasVariable->GetCurrentText();
    auto varParams = _params->GetRakeBiasVariable();
    if (varGUI.compare(varParams) != 0) { _params->SetRakeBiasVariable(varGUI); }
}

void FlowSeedingSubtab::_rakeBiasStrengthChanged()
{
    // The value returned from the GUI is always valid
    auto strenGUI = _rakeBiasStrength->GetCurrentValue();
    if (strenGUI != _params->GetRakeBiasStrength()) {
        _rakeBiasStrength->SetCurrentValue(strenGUI);
        _params->SetRakeBiasStrength(strenGUI);
    }
}

void FlowSeedingSubtab::_rakeNumOfSeedsChanged()
{
    /* These fields should ALWAYS contain legal values, even when not in use.
       That's why we validate every one of them!                           */

    const std::vector<long> oldVal = _params->GetRakeNumOfSeeds();
    std::vector<long>       newVal(4, -1);

    std::vector<VLineEdit *> pointers = {_rakeXNum, _rakeYNum, _rakeZNum, _rakeTotalNum};
    for (int i = 0; i < 4; i++) {
        long tmp;
        try {
            tmp = std::stol(pointers[i]->GetEditText());
        } catch (const std::invalid_argument &e)    // If not a long number
        {
            std::cerr << "bad input: " << pointers[i]->GetEditText() << std::endl;
            newVal[i] = oldVal[i];
            pointers[i]->SetEditText(QString::number(oldVal[i]));
            continue;
        }

        if (tmp > 0)    // In the valid range, which is positive here
        {
            newVal[i] = tmp;
            /* std::stol() would convert "383aaa" without throwing an exception.
               We set the correct text based on the number identified.        */
            pointers[i]->SetEditText(QString::number(tmp));
        } else {
            newVal[i] = oldVal[i];
            pointers[i]->SetEditText(QString::number(oldVal[i]));
        }
    }

    /* Only write back to _params when newVal is different from oldVal */
    bool diff = false;
    for (int i = 0; i < 4; i++) {
        if (newVal[i] != oldVal[i]) {
            diff = true;
            break;
        }
    }
    if (diff) { _params->SetRakeNumOfSeeds(newVal); }
}

void FlowSeedingSubtab::_rakeGeometryChanged()
{
    std::vector<float> range;
    _rake->GetCurrentValues(range);
    VAssert(range.size() == 6);
    _params->SetRake(range);
}

void FlowSeedingSubtab::_seedGenModeChanged(int newIdx) { _params->SetSeedGenMode(newIdx); }

void FlowSeedingSubtab::_fileReaderChanged()
{
    std::string filename = _fileReader->GetPath();
    _params->SetSeedInputFilename(filename);
}

void FlowSeedingSubtab::_fileWriterChanged()
{
    std::string filename = _fileWriter->GetPath();
    _params->SetFlowlineOutputFilename(filename);
    _params->SetNeedFlowlineOutput(true);
}

void FlowSeedingSubtab::_flowDirectionChanged(int newIdx) { _params->SetFlowDirection(newIdx); }

//
//================================
//
FlowGeometrySubtab::FlowGeometrySubtab(QWidget *parent) : QVaporSubtab(parent)
{
    _geometryWidget = new GeometryWidget(this);
    _copyRegionWidget = new CopyRegionWidget(this);
    _transformTable = new TransformTable(this);
    _geometryWidget->Reinit((DimFlags)THREED, (VariableFlags)VECTOR);

    _layout->addWidget(_geometryWidget, 0, 0);
    _layout->addWidget(_copyRegionWidget, 0, 0);
    _layout->addWidget(_transformTable, 0, 0);

    _params = NULL;
}

void FlowGeometrySubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(rParams);
    assert(_params);

    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    _copyRegionWidget->Update(paramsMgr, rParams);
    _transformTable->Update(rParams->GetTransform());
}

//
//================================
//
FlowAnnotationSubtab::FlowAnnotationSubtab(QWidget *parent) : QVaporSubtab(parent)
{
    _colorbarWidget = new ColorbarWidget(this);
    _layout->addWidget(_colorbarWidget, 0, 0);
}

void FlowAnnotationSubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
