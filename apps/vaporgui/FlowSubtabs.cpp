#include "FlowSubtabs.h"
#include "vapor/DataMgrUtils.h"
#include "vapor/Box.h"
#include "ErrorReporter.h"

#include "VFrame.h"
#include "VIntSpinBox.h"
#include "VComboBox.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include "VSliderEdit.h"
#include "VLineEdit.h"
#include "VFileSelector.h"
#include "VGeometry2.h"
#include "VPushButton.h"

#include <QScrollArea>

#define verbose 1

namespace {
const std::string UNSTEADY_STRING = "Pathlines";
const std::string STEADY_STRING = "Streamlines";
const std::string GRIDDED_STRING = "Gridded";
const std::string LIST_STRING = "List of seeds";
const std::string RANDOM_STRING = "Random";
const std::string RANDOM_BIAS_STR = "Random w/ Bias";

const int MIN_AXIS_SEEDS = 1;
const int MAX_AXIS_SEEDS = 50;
const int MIN_RANDOM_SEEDS = 1;
const int MAX_RANDOM_SEEDS = 500;

const int MAX_PATHLINE_LENGTH = 5000;

const int X = 0;
const int Y = 1;
const int Z = 2;

const int Z_RAKE_MIN = 4;
const int Z_RAKE_MAX = 5;
}    // namespace

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
    _variablesWidget->Reinit((VariableFlags)(VECTOR | COLOR), (DimFlags)(TWOD | THREED));
    _layout->addWidget(_variablesWidget, 0, 0);

    connect(_variablesWidget, &VariablesWidget::_dimensionalityChanged, this, &FlowVariablesSubtab::_dimensionalityChanged);
}

void FlowVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(rParams);
    assert(_params);

    _paramsMgr = paramsMgr;

    _variablesWidget->Update(dataMgr, paramsMgr, rParams);

    GUIStateParams *gp = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    int             nDims = gp->GetFlowDimensionality();
    bool            no3DVars = dataMgr->GetDataVarNames(3).size() ? false : true;

    // If dimensionality has changed, tell _variablesWidget to adjust itself for 2DFlow,
    // and update GUIStateParams to the new dimension.
    //
    // Note: We don't want the variables-widget making this call because this is
    // not always our desired behavior.
    //
    // The barb renderer can operate on three two-dimensional variables.
    // However the flow renderer operates on two two-dimensional variables.
    // Since the VariablesWidget has no information on what renderer it is working
    // with, we need to configure these two different cases externally, which is what's
    // being done here.
    if (no3DVars && nDims != _variablesWidget->GetActiveDimension()) {
        _variablesWidget->Configure2DFieldVars();
        gp->SetFlowDimensionality(_variablesWidget->GetActiveDimension());
    } else if (nDims != _variablesWidget->GetActiveDimension()) {
        if (nDims == 2)
            _variablesWidget->Configure2DFieldVars();
        else
            _variablesWidget->Configure3DFieldVars();
        gp->SetFlowDimensionality(_variablesWidget->GetActiveDimension());
    }
}

void FlowVariablesSubtab::_dimensionalityChanged(int nDims) const
{
    GUIStateParams *gp = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    gp->SetFlowDimensionality(nDims);
    if (nDims == 2)
        _variablesWidget->Configure2DFieldVars();
    else
        _variablesWidget->Configure3DFieldVars();
}

