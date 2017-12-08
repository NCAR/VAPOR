#include "ContourSubtabs.h"

ContourAppearanceSubtab::ContourAppearanceSubtab(QWidget *parent) {
    setupUi(this);

    _TFWidget->Reinit((TFWidget::Flags)(TFWidget::CONSTANT));
    _TFWidget->mappingFrame->setIsolineSliders(true);
    _TFWidget->mappingFrame->setOpacityMapping(false);

    _lineWidthCombo = new Combo(lineWidthEdit, lineWidthSlider);
    _countCombo = new Combo(contourCountEdit, contourCountSlider, true);
    _cMinCombo = new Combo(contourMinEdit, contourMinSlider);
    _spacingCombo = new Combo(contourSpacingEdit, contourSpacingSlider);

    connect(_lineWidthCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetLineThickness(double)));
    connect(_countCombo, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumContours(int)));
    connect(_cMinCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetContourMinimum(double)));
    connect(_spacingCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetContourSpacing(double)));
    connect(boundsCombo, SIGNAL(currentIndexChanged(int)), this,
            SLOT(ContourBoundsChanged(int)));
    connect(_TFWidget, SIGNAL(emitChange()), this,
            SLOT(EndTFChange()));
}

void ContourAppearanceSubtab::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams) {
    _cParams = (VAPoR::ContourParams *)rParams;
    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;

    _lineWidth = _cParams->GetLineThickness();
    GLfloat lineWidthRange[2] = {0.f, 0.f};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    _lineWidthCombo->Update(lineWidthRange[0], lineWidthRange[1],
                            _lineWidth);

    bool lock = _cParams->GetTFLock();
    if (lock)
        boundsCombo->setCurrentIndex(1);
    else
        boundsCombo->setCurrentIndex(0);

    _numContours = _cParams->GetNumContours();
    _countCombo->Update(1, 50, _numContours);

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    _contourMin = _cParams->GetContourMin();
    _cMinCombo->Update(minBound, maxBound, _contourMin);

    _spacing = _cParams->GetContourSpacing();
    double maxSpacing = (maxBound - minBound) / (_numContours - 1);
    _spacingCombo->Update(0, maxSpacing, _spacing);

    _contourMax = _contourMin + _spacing * (_numContours - 1);
    QString QContourMax = QString::number(_contourMax);
    contourMaxLabel->setText(QContourMax);

    _TFWidget->Update(dataMgr, paramsMgr, _cParams);
    _ColorbarWidget->Update(dataMgr, paramsMgr, _cParams);
}

void ContourAppearanceSubtab::Initialize(VAPoR::ContourParams *cParams) {
    _paramsMgr->BeginSaveStateGroup("Initialize ContourAppearanceSubtab");

    _cParams = cParams;
    string varname = _cParams->GetVariableName();
    if (varname.empty())
        return;

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);

    _numContours = _cParams->GetNumContours();
    _contourMin = minBound;
    _contourMax = maxBound;
    _spacing = (_contourMax - _contourMin) / (double)(_numContours - 1);

    _cMinCombo->Update(minBound, maxBound, _contourMin);
    _countCombo->Update(1, 50, _numContours);
    _spacingCombo->Update(0, _spacing, _spacing);

    SetContourValues();

    _paramsMgr->EndSaveStateGroup();
}

void ContourAppearanceSubtab::SetContourValues() {
    vector<double> cVals;
    for (size_t i = 0; i < _numContours; i++) {
        cVals.push_back(_contourMin + _spacing * i);
    }

    string varName = _cParams->GetVariableName();
    _cParams->SetContourValues(varName, cVals);
}

void ContourAppearanceSubtab::EndTFChange() {
    bool locked = _cParams->GetTFLock();
    if (!locked)
        return;

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);

    // Check that our minimum is still valid, adjust if not
    if (_contourMin < minBound) {
        _contourMin = minBound;
        _contourMax = _contourMin + _spacing * (_numContours - 1);
    } else if (_contourMin > maxBound) {
        _contourMin = maxBound;
        _contourMax = maxBound;
    }
    _cMinCombo->Update(minBound, maxBound, _contourMin);

    // Check that our spacing is still valid, adjust if not
    if (_contourMax > maxBound) {
        _spacing = (maxBound - _contourMin) / (_numContours - 1);
    }

    double maxSpacing = (maxBound - minBound) / (_numContours - 1);
    _spacingCombo->Update(0, maxSpacing, _spacing);

    SetContourValues();
}

void ContourAppearanceSubtab::GetContourBounds(double &min, double &max) {
    double lower, upper, spacing;
    string varname = _cParams->GetVariableName();
    bool locked = _cParams->GetLockToTF();

    if (locked) {
        VAPoR::MapperFunction *tf = _cParams->GetMapperFunc(varname);
        min = tf->getMinMapValue();
        max = tf->getMaxMapValue();
    } else {
        size_t ts = _cParams->GetCurrentTimestep();
        int level = _cParams->GetRefinementLevel();
        int lod = _cParams->GetCompressionLevel();
        vector<double> minMax(2, 0);

        _dataMgr->GetDataRange(ts, varname, level, lod, minMax);
        min = minMax[0];
        max = minMax[1];
    }
}

void ContourAppearanceSubtab::disableSpacingWidgets() {
    contourSpacingSlider->setEnabled(false);
    contourSpacingEdit->setEnabled(false);
}

void ContourAppearanceSubtab::enableSpacingWidgets() {
    contourSpacingSlider->setEnabled(true);
    contourSpacingEdit->setEnabled(true);
}

// Always adjust _count here
// Never change _contourMin here
// Adjust _spacing to accomodate new contours, if _maxContour exeeds bounds
void ContourAppearanceSubtab::SetNumContours(int count) {
    // Band-aid
    // Don't let user mess with spacing if there's only one contour
    if (count == 1)
        disableSpacingWidgets();
    else
        enableSpacingWidgets();

    _numContours = count;
    _contourMax = _contourMin + _spacing * (_numContours - 1);

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    if (_contourMax > maxBound) { // Adjust spacing
        _spacing = (maxBound - _contourMin) / (_numContours - 1);
        _contourMax = _contourMin + _spacing * _numContours;
    }

    SetContourValues();
}

// Always adjust _contourMin and _contourMax here
// Never adjust _numContours here
// Adjustment of _contourMin will always adjust the spacing if our
// maximum contour is greater than our maximum bound.
void ContourAppearanceSubtab::SetContourMinimum(double min) {
    _contourMin = min;
    _contourMax = _contourMin + _spacing * (_numContours - 1);

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    if (_contourMax > maxBound) { // Adjust spacing
        _spacing = (maxBound - _contourMin) / (_numContours - 1);
        _contourMax = _contourMin + _spacing * _numContours;
    }

    SetContourValues();
}

// Always adjust _spacing and _contourMax here
// Never adjust _numContours here
// If _contourMax > maxBound, push _contourMin back until equal to minBound
void ContourAppearanceSubtab::SetContourSpacing(double spacing) {
    if (_numContours == 1)
        return;

    _spacing = spacing;
    _contourMax = _contourMin + _spacing * (_numContours - 1);

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    if (_contourMax > maxBound) {
        _contourMin = maxBound - _spacing * (_numContours - 1);
        _contourMax = maxBound;
    }

    SetContourValues();
}

void ContourAppearanceSubtab::ContourBoundsChanged(int index) {
    if (index == 0) {
        _cParams->SetLockToTF(false);
        EndTFChange();
    } else
        _cParams->SetLockToTF(true);
}
