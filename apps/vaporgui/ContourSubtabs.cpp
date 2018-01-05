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
            SLOT(SetContourCount(int)));
    connect(_cMinCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetContourMinimum(double)));
    connect(_spacingCombo, SIGNAL(valueChanged(double)), this,
            SLOT(SetContourSpacing(double)));
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

    double lineWidth = _cParams->GetLineThickness();
    GLfloat lineWidthRange[2] = {0.f, 0.f};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    _lineWidthCombo->Update(lineWidthRange[0], lineWidthRange[1],
                            lineWidth);

    double count = _cParams->GetContourCount();
    _countCombo->Update(1, 50, count);

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    double contourMin = _cParams->GetContourMin();
    _cMinCombo->Update(minBound, maxBound, contourMin);

    double spacing = _cParams->GetContourSpacing();
    double maxSpacing = (maxBound - minBound);
    _spacingCombo->Update(0, maxSpacing, spacing);

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

    int count = _cParams->GetContourCount();
    double contourMin = minBound;
    double contourMax = maxBound;
    double spacing = (contourMax - contourMin) / (count - 1);

    _cMinCombo->Update(minBound, maxBound, contourMin);
    _countCombo->Update(1, 50, count);
    _spacingCombo->Update(0, spacing, spacing);

    SetContourValues(count, contourMin, spacing);

    _paramsMgr->EndSaveStateGroup();
}

void ContourAppearanceSubtab::SetContourValues(
    int contourCount,
    double contourMin,
    double spacing) {
    vector<double> cVals;

    for (size_t i = 0; i < contourCount; i++) {
        cVals.push_back(contourMin + spacing * i);
    }

    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);
    if (maxBound < cVals[contourCount - 1]) {
        contourCountEdit->setStyleSheet("background-color: red; "
                                        "color: white;");
        contourCountEdit->setToolTip(
            "If the Contour Count display is red, one or\n"
            "more contours have exeeded the bounds of the\n"
            "current variable data range.  Not all contours\n"
            "indicated by Count are being rendered.");
    } else {
        contourCountEdit->setStyleSheet("background-color: white; "
                                        "color: black;");
        contourCountEdit->setToolTip("");
    }

    string varName = _cParams->GetVariableName();
    _cParams->SetContourValues(varName, cVals);
}

void ContourAppearanceSubtab::EndTFChange() {
    double minBound, maxBound;
    GetContourBounds(minBound, maxBound);

    double spacing = _cParams->GetContourSpacing();
    double contourMin = _cParams->GetContourMin();
    int count = _cParams->GetContourCount();

    _cMinCombo->Update(minBound, maxBound, contourMin);

    SetContourValues(count, contourMin, spacing);
}

void ContourAppearanceSubtab::GetContourBounds(double &min, double &max) {
    double lower, upper, spacing;
    string varname = _cParams->GetVariableName();
    bool locked = _cParams->GetLockToTF();

    size_t ts = _cParams->GetCurrentTimestep();
    int level = _cParams->GetRefinementLevel();
    int lod = _cParams->GetCompressionLevel();
    vector<double> minMax(2, 0);

    _dataMgr->GetDataRange(ts, varname, level, lod, minMax);
    min = minMax[0];
    max = minMax[1];
}

void ContourAppearanceSubtab::SetContourCount(int count) {
    double spacing = _cParams->GetContourSpacing();
    double contourMin = _cParams->GetContourMin();
    double minBound, maxBound;

    SetContourValues(count, contourMin, spacing);
}

void ContourAppearanceSubtab::SetContourMinimum(double min) {
    double spacing = _cParams->GetContourSpacing();
    int count = _cParams->GetContourCount();

    SetContourValues(count, min, spacing);
}

void ContourAppearanceSubtab::SetContourSpacing(double spacing) {
    int count = _cParams->GetContourCount();
    if (count == 1)
        return;

    double min = _cParams->GetContourMin();

    SetContourValues(count, min, spacing);
}

void ContourAppearanceSubtab::ContourBoundsChanged(int index) {
    if (index == 0) {
        _cParams->SetLockToTF(false);
        EndTFChange();
    } else
        _cParams->SetLockToTF(true);
}