//
//================================
//
FlowAppearanceSubtab::FlowAppearanceSubtab(QWidget *parent) : QVaporSubtab(parent)
{
    _TFEditor = new TFEditor(true);

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
FlowSeedingSubtab::FlowSeedingSubtab(QWidget *parent)
: QVaporSubtab(parent), _numDims(-1),    // Initial value.  Will be configured during Update().
  _oldZRakeNumSeeds(5), _oldZRakeMin(FLT_MIN), _oldZRakeMax(FLT_MAX), _oldZPeriodicity(false)
{
    _params = nullptr;

    _createIntegrationSection();
    _createSeedingSection(parent);

    // Lastly add file selector for writing seed geometry
    //
    _geometryWriterSection = new VSection("Write Flowlines to File");
    layout()->addWidget(_geometryWriterSection);
    string defaultPath = QDir::homePath().toStdString() + "/vaporFlow.txt";
    _geometryWriterSelector = new VFileWriter("Select", defaultPath);
    _geometryWriterSection->layout()->addWidget(new VLineItem("Target file", _geometryWriterSelector));
    _geometryWriterExecutor = new VPushButton("Write to file");
    _geometryWriterSection->layout()->addWidget(new VLineItem("", _geometryWriterExecutor));
    connect(_geometryWriterExecutor, &VPushButton::ButtonClicked, this, &FlowSeedingSubtab::_geometryWriterClicked);
    layout()->addWidget(_geometryWriterSection);
}

void FlowSeedingSubtab::_createSeedingSection(QWidget *parent)
{
    _seedDistributionSection = new VSection("Seed Distribution Settings");
    layout()->addWidget(_seedDistributionSection);

    std::vector<std::string> values = {GRIDDED_STRING, RANDOM_STRING, RANDOM_BIAS_STR, LIST_STRING};
    _seedTypeCombo = new VComboBox(values);
    _seedDistributionSection->layout()->addWidget(new VLineItem("Seed distribution type", _seedTypeCombo));
    connect(_seedTypeCombo, &VComboBox::ValueChanged, this, &FlowSeedingSubtab::_configureSeedType);

    // Gridded seed selection
    _griddedSeedsFrame = new VFrame();
    _seedDistributionSection->layout()->addWidget(_griddedSeedsFrame);

    _xSeedSliderEdit = new VSliderEdit();
    _xSeedSliderEdit->SetIntType(true);
    _xSeedSliderEdit->SetRange(MIN_AXIS_SEEDS, MAX_AXIS_SEEDS);
    _griddedSeedsFrame->addWidget(new VLineItem("X axis seeds", _xSeedSliderEdit));
    connect(_xSeedSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_rakeNumOfSeedsChanged);
    _ySeedSliderEdit = new VSliderEdit();
    _ySeedSliderEdit->SetIntType(true);
    _ySeedSliderEdit->SetRange(MIN_AXIS_SEEDS, MAX_AXIS_SEEDS);
    _griddedSeedsFrame->addWidget(new VLineItem("Y axis seeds", _ySeedSliderEdit));
    connect(_ySeedSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_rakeNumOfSeedsChanged);
    _zSeedSliderEdit = new VSliderEdit();
    _zSeedSliderEdit->SetIntType(true);
    _zSeedSliderEdit->SetRange(MIN_AXIS_SEEDS, MAX_AXIS_SEEDS);
    _zSeedLine = new VLineItem("Z axis seeds", _zSeedSliderEdit);
    _griddedSeedsFrame->addWidget(_zSeedLine);
    connect(_zSeedSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_rakeNumOfSeedsChanged);

    // List of seeds selection
    _listOfSeedsFileReader = new VFileReader();
    connect(_listOfSeedsFileReader, &VFileReader::ValueChanged, this, &FlowSeedingSubtab::_seedListFileChanged);
    _listOfSeedsFrame = new VFrame();
    _listOfSeedsFrame->addWidget(new VLineItem("List of seeds file", _listOfSeedsFileReader));
    _seedDistributionSection->layout()->addWidget(_listOfSeedsFrame);
    _listOfSeedsFrame->setToolTip("A text file that contains all the seeds.\n\
Each line of this file specifies a seed location and time as comma separated X, Y, Z, T values,\n\
where T is optional.\n\
Any line that is empty or starts with a # is not considered as input.");

    // Random distribution selection
    _randomSeedsFrame = new VFrame();
    _seedDistributionSection->layout()->addWidget(_randomSeedsFrame);

    _randomSeedsSliderEdit = new VSliderEdit(MIN_RANDOM_SEEDS, MAX_RANDOM_SEEDS);
    _randomSeedsSliderEdit->SetIntType(true);
    _randomSeedsFrame->addWidget(new VLineItem("Seed count", _randomSeedsSliderEdit));    // 1st widget
    connect(_randomSeedsSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_randomNumOfSeedsChanged);

    _biasWeightSliderEdit = new VSliderEdit(-10.0, 10.0, 0.0);
    _randomSeedsFrame->addWidget(new VLineItem("Bias weight", _biasWeightSliderEdit));    // 2nd widget
    connect(_biasWeightSliderEdit, &VSliderEdit::ValueChanged, this, &FlowSeedingSubtab::_biasStrengthChanged);

    _biasVariableComboBox = new VComboBox(std::vector<std::string>());
    _randomSeedsFrame->addWidget(new VLineItem("Bias variable", _biasVariableComboBox));    // 3rd widget
    connect(_biasVariableComboBox, &VComboBox::ValueChanged, this, &FlowSeedingSubtab::_biasVariableChanged);

    // Rake selector
    _rakeRegionSection = new VSection("Rake Region");
    layout()->addWidget(_rakeRegionSection);
    _rakeWidget = new VGeometry2();
    _rakeRegionSection->layout()->addWidget(_rakeWidget);
    connect(_rakeWidget, &VGeometry2::ValueChanged, this, &FlowSeedingSubtab::_rakeGeometryChanged);

    VAssert(parent);
    connect(parent, SIGNAL(currentChanged(int)), this, SLOT(_selectedTabChanged(int)));

    _configureSeedType(GRIDDED_STRING);
}

void FlowSeedingSubtab::_createIntegrationSection()
{
    _integrationSection = new VSection("Flow Integration Settings");
    layout()->addWidget(_integrationSection);

    // Steady flow options
    //
    std::vector<std::string> values = {STEADY_STRING, UNSTEADY_STRING};
    _flowTypeCombo = new VComboBox(values);
    connect(_flowTypeCombo, &VComboBox::ValueChanged, this, &FlowSeedingSubtab::_configureFlowType);
    _integrationSection->layout()->addWidget(new VLineItem("Flow type", _flowTypeCombo));

    _streamlineFrame = new VFrame();
    _integrationSection->layout()->addWidget(_streamlineFrame);

    values = {"Forward", "Backward", "Bi-Directional"};
    _streamlineDirectionCombo = new VComboBox(values);
    connect(_streamlineDirectionCombo, &VComboBox::IndexChanged, this, &FlowSeedingSubtab::_streamlineDirectionChanged);
    _streamlineFrame->addWidget(new VLineItem("Flow direction", _streamlineDirectionCombo));

    _streamlineSamplesSliderEdit = new VSliderEdit();
    _streamlineSamplesSliderEdit->SetIntType(true);
    connect(_streamlineSamplesSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_streamlineSamplesChanged);
    _streamlineFrame->addWidget(new VLineItem("Integration steps", _streamlineSamplesSliderEdit));

    // Unsteady flow options
    //
    _pathlineFrame = new VFrame();
    _integrationSection->layout()->addWidget(_pathlineFrame);

    _pathlineLengthSliderEdit = new VSliderEdit();
    _pathlineLengthSliderEdit->SetIntType(true);
    connect(_pathlineLengthSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_pathlineLengthChanged);
    VLineItem *lengthLE = new VLineItem("Pathline length", _pathlineLengthSliderEdit);
    QString    lengthTip = "Controls the length of the pathlines.  The units for this parameter\n"
                        "are timesteps.  A pathline with a length of 3 will display a line\n"
                        "that traverses the current timestep, as well as where the pathline\n"
                        "was in the previous two timesteps.";
    lengthLE->setToolTip(lengthTip);
    _pathlineFrame->addWidget(lengthLE);

    _pathlineInjInterval = new VSliderEdit();
    _pathlineInjInterval->SetIntType(true);
    connect(_pathlineInjInterval, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_seedInjIntervalChanged);
    VLineItem *injectionLE = new VLineItem("Injection interval", _pathlineInjInterval);
    QString    injectionTip = "Controls the period in which seeds are injected into the scene.\n"
                           "For example, a value of 3 will inject seeds every 3 time steps.";
    injectionLE->setToolTip(injectionTip);
    _pathlineFrame->addWidget(injectionLE);

    // Universal options: Velocity multiplier and periodicity checkboxes
    //
    _velocityMultiplierLineEdit = new VLineEdit();
    _velocityMultiplierLineEdit->SetIsDouble(true);
    _velocityMultiplierLineEdit->UseDoubleMenu();
    connect(_velocityMultiplierLineEdit, &VLineEdit::ValueChanged, this, &FlowSeedingSubtab::_velocityMultiplierChanged);
    _integrationSection->layout()->addWidget(new VLineItem("Vector field  multiplier", _velocityMultiplierLineEdit));

    // Periodicity Checkboxes
    //
    _periodicXCheckBox = new VCheckBox();
    connect(_periodicXCheckBox, &VCheckBox::ValueChanged, this, &FlowSeedingSubtab::_periodicClicked);
    _integrationSection->layout()->addWidget(new VLineItem("X axis periodicity", _periodicXCheckBox));
    _periodicYCheckBox = new VCheckBox();
    connect(_periodicYCheckBox, &VCheckBox::ValueChanged, this, &FlowSeedingSubtab::_periodicClicked);
    _integrationSection->layout()->addWidget(new VLineItem("Y axis periodicity", _periodicYCheckBox));
    _periodicZCheckBox = new VCheckBox();
    _zPeriodicityLine = new VLineItem("Z axis periodicity", _periodicZCheckBox);
    connect(_periodicZCheckBox, &VCheckBox::ValueChanged, this, &FlowSeedingSubtab::_periodicClicked);
    _integrationSection->layout()->addWidget(_zPeriodicityLine);

    _configureFlowType(STEADY_STRING);
}

void FlowSeedingSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(params);
    _paramsMgr = paramsMgr;
    VAssert(_params);

    _updateRake(dataMgr);

    GUIStateParams *gp = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    int             newDims = gp->GetFlowDimensionality();
    if (_numDims != newDims) {
        _numDims = newDims;
        _resizeFlowParamsVectors();
    }
    // Update integration tab
    //
    bool        isSteady = _params->GetIsSteady();
    std::string flowType = isSteady ? STEADY_STRING : UNSTEADY_STRING;
    _configureFlowType(flowType);

    if (isSteady)
        _flowTypeCombo->SetValue(STEADY_STRING);
    else
        _flowTypeCombo->SetValue(UNSTEADY_STRING);

    _updatePathlineWidgets(dataMgr);
    _updateStreamlineWidgets(dataMgr);

    // Periodicity checkboxes
    auto bools = _params->GetPeriodic();
    _periodicXCheckBox->SetValue(bools[X]);
    _periodicYCheckBox->SetValue(bools[Y]);
    if (_numDims == 3) {
        VAssert(bools.size() == 3);
        _periodicZCheckBox->SetValue(bools[Z]);
    }

    // Find out we're operating on 2D or 3D variables
    if (_numDims == 2) {
        _zPeriodicityLine->hide();
        _zSeedLine->hide();
    } else {
        _zPeriodicityLine->show();
        _zSeedLine->show();
    }

    // Velocity multiplier
    auto mltp = _params->GetVelocityMultiplier();
    _velocityMultiplierLineEdit->SetValue(std::to_string(mltp));

    // Update seeding tab
    //
    int mode = _params->GetSeedGenMode();
    if (mode == static_cast<int>(VAPoR::FlowSeedMode::UNIFORM)) {
        _seedTypeCombo->SetValue(GRIDDED_STRING);
        _configureSeedType(GRIDDED_STRING);
    } else if (mode == static_cast<int>(VAPoR::FlowSeedMode::RANDOM)) {
        _seedTypeCombo->SetValue(RANDOM_STRING);
        _configureSeedType(RANDOM_STRING);
    } else if (mode == static_cast<int>(VAPoR::FlowSeedMode::RANDOM_BIAS)) {
        _seedTypeCombo->SetValue(RANDOM_BIAS_STR);
        _configureSeedType(RANDOM_BIAS_STR);
    } else if (mode == static_cast<int>(VAPoR::FlowSeedMode::LIST)) {
        _seedTypeCombo->SetValue(LIST_STRING);
        _configureSeedType(LIST_STRING);
    }

    // Random rake values
    std::vector<std::string> vars = dataMgr->GetDataVarNames(_numDims);
    _biasVariableComboBox->SetOptions(vars);
    std::string var = _params->GetRakeBiasVariable();
    if (var.empty())    // The variable isn't set by the user yet. Let's set it!
    {
        auto varDefault = _biasVariableComboBox->GetValue();
        _params->SetRakeBiasVariable(varDefault);
    } else
        _biasVariableComboBox->SetValue(var);

    auto bias = _params->GetRakeBiasStrength();
    _biasWeightSliderEdit->SetValue(bias);

    // Random and Gridded # seeds
    // std::vector<long> seedVec = _params->GetGridNumOfSeeds();
    std::vector<long> seedVec = _params->GetGridNumOfSeeds();
    _xSeedSliderEdit->SetValue(seedVec[X]);
    _ySeedSliderEdit->SetValue(seedVec[Y]);
    if (_numDims == 3) {
        VAssert(seedVec.size() == 3);
        _zSeedSliderEdit->SetValue(seedVec[Z]);
    }

    _randomSeedsSliderEdit->SetValue(_params->GetRandomNumOfSeeds());
    //    _blockUnblockSignals( false );
}

