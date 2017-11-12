#include "ContourSubtabs.h"

ContourAppearanceSubtab::ContourAppearanceSubtab(QWidget *parent) {
    setupUi(this);

    _TFWidget->Reinit((TFWidget::Flags)(TFWidget::CONSTCOLOR));
    _TFWidget->mappingFrame->setIsolineSliders(true);
    _TFWidget->mappingFrame->setOpacityMapping(false);

    _lineWidthCombo = new Combo(lineWidthEdit, lineWidthSlider);
    _countCombo = new Combo(contourCountEdit, contourCountSlider, true);
    _cMinCombo = new Combo(contourMinEdit, contourMinSlider);
    _spacingCombo = new Combo(contourSpacingEdit, contourSpacingSlider);

    connect(_lineWidthCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetLineThickness(double)));
    connect(_countCombo, SIGNAL(valueChanged(int)), this,
            SLOT(SetContourCount(int)));
    connect(_cMinCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetContourMinimum(double)));
    connect(_spacingCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetContourSpacing(double)));
    connect(lockToTFCheckbox, SIGNAL(toggled(bool)), this,
            SLOT(LockToTFChecked(bool)));
    connect(_TFWidget, SIGNAL(emitChange()), this,
            SLOT(EndTFChange()));
}

double ContourAppearanceSubtab::GetContourMinOrMax(string minOrMax) {

    bool locked = _cParams->GetLockToTF();
    lockToTFCheckbox->setChecked(locked);

    // Apply params to contour minimum.  Restrict the minimum according
    // to the transfer function if we're locking to it (via 'locked'
    // parameter).  If not, restrict according to variable min/max.
    //
    //double min = _cParams->GetContourMin();
    string varname = _cParams->GetVariableName();
    if (varname.empty())
        return (0.0);

    double spacing, maxSpacing;
    if (locked) {
        // Update contour minimum combo
        //
        VAPoR::MapperFunction *tf = _cParams->GetMapperFunc(varname);
        double lower = tf->getMinMapValue();
        double upper = tf->getMaxMapValue();

        if (minOrMax == "max")
            return upper;
        else
            return lower;
    } else {
        // Apply settings to contour minimum and spacing, bounded only
        // by the min/max values of the variable.
        //
        int lod = _cParams->GetCompressionLevel();
        int level = _cParams->GetRefinementLevel();
        int ts = _cParams->GetCurrentTimestep();
        VAPoR::Grid *var = _dataMgr->GetVariable(ts, varname, level, lod);
        float range[2];
        var->GetRange(range);
        if (minOrMax == "max")
            return range[1];
        else
            return range[0];
    }
}

void ContourAppearanceSubtab::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams) {
    _cParams = (VAPoR::ContourParams *)rParams;
    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;

    // Apply params to lineThickness.  Get range
    // for thickness from gl system call
    //
    GLfloat lineWidthRange[2] = {0.f, 0.f};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    _lineWidthCombo->Update(lineWidthRange[0], lineWidthRange[1],
                            _cParams->GetLineThickness());

    // Lock contours to TF bounds state
    //
    bool lock = _cParams->GetTFLock();
    lockToTFCheckbox->setChecked(lock);
    // If we're locked, disable sliders and edits.
    // Enable if unlocked.
    //
    contourSpacingSlider->setEnabled(!lock);
    contourSpacingEdit->setEnabled(!lock);
    contourMinSlider->setEnabled(!lock);
    contourMinEdit->setEnabled(!lock);

    // Apply params to contour count
    //
    int numContours = _cParams->GetNumContours();
    _countCombo->Update(1, 50, numContours);
    if (_cParams->GetNumContours() < 2) {
        contourSpacingSlider->setEnabled(false);
        contourSpacingEdit->setEnabled(false);
    }

    // Update contour spacing and minimum settings, which may
    // or may not be locked within the transfer function bounds.
    //
    double minComboMin = GetContourMinOrMax("min");
    double minComboMax = GetContourMinOrMax("max");
    double minVal = _cParams->GetContourMin();
    _cMinCombo->Update(minComboMin, minComboMax, minVal);

    double spacing = _cParams->GetContourSpacing();
    double maxSpacing = (minComboMax - minVal) / (double)(numContours - 1);

    // Using this equation omits the space-slider jump after being moved.  Why?
    //double maxSpacing = (minComboMax - minComboMin) / (double)(numContours-1);
    _spacingCombo->Update(0, maxSpacing, spacing);

    _TFWidget->Update(dataMgr, paramsMgr, _cParams);
    _ColorbarWidget->Update(dataMgr, paramsMgr, _cParams);
}

void ContourAppearanceSubtab::disableSliders() {
    _cMinCombo->blockSignals(true);
    _countCombo->blockSignals(true);
    _spacingCombo->blockSignals(true);
}

