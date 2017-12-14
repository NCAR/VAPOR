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

    double spacing = _cParams->GetContourSpacing();
    double maxSpacing = (maxBound - minBound);
    _spacingCombo->Update(0, maxSpacing, spacing);

    _contourMax = _contourMin + spacing * (_numContours - 1);
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

    int numContours = _cParams->GetNumContours();
    double contourMin = minBound;
    double contourMax = maxBound;
    double spacing = (contourMax - contourMin);

    _cMinCombo->Update(minBound, maxBound, contourMin);
    _countCombo->Update(1, 50, numContours);
    _spacingCombo->Update(0, spacing, spacing);

    SetContourValues();

    _paramsMgr->EndSaveStateGroup();
}

void ContourAppearanceSubtab::SetContourValues() {
    vector<double> cVals;

    double spacing = _cParams->GetContourSpacing();
    double contourMin = _cParams->GetContourMin();

    for (size_t i = 0; i < _numContours; i++) {
        cVals.push_back(contourMin + spacing * i);
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

    double spacing = _cParams->GetContourSpacing();
    double contourMin = _cParams->GetContourMin();
    int count = _cParams->GetContourCount();

    // Check that our minimum is still valid, adjust if not
    if (contourMin < minBound) {
        contourMin = minBound;
        _contourMax = _contourMin + spacing * (_numContours - 1);
    } else if (contourMin > maxBound) {
        contourMin = maxBound;
        _contourMax = maxBound;
    }
    _cMinCombo->Update(minBound, maxBound, contourMin);

    // Check that our spacing is still valid, adjust if not
    if (_contourMax > maxBound) {
        spacing = (maxBound - contourMin) / (count - 1);
    }
    double maxSpacing = (maxBound - minBound);
    _spacingCombo->Update(0, maxSpacing, spacing);

    // Set values in params database
    _paramsMgr->BeginSaveStateGroup("Contour TF changed");
    _cParams->SetContourSpacing(spacing);
    _cParams->SetContourMin(contourMin);
    SetContourValues();
    _paramsMgr->EndSaveStateGroup();
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
void ContourAppearanceSubtab::SetNumContours(int count) {
    // Band-aid
    // Don't let user mess with spacing if there's only one contour
    if (count == 1)
        disableSpacingWidgets();
    else
        enableSpacingWidgets();

    double spacing = _cParams->GetContourSpacing();
    double contourMin = _cParams->GetContourMin();
    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    _contourMax = contourMin + spacing * (count - 1);

    // Adjust spacing if necessary
    if (_contourMax > maxBound) { // Adjust spacing
        double contourMin = _cParams->GetContourMin();
        spacing = (maxBound - contourMin) / (count - 1);
        _contourMax = _contourMin + _spacing * _numContours;
    }

    _paramsMgr->BeginSaveStateGroup("Number of contours changed");
    _cParams->SetNumContours(count);
    _cParams->SetContourSpacing(spacing);
    _cParams->SetContourMin(contourMin);
    SetContourValues();
    _paramsMgr->EndSaveStateGroup();
}

// Always adjust contourMin and _contourMax here
// Adjust numContours here if necessary if we exceed our bounds
void ContourAppearanceSubtab::SetContourMinimum(double min) {
    _contourMin = min;
    _contourMax = _contourMin + _spacing * (_numContours - 1);

    int numContours = _cParams->GetNumContours();
    double spacing = _cParams->GetContourSpacing();

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    if (_contourMax > maxBound) { // Adjust numContours
        double range = maxBound - min;
        numContours = range / spacing;
        _contourMax = _contourMin + spacing * numContours;
    }

    _paramsMgr->BeginSaveStateGroup("Contour minimum changed");
    _cParams->SetContourMin(min);
    _cParams->SetNumContours(numContours);
    SetContourValues();
    _paramsMgr->EndSaveStateGroup();
}

// Always adjust spacing and _contourMax here
// Adjust numContours if we exceed our bounds
void ContourAppearanceSubtab::SetContourSpacing(double spacing) {
    int numContours = _cParams->GetNumContours();
    if (numContours == 1)
        return;

    double contourMin = _cParams->GetContourMin();
    _contourMax = contourMin + spacing * (numContours - 1);

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    if (_contourMax > maxBound) {
        double range = maxBound - contourMin;
        numContours = 1 + range / spacing;
        cout << "adjustig num contours via spacing change" << endl;
        _contourMax = contourMin + (numContours - 1) * spacing;
    }

    _paramsMgr->BeginSaveStateGroup("Contour spacing changed");
    _cParams->SetContourSpacing(spacing);
    _cParams->SetContourCount(numContours);
    SetContourValues();
    _paramsMgr->EndSaveStateGroup();
}

void ContourAppearanceSubtab::ContourBoundsChanged(int index) {
    if (index == 0) {
        _cParams->SetLockToTF(false);
        EndTFChange();
    } else
        _cParams->SetLockToTF(true);
}