void FlowSeedingSubtab::_blockUnblockSignals(bool block)
{
    QList<QWidget *>                 widgetList = this->findChildren<QWidget *>();
    QList<QWidget *>::const_iterator widgetIter(widgetList.begin());
    QList<QWidget *>::const_iterator lastWidget(widgetList.end());

    while (widgetIter != lastWidget) {
        (*widgetIter)->blockSignals(block);
        ++widgetIter;
    }
}

void FlowSeedingSubtab::_updateRake(VAPoR::DataMgr *dataMgr)
{
    std::vector<double> minExt, maxExt;
    std::vector<int>    axes;
    VAPoR::DataMgrUtils::GetExtents(dataMgr, _params->GetCurrentTimestep(), _params->GetFieldVariableNames(), _params->GetRefinementLevel(), _params->GetCompressionLevel(), minExt, maxExt, axes);
    // If there are no valid extents to set the rake with, just return
    //
    int minSize = minExt.size();
    int maxSize = maxExt.size();
    if (minSize != 3 && minSize != 2)
        return;
    else if (maxSize != 3 && maxSize != 2)
        return;

    std::vector<float> range;
    for (int i = 0; i < axes.size(); i++) {
        range.push_back(float(minExt[i]));
        range.push_back(float(maxExt[i]));
    }

    // Initialize _oldZRakeMin/Max
    //
    if (axes.size() == 3) {
        if (_oldZRakeMin == FLT_MIN) _oldZRakeMin = minExt[Z];
        if (_oldZRakeMax == FLT_MAX) _oldZRakeMax = maxExt[Z];
    }

    _rakeWidget->SetRange(range);

    auto rakeVals = _params->GetRake();
    _rakeWidget->SetValue(rakeVals);
}