void ContourAppearanceSubtab::enableSliders() {
    _countCombo->blockSignals(false);
    _cMinCombo->blockSignals(false);
    _spacingCombo->blockSignals(false);
}

void ContourAppearanceSubtab::Initialize(VAPoR::ContourParams *cParams) {
    _paramsMgr->BeginSaveStateGroup("Initialize ContourAppearanceSubtab");

    _cParams = cParams;
    string varname = _cParams->GetVariableName();
    if (varname.empty())
        return;

    VAPoR::MapperFunction *tf = _cParams->GetMapperFunc(varname);
    double lower = tf->getMinMapValue();
    double upper = tf->getMaxMapValue();
    int count = _cParams->GetNumContours();
    double spacing = (upper - lower) / (double)(count - 1);

    disableSliders();
    _cMinCombo->Update(lower, upper, lower);
    _countCombo->Update(1, 50, count);
    _spacingCombo->Update(0, upper - lower, spacing);
    enableSliders();

    SetContourValues();

    _paramsMgr->EndSaveStateGroup();
}

// Do not BeginSaveStateGroup here!  This funciton
// is always encapsulated by a Begin/EndSaveStateGroup.
//
void ContourAppearanceSubtab::SetContourValues() {
    vector<double> cVals;
    int numContours = _countCombo->GetValue();
    double spacing = _spacingCombo->GetValue();
    double min = _cMinCombo->GetValue();

    for (size_t i = 0; i < numContours; i++) {
        cVals.push_back(min + spacing * i);
    }
    string varName = _cParams->GetVariableName();
    _cParams->SetContourValues(varName, cVals);
}

void ContourAppearanceSubtab::EndTFChange() {
    double min = GetContourMinOrMax("min");
    double max = GetContourMinOrMax("max");
    double minVal = _cMinCombo->GetValue();
    bool locked = _cParams->GetTFLock();
    if (locked)
        minVal = min;
    if (minVal > max)
        minVal = max;

    _cMinCombo->Update(min, max, minVal);

    int numContours = _countCombo->GetValue();
    double spacing = _spacingCombo->GetValue();
    double maxSpacing = (max - minVal) / (double)(numContours - 1);
    double span = spacing * (numContours - 1) + minVal;

    if ((span > max) || (_cParams->GetLockToTF())) {
        spacing = maxSpacing;
    }

    _spacingCombo->Update(0, maxSpacing, spacing);
    SetContourValues();
}

void ContourAppearanceSubtab::SetContourCount(int count) {
    disableSliders();

    //_countCombo->Update(1, 50, count);

    int previousCount = _cParams->GetNumContours();
    bool locked = _cParams->GetLockToTF();
    double lower, upper, spacing;
    string varname = _cParams->GetVariableName();
    VAPoR::MapperFunction *tf = _cParams->GetMapperFunc(varname);
    if (locked) {
        lower = tf->getMinMapValue();
        upper = tf->getMaxMapValue();
        spacing = (upper - lower) / (count - 1);

        _cMinCombo->Update(lower, upper, lower);
        _spacingCombo->Update(0, upper - lower, spacing);
    } else if (previousCount == 1) {
        lower = _cParams->GetContourMin();
        upper = tf->getMaxMapValue();
        spacing = (upper - lower) / (count - 1);

        _cMinCombo->Update(lower, upper, lower);
        _spacingCombo->Update(0, upper - lower, spacing);
    }

    _countCombo->Update(1, 50, count);

    SetContourValues();

    enableSliders();
}

void ContourAppearanceSubtab::SetContourMinimum(double value) {
    disableSliders();

    double min = GetContourMinOrMax("min");
    double max = GetContourMinOrMax("max");
    _cMinCombo->Update(min, max, value);

    double spacing = _spacingCombo->GetValue();
    double count = _countCombo->GetValue();
    bool locked = _cParams->GetLockToTF();

    if (!locked) {
        double maxContour = value + spacing * (count - 1);
        if (maxContour > max) {
            spacing = (max - value) / (count - 1);
            _spacingCombo->Update(0, spacing, spacing);
        }
    }

    SetContourValues();
    enableSliders();
}

void ContourAppearanceSubtab::SetContourSpacing(double spacing) {
    int count = _cParams->GetNumContours();
    if (count < 2)
        return;

    disableSliders();

    double min = GetContourMinOrMax("min");
    double max = GetContourMinOrMax("max");
    double maxSpacing = (max - min) / (count - 1);
    _spacingCombo->Update(0, maxSpacing, spacing);

    SetContourValues();
    enableSliders();
}

void ContourAppearanceSubtab::LockToTFChecked(bool checked) {
    _cParams->SetLockToTF(checked);
    if (checked) {
        EndTFChange();
    }
}
