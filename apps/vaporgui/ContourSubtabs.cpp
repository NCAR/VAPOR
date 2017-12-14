#include "ContourSubtabs.h"

ContourAppearanceSubtab::ContourAppearanceSubtab(QWidget* parent) {
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
	connect(boundsCombo, SIGNAL(currentIndexChanged(int)), this,
		SLOT(ContourBoundsChanged(int)));
	connect(_TFWidget, SIGNAL(emitChange()), this,
		SLOT(EndTFChange()));
}

void ContourAppearanceSubtab::Update(
	VAPoR::DataMgr *dataMgr,
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::RenderParams *rParams
) {
	_cParams = (VAPoR::ContourParams*)rParams;
	_dataMgr = dataMgr;
	_paramsMgr = paramsMgr;

	double lineWidth = _cParams->GetLineThickness();
	GLfloat lineWidthRange[2] = {0.f, 0.f};
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
	_lineWidthCombo->Update(lineWidthRange[0], lineWidthRange[1],
		lineWidth
	);

	bool lock = _cParams->GetTFLock();
	if (lock) boundsCombo->setCurrentIndex(1);
	else boundsCombo->setCurrentIndex(0);

	double count = _cParams->GetContourCount();
	_countCombo->Update(1, 50, count);

	double minBound, maxBound;
	GetContourBounds(minBound, maxBound);
	double contourMin = _cParams->GetContourMin();
	_cMinCombo->Update(minBound, maxBound, contourMin);

	double spacing = _cParams->GetContourSpacing();
	double maxSpacing = (maxBound - minBound);
	_spacingCombo->Update(0, maxSpacing, spacing);

	double contourMax = contourMin + (count-1)*(spacing);
	contourMax = contourMin + spacing*(count-1);
	QString QContourMax = QString::number(contourMax);
	contourMaxLabel->setText(QContourMax);

	_TFWidget->Update(dataMgr, paramsMgr, _cParams);
	_ColorbarWidget->Update(dataMgr, paramsMgr, _cParams);
}

void ContourAppearanceSubtab::Initialize(VAPoR::ContourParams* cParams) {
	_paramsMgr->BeginSaveStateGroup("Initialize ContourAppearanceSubtab");

	_cParams = cParams;
	string varname = _cParams->GetVariableName();
	if (varname.empty()) return;

	double minBound, maxBound;
	GetContourBounds(minBound, maxBound);

	int count = _cParams->GetContourCount();
	double contourMin = minBound;
	double contourMax = maxBound;
	double spacing = (contourMax - contourMin) / (count-1);

	_cMinCombo->Update(minBound, maxBound, contourMin);
	_countCombo->Update(1, 50, count);
	_spacingCombo->Update(0, spacing, spacing);

	SetContourValues(count, contourMin, spacing); 
	
	_paramsMgr->EndSaveStateGroup();
}

void ContourAppearanceSubtab::SetContourValues(
		int contourCount,
		double contourMin,
		double spacing)
	{
	vector<double> cVals;

	for (size_t i=0; i<contourCount; i++) {
		cVals.push_back(contourMin + spacing*i);
	}

	string varName = _cParams->GetVariableName();
	_cParams->SetContourValues(varName, cVals);
}

void ContourAppearanceSubtab::EndTFChange() {
	bool locked = _cParams->GetTFLock();
	if (!locked) return;

	double minBound, maxBound;
	GetContourBounds(minBound, maxBound);


	double spacing = _cParams->GetContourSpacing();
	double contourMin = _cParams->GetContourMin();
	int count = _cParams->GetContourCount();
	double contourMax = contourMin + spacing*(count-1);

	// Check that our minimum is still valid, adjust if not
	if (contourMin < minBound) {
		contourMin = minBound;
		contourMax = contourMin + spacing*(count-1);
	}
	if (contourMax > maxBound) {
		contourMax = maxBound;
		spacing = (contourMax - contourMin) / (count-1);
	}
	_cMinCombo->Update(minBound, maxBound, contourMin);

	SetContourValues(count, contourMin, spacing);
}

void ContourAppearanceSubtab::GetContourBounds(double &min, double &max) {
	double lower, upper, spacing; 
	string varname = _cParams->GetVariableName();
	bool locked = _cParams->GetLockToTF();
	
	if (locked){
		VAPoR::MapperFunction* tf = _cParams->GetMapperFunc(varname);
		min = tf->getMinMapValue();
		max = tf->getMaxMapValue();
	}
	else {
		size_t ts = _cParams->GetCurrentTimestep();
		int level = _cParams->GetRefinementLevel();
		int lod = _cParams->GetCompressionLevel();
		vector<double> minMax(2,0);

		_dataMgr->GetDataRange(ts, varname, level, lod, minMax);
		min = minMax[0];
		max = minMax[1];
	}
}

// Always adjust _count here
// Never change _contourMin here
void ContourAppearanceSubtab::SetContourCount(int count) {
	double spacing = _cParams->GetContourSpacing();
	double contourMin = _cParams->GetContourMin();
	double minBound, maxBound;
	double contourMax = contourMin + spacing*(count-1);

	// Adjust spacing if necessary
	GetContourBounds(minBound, maxBound);
	if (contourMax > maxBound) {  // Adjust spacing
		double contourMin = _cParams->GetContourMin();
		spacing = (maxBound - contourMin) / (count - 1);
	}

	SetContourValues(count, contourMin, spacing);
}

// Always adjust contourMin and _contourMax here
// Adjust count here if necessary if we exceed our bounds
void ContourAppearanceSubtab::SetContourMinimum(double min) {
	double spacing = _cParams->GetContourSpacing();
	int count = _cParams->GetContourCount();
	double contourMax = min + spacing*(count-1);

	double minBound, maxBound;
	GetContourBounds(minBound, maxBound);
	if (contourMax > maxBound) {  // Adjust count
		double range = maxBound - min;
		count = 1 + range/spacing;
	}
	
	SetContourValues(count, min, spacing);
}

// Always adjust spacing and _contourMax here
// Adjust count if we exceed our bounds
void ContourAppearanceSubtab::SetContourSpacing(double spacing) {
	int count = _cParams->GetContourCount();
	if (count == 1) return;

	double min = _cParams->GetContourMin();
	double contourMax = min + spacing*(count-1);

	double minBound, maxBound;
	GetContourBounds(minBound, maxBound);
	if (contourMax > maxBound) {
		double range = maxBound - min;
		count = 1 + range/spacing;
	}
	
	SetContourValues(count, min, spacing);
}

void ContourAppearanceSubtab::ContourBoundsChanged(int index) {
	if (index==0) {
		_cParams->SetLockToTF(false);
		EndTFChange();
	}
	else
		_cParams->SetLockToTF(true);
}