void FlowSeedingSubtab::_resizeFlowParamsVectors()
{
    auto rakeSeeds = _params->GetGridNumOfSeeds();
    auto rakeRegion = _params->GetRake();
    auto periodicity = _params->GetPeriodic();

    // Going from 3d vectors to 2d vectors.
    // Save the values we remove for restoration later on.
    if (_numDims == 2) {
        _oldZRakeNumSeeds = rakeSeeds[Z];
        rakeSeeds.resize(2);

        _oldZRakeMin = rakeRegion[Z_RAKE_MIN];
        _oldZRakeMax = rakeRegion[Z_RAKE_MAX];
        rakeRegion.resize(4);

        _oldZPeriodicity = periodicity[Z];
        periodicity.resize(2);
    }
    // Going from 2D vectors to 3D vectors.
    // Restore previously saved values.
    else {
        periodicity.resize(3);
        periodicity[Z] = _oldZPeriodicity;

        for (int i = 0; i < 6; i++) rakeRegion.resize(6);
        rakeRegion[Z_RAKE_MIN] = _oldZRakeMin;
        rakeRegion[Z_RAKE_MAX] = _oldZRakeMax;

        rakeSeeds.resize(3);
        rakeSeeds[Z] = _oldZRakeNumSeeds;
    }

    _paramsMgr->BeginSaveStateGroup("Resizing flow params vectors");
    _params->SetGridNumOfSeeds(rakeSeeds);
    _params->SetRake(rakeRegion);
    _params->SetPeriodic(periodicity);
    _paramsMgr->EndSaveStateGroup();
}

