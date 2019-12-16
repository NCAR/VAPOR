#include "FlowSubtabs.h"
#include "vapor/DataMgrUtils.h"
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

const int MIN_AXIS_SEEDS = 1;
const int MAX_AXIS_SEEDS = 1000;
const int MIN_RANDOM_SEEDS = 1;
const int MAX_RANDOM_SEEDS = 1000;

const int MAX_PATHLINE_LENGTH = 10000;

const int X = 0;
const int Y = 1;
const int Z = 2;
const int RANDOM_INDEX = 3;
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
    _variablesWidget->Reinit((VariableFlags)(VECTOR | COLOR), (DimFlags)(THREED));
    _layout->addWidget(_variablesWidget, 0, 0);
}

void FlowVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(rParams);
    assert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);
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
FlowSeedingSubtab::FlowSeedingSubtab(QWidget *parent) : QVaporSubtab(parent)
{
    _params = nullptr;

    _createIntegrationSection();
    _createSeedingSection(parent);

    // Lastly add file selector for writing seed geometry
    //
    _geometryWriterSection = new VSection("Write Flowlines to File");
    layout()->addWidget(_geometryWriterSection);
    _geometryWriterSelector = new VFileWriter();
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

    std::vector<std::string> values = {GRIDDED_STRING, RANDOM_STRING, LIST_STRING};
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
    _griddedSeedsFrame->addWidget(new VLineItem("Z axis seeds", _zSeedSliderEdit));
    connect(_zSeedSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_rakeNumOfSeedsChanged);

    // List of seeds selection
    _listOfSeedsFileReader = new VFileReader();
    connect(_listOfSeedsFileReader, &VFileReader::ValueChanged, this, &FlowSeedingSubtab::_seedListFileChanged);
    _listOfSeedsFrame = new VFrame();
    _listOfSeedsFrame->addWidget(new VLineItem("List of seeds file", _listOfSeedsFileReader));
    _seedDistributionSection->layout()->addWidget(_listOfSeedsFrame);

    // Random distribution selection
    _randomSeedsFrame = new VFrame();
    _seedDistributionSection->layout()->addWidget(_randomSeedsFrame);

    _randomSeedsSliderEdit = new VSliderEdit(MIN_RANDOM_SEEDS, MAX_RANDOM_SEEDS);
    _randomSeedsSliderEdit->SetIntType(true);
    _randomSeedsFrame->addWidget(new VLineItem("Seed count", _randomSeedsSliderEdit));
    connect(_randomSeedsSliderEdit, &VSliderEdit::ValueChangedInt, this, &FlowSeedingSubtab::_rakeNumOfSeedsChanged);

    _biasWeightSliderEdit = new VSliderEdit(-1, 1, 0);
    _randomSeedsFrame->addWidget(new VLineItem("Bias weight", _biasWeightSliderEdit));
    connect(_biasWeightSliderEdit, &VSliderEdit::ValueChanged, this, &FlowSeedingSubtab::_biasStrengthChanged);

    _biasVariableComboBox = new VComboBox(std::vector<std::string>());
    _randomSeedsFrame->addWidget(new VLineItem("Bias variable", _biasVariableComboBox));
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
    _pathlineFrame->addWidget(new VLineItem("Streamline length", _pathlineLengthSliderEdit));

    /*_pathlineStartSliderEdit= new VSliderEdit();
    _pathlineStartSliderEdit->SetIntType(true);
    connect( _pathlineStartSliderEdit, &VSliderEdit::ValueChangedInt,
        this, &FlowSeedingSubtab::_pathlineStartTimeChanged );
    _pathlineFrame->addWidget( new VLineItem("Injection start time", _pathlineStartSliderEdit));
    _pathlineStartSliderEdit->setEnabled(false);

    _pathlineEndSliderEdit = new VSliderEdit();
    _pathlineEndSliderEdit->SetIntType(true);
    connect( _pathlineEndSliderEdit, &VSliderEdit::ValueChangedInt,
        this, &FlowSeedingSubtab::_pathlineEndTimeChanged );
    _pathlineFrame->addWidget( new VLineItem("Injection end time", _pathlineEndSliderEdit));
    _pathlineEndSliderEdit->setEnabled(false);

    _pathlineInjIntervalSliderEdit = new VSliderEdit();
    _pathlineInjIntervalSliderEdit->SetIntType(true);
    connect( _pathlineInjIntervalSliderEdit,  &VSliderEdit::ValueChangedInt,
        this, &FlowSeedingSubtab::_seedInjIntervalChanged );
    _pathlineFrame->addWidget( new VLineItem("Injection interval", _pathlineInjIntervalSliderEdit));

    _pathlineLifetimeSliderEdit = new VSliderEdit();
    _pathlineLifetimeSliderEdit->SetIntType(true);
    connect( _pathlineLifetimeSliderEdit,  &VSliderEdit::ValueChangedInt,
        this, &FlowSeedingSubtab::_pathlineLifetimeChanged );
    _pathlineFrame->addWidget( new VLineItem("Seed lifetime", _pathlineLifetimeSliderEdit) );
    _pathlineLifetimeSliderEdit->setEnabled(false);*/

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
    connect(_periodicZCheckBox, &VCheckBox::ValueChanged, this, &FlowSeedingSubtab::_periodicClicked);
    _integrationSection->layout()->addWidget(new VLineItem("Z axis periodicity", _periodicZCheckBox));

    _configureFlowType(STEADY_STRING);
}

void FlowSeedingSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params)
{
    _params = dynamic_cast<VAPoR::FlowParams *>(params);
    _paramsMgr = paramsMgr;
    VAssert(_params);

    // Update integration tab
    //
    bool isSteady = _params->GetIsSteady();
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
    _periodicZCheckBox->SetValue(bools[Z]);

    // Velocity multiplier
    auto mltp = _params->GetVelocityMultiplier();
    _velocityMultiplierLineEdit->SetValue(std::to_string(mltp));

    // Update seeding tab
    //
    int mode = _params->GetSeedGenMode();
    if (mode == (int)VAPoR::FlowSeedMode::UNIFORM)
        _seedTypeCombo->SetValue(GRIDDED_STRING);
    else if (mode == (int)VAPoR::FlowSeedMode::RANDOM)
        _seedTypeCombo->SetValue(RANDOM_STRING);
    else if (mode == (int)VAPoR::FlowSeedMode::LIST)
        _seedTypeCombo->SetValue(LIST_STRING);

    // Random rake values
    std::vector<std::string> vars = dataMgr->GetDataVarNames(3);    // Do we support 2D flow?
    _biasVariableComboBox->SetOptions(vars);
    std::string var = _params->GetRakeBiasVariable();
    if (var.empty())    // The variable isn't set by the user yet. Let's set it!
    {
        auto varDefault = _biasVariableComboBox->GetValue();
        _params->SetRakeBiasVariable(varDefault);
    } else
        _biasVariableComboBox->SetValue(var);

    double bias = _params->GetRakeBiasStrength();
    _biasWeightSliderEdit->SetValue(bias);

    // Random and Gridded # seeds
    std::vector<long> seedVec = _params->GetRakeNumOfSeeds();
    _xSeedSliderEdit->SetValue(seedVec[X]);
    _ySeedSliderEdit->SetValue(seedVec[Y]);
    _zSeedSliderEdit->SetValue(seedVec[Z]);
    _randomSeedsSliderEdit->SetValue(seedVec[RANDOM_INDEX]);

    // Update rake
    std::vector<double> minExt, maxExt;
    std::vector<int>    axes;
    VAPoR::DataMgrUtils::GetExtents(dataMgr, _params->GetCurrentTimestep(), _params->GetFieldVariableNames(), _params->GetRefinementLevel(), _params->GetCompressionLevel(), minExt, maxExt, axes);
    VAssert(minExt.size() == 3 && maxExt.size() == 3);
    std::vector<float> range;
    for (int i = 0; i < 3; i++) {
        range.push_back(float(minExt[i]));
        range.push_back(float(maxExt[i]));
    }
    _rakeWidget->SetRange(range);
    auto rakeVals = _params->GetRake();
    /* In case the user hasn't set the rake, set the current value to be the rake extents,
       plus update the params.  Otherwise, apply the actual rake values.*/
    if (std::isnan(rakeVals[0])) {
        _rakeWidget->SetValue(range);
        _params->SetRake(range);
    } else {
        _rakeWidget->SetValue(rakeVals);
    }
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

    // Unsteady flow integration length
    _pathlineLengthSliderEdit->SetRange(0, numTS - 1);
    int valParams = _params->GetPastNumOfTimeSteps();
    if (valParams < 0)    // initial value, we need to set it to all time steps!
    {
        _pathlineLengthSliderEdit->SetValue(1);
        _params->SetPastNumOfTimeSteps(1);
    } else {
        _pathlineLengthSliderEdit->SetValue(valParams);
    }

    /*
    // Seed injection interval
    _pathlineInjIntervalSliderEdit->SetRange(0, numTS - 1 );
    int injIntv = _params->GetSeedInjInterval();
    if( injIntv < 0 )       // initial value, we set it to 0
    {
        _pathlineInjIntervalSliderEdit->SetValue( 0 );
        _params->SetSeedInjInterval( 0 );
    }
    else
    {
        _pathlineInjIntervalSliderEdit->SetValue( injIntv );
    }
    */
}

void FlowSeedingSubtab::_periodicClicked()
{
    std::vector<bool> bools(3, false);
    bools[0] = _periodicXCheckBox->GetValue();
    bools[1] = _periodicYCheckBox->GetValue();
    bools[2] = _periodicZCheckBox->GetValue();
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

    if (newval >= 0.001 && newval <= 1000.0)    // in the valid range
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
        if (_params != nullptr) _params->SetSeedGenMode((int)VAPoR::FlowSeedMode::UNIFORM);
    } else if (value == LIST_STRING) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->show();
        _randomSeedsFrame->hide();
        _rakeRegionSection->hide();
        if (_params != nullptr) _params->SetSeedGenMode((int)VAPoR::FlowSeedMode::LIST);
    } else if (value == RANDOM_STRING) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->show();
        _rakeRegionSection->show();
        if (_params != nullptr) _params->SetSeedGenMode((int)VAPoR::FlowSeedMode::RANDOM_BIAS);
    }
}

void FlowSeedingSubtab::_biasVariableChanged(const std::string &variable) { _params->SetRakeBiasVariable(variable); }

void FlowSeedingSubtab::_biasStrengthChanged(double strength) { _params->SetRakeBiasStrength(strength); }

void FlowSeedingSubtab::_rakeNumOfSeedsChanged()
{
    std::vector<long> seedsVector(4, (long)1.0);
    seedsVector[X] = _xSeedSliderEdit->GetValue();
    seedsVector[Y] = _ySeedSliderEdit->GetValue();
    seedsVector[Z] = _zSeedSliderEdit->GetValue();
    seedsVector[RANDOM_INDEX] = _randomSeedsSliderEdit->GetValue();
    _params->SetRakeNumOfSeeds(seedsVector);
}

void FlowSeedingSubtab::_seedListFileChanged(const std::string &value) { _params->SetSeedInputFilename(value); }

void FlowSeedingSubtab::_rakeGeometryChanged(const std::vector<float> &range)
{
    VAssert(range.size() == 6);
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