void FlowSeedingSubtab::_updateStreamlineWidgets(VAPoR::DataMgr *dataMgr)
{
    // Steady flow direction combo
    int dir = _params->GetFlowDirection();
    if (dir >= 0 && dir < _streamlineDirectionCombo->GetCount())
        _streamlineDirectionCombo->SetIndex(dir);
    else {
        _streamlineDirectionCombo->SetIndex(0);
        _params->SetFlowDirection(0);    // use 0 as the default option
    }

    // Steady flow integration length (flowNumOfSteps)
    int steadyNumOfSteps = _params->GetSteadyNumOfSteps();
    _streamlineSamplesSliderEdit->SetValue(steadyNumOfSteps);
    _streamlineSamplesSliderEdit->SetRange(0, MAX_PATHLINE_LENGTH);
}

void FlowSeedingSubtab::_updatePathlineWidgets(VAPoR::DataMgr *dataMgr)
{
    int numTS = dataMgr->GetNumTimeSteps();

    // Unsteady flow display past number of time steps
    _pathlineLengthSliderEdit->SetRange(0, numTS - 1);
    int valParams = _params->GetPastNumOfTimeSteps();
    if (valParams < 0)    // initial value, we need to set it to all time steps!
    {
        _pathlineLengthSliderEdit->SetValue(1);
        _params->SetPastNumOfTimeSteps(1);
    } else {
        _pathlineLengthSliderEdit->SetValue(valParams);
    }

    // Unsteady seed injection interval
    _pathlineInjInterval->SetRange(0, numTS - 1);
    int injIntv = _params->GetSeedInjInterval();
    if (injIntv < 0)    // initial value, we set it to 0
    {
        _pathlineInjInterval->SetValue(0);
        _params->SetSeedInjInterval(0);
    } else {
        _pathlineInjInterval->SetValue(injIntv);
    }
}

void FlowSeedingSubtab::_periodicClicked()
{
    std::vector<bool> bools(2, false);
    bools[0] = _periodicXCheckBox->GetValue();
    bools[1] = _periodicYCheckBox->GetValue();
    if (_numDims == 3) bools.push_back(_periodicZCheckBox->GetValue());

    _params->SetPeriodic(bools);
}

void FlowSeedingSubtab::_velocityMultiplierChanged(const std::string &value)
{
    double oldval = _params->GetVelocityMultiplier();
    double newval;
    try {
        newval = std::stod(value);
    } catch (const std::invalid_argument &e) {
        MSG_ERR("Bad input: " + value);
        _velocityMultiplierLineEdit->SetValue(std::to_string(oldval));
        return;
    }

    if (newval > 0.0)    // in the valid range
    {
        // std::stod() would convert "3.83aaa" without throwing an exception.
        // We set the correct text based on the number identified.
        _velocityMultiplierLineEdit->SetValue(std::to_string(newval));
        // Only write back to _params if newval is different from oldval
        if (newval != oldval) _params->SetVelocityMultiplier(newval);
    } else
        _velocityMultiplierLineEdit->SetValue(std::to_string(oldval));
}

void FlowSeedingSubtab::_pathlineStartTimeChanged(int newVal) {}

void FlowSeedingSubtab::_pathlineEndTimeChanged(int newVal) {}

void FlowSeedingSubtab::_pathlineLifetimeChanged(int newVal) {}

void FlowSeedingSubtab::_pathlineLengthChanged(int newVal) { _params->SetPastNumOfTimeSteps(newVal); }

void FlowSeedingSubtab::_streamlineSamplesChanged(int newval) { _params->SetSteadyNumOfSteps(newval); }

void FlowSeedingSubtab::_seedInjIntervalChanged(int interval) { _params->SetSeedInjInterval(interval); }

void FlowSeedingSubtab::_configureFlowType(const std::string &value)
{
    bool isSteady = true;
    if (value == UNSTEADY_STRING) {
        isSteady = false;
        _pathlineFrame->show();
        _streamlineFrame->hide();
    } else {
        _pathlineFrame->hide();
        _streamlineFrame->show();
    }

    if (_params != nullptr) { _params->SetIsSteady((long)isSteady); }
}

void FlowSeedingSubtab::_configureSeedType(const std::string &value)
{
    if (value == GRIDDED_STRING) {
        _griddedSeedsFrame->show();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->hide();
        _rakeRegionSection->show();
        if (_params != nullptr) _params->SetSeedGenMode(static_cast<int>(VAPoR::FlowSeedMode::UNIFORM));
    } else if (value == LIST_STRING) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->show();
        _randomSeedsFrame->hide();
        _rakeRegionSection->hide();
        if (_params != nullptr) _params->SetSeedGenMode(static_cast<int>(VAPoR::FlowSeedMode::LIST));
    } else if (value == RANDOM_STRING) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->hide();
        _rakeRegionSection->show();
        _randomSeedsFrame->show();
        _randomSeedsFrame->hideChildAtIdx(1);    // bias strength
        _randomSeedsFrame->hideChildAtIdx(2);    // bias variable
        if (_params != nullptr) _params->SetSeedGenMode(static_cast<int>(VAPoR::FlowSeedMode::RANDOM));
    } else if (value == RANDOM_BIAS_STR) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->hide();
        _rakeRegionSection->show();
        _randomSeedsFrame->show();
        _randomSeedsFrame->showChildAtIdx(1);    // bias strength
        _randomSeedsFrame->showChildAtIdx(2);    // bias variable
        if (_params != nullptr) _params->SetSeedGenMode(static_cast<int>(VAPoR::FlowSeedMode::RANDOM_BIAS));
    }
}

void FlowSeedingSubtab::_biasVariableChanged(const std::string &variable) { _params->SetRakeBiasVariable(variable); }

void FlowSeedingSubtab::_biasStrengthChanged(double strength) { _params->SetRakeBiasStrength(strength); }

void FlowSeedingSubtab::_rakeNumOfSeedsChanged()
{
    std::vector<long> seedsVector(2, 1);
    seedsVector[X] = _xSeedSliderEdit->GetValue();
    seedsVector[Y] = _ySeedSliderEdit->GetValue();
    if (_numDims == 3) seedsVector.push_back(_zSeedSliderEdit->GetValue());

    _paramsMgr->BeginSaveStateGroup("Number of flow seeds changed");
    _params->SetGridNumOfSeeds(seedsVector);
    _paramsMgr->EndSaveStateGroup();
}

void FlowSeedingSubtab::_randomNumOfSeedsChanged() { _params->SetRandomNumOfSeeds(_randomSeedsSliderEdit->GetValue()); }

void FlowSeedingSubtab::_seedListFileChanged(const std::string &value) { _params->SetSeedInputFilename(value); }

void FlowSeedingSubtab::_rakeGeometryChanged(const std::vector<float> &range)
{
    VAssert(range.size() == 6 || range.size() == 4);
    _params->SetRake(range);
}

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

void FlowSeedingSubtab::_streamlineDirectionChanged(int index) { _params->SetFlowDirection(index); }

void FlowSeedingSubtab::_geometryWriterClicked()
{
    bool enabled = _params->IsEnabled();
    if (!enabled) {
        MSG_ERR("The Flow renderer must be enabled to compute trajectories "
                "before writing the anything to a file.");
        return;
    }

    std::string file = _geometryWriterSelector->GetValue();

    _paramsMgr->BeginSaveStateGroup("Write flowline geometry file");
    _params->SetFlowlineOutputFilename(file);
    _params->SetNeedFlowlineOutput(true);
    _paramsMgr->EndSaveStateGroup();
}

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
