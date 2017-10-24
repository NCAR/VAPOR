#include "ContourSubtabs.h"

ContourAppearanceSubtab::ContourAppearanceSubtab(QWidget* parent) {
		setupUi(this);

		_TFWidget->Reinit((TFWidget::Flags)(0));
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

		connect(_TFWidget->mappingFrame, SIGNAL(mappingChanged()), this,
			SLOT(MappingChanged()));
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
		double min = _cParams->GetContourMin();
		string varname = _cParams->GetVariableName();
		double spacing, maxSpacing;
		if (locked) {
			// Update contour minimum combo
			//
			VAPoR::MapperFunction* mf = _cParams->GetMapperFunc(varname);
			double lower = mf->getMinMapValue();
			double upper = mf->getMaxMapValue();
			if (minOrMax == "max") return upper;
			else return lower;
		}
		else {
			// Apply settings to contour minimum and spacing, bounded only
			// by the min/max values of the variable.
			//
			int lod = _cParams->GetCompressionLevel();
			int level = _cParams->GetRefinementLevel();
			int ts = _cParams->GetCurrentTimestep();
			VAPoR::Grid* var = _dataMgr->GetVariable(ts, varname, level, lod);
			float range[2];
			var->GetRange(range);
			if (minOrMax == "max") return range[1];
			else return range[0];
		}   
	}

	void ContourAppearanceSubtab::Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		_cParams = (VAPoR::ContourParams*)rParams;
		_dataMgr = dataMgr;
		_paramsMgr = paramsMgr;

		_TFWidget->Update(dataMgr, paramsMgr, _cParams);
		_ColorbarWidget->Update(dataMgr, paramsMgr, _cParams);

		// Apply params to lineThickness.  Get range
		// for thickness from gl system call
		//
		GLfloat lineWidthRange[2] = {0.f, 0.f};
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
		_lineWidthCombo->Update(lineWidthRange[0], lineWidthRange[1],
			_cParams->GetLineThickness()
		);

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

		// Update contour spacing and minimum settings, which may
		// or may not be locked within the transfer function bounds.
		//
		double minComboMin = GetContourMinOrMax("min");
		double minComboMax = GetContourMinOrMax("max");
		double minVal = _cParams->GetContourMin();
		_cMinCombo->Update(minComboMin, minComboMax, minVal);

		double spacing = _cParams->GetContourSpacing();
		double maxSpacing = (minComboMax - minVal) / (double)(numContours-1);
		if (spacing > maxSpacing) spacing = maxSpacing;
		_spacingCombo->Update(0, maxSpacing, spacing);
	}

	void ContourAppearanceSubtab::Initialize(VAPoR::ContourParams* cParams) {
		_paramsMgr->BeginSaveStateGroup("Initialize ContourAppearanceSubtab");

		_cParams = cParams;
		string varname = _cParams->GetVariableName();
		VAPoR::MapperFunction* mf = _cParams->GetMapperFunc(varname);
		double lower = mf->getMinMapValue();
		double upper = mf->getMaxMapValue();
		int count = 7;
		double spacing = (upper - lower) / (double)(count-1);
	
		_cParams->SetNumContours(count);
		_cParams->SetContourMin(lower);
		_cParams->SetContourSpacing(spacing);
		SetIsovalues(); 
		
		_paramsMgr->EndSaveStateGroup();
	}

	// Do not BeginSaveStateGroup here!  This funciton
	// is always encapsulated by a Begin/EndSaveStateGroup.
	//
	void ContourAppearanceSubtab::SetIsovalues() {
		vector<double> cVals;
		int numContours = _cParams->GetNumContours();
		double spacing = _cParams->GetContourSpacing();
		double min = _cParams->GetContourMin();
		for (size_t i=0; i<numContours; i++) {
			cVals.push_back(min + spacing*i);
		}
		_cParams->SetIsovalues(cVals);
	}

	void ContourAppearanceSubtab::EndTFChange() {
		_paramsMgr->BeginSaveStateGroup("Transfer function change completed."
			"Update contours.");


		double min = GetContourMinOrMax("min");
		double max = GetContourMinOrMax("max");
		double minVal = _cParams->GetContourMin();
		if (minVal < min) minVal = min;
		if (minVal > max) minVal = max;
		_cMinCombo->Update(min, max, minVal);
		_cParams->SetContourMin(minVal);

		int numContours = _cParams->GetNumContours();
		double spacing = _cParams->GetContourSpacing();
		double maxSpacing = (max - minVal) / (double)(numContours-1);
		double span = spacing*(numContours-1) + minVal;
		
		if ((span > max) || (_cParams->GetLockToTF())) {
			 spacing = maxSpacing;
		}
		
		_spacingCombo->Update(0, maxSpacing, spacing);
		_cParams->SetContourSpacing(spacing);
		SetIsovalues();
		
		_paramsMgr->EndSaveStateGroup();
	}

	void ContourAppearanceSubtab::SetContourCount(int count) {
		_paramsMgr->BeginSaveStateGroup("Set contour count.");

		if (count < 1) count = 1;

		bool locked = _cParams->GetLockToTF();
		string varname = _cParams->GetVariableName();

		// If we're locked to the transfer function and our span exceeds
		// the TF max value, adjust our spacing to make room for the added
		// contours
		//
		if (locked) {
			VAPoR::MapperFunction* mf = _cParams->GetMapperFunc(varname);
			double lower = mf->getMinMapValue();
			double upper = mf->getMaxMapValue();
			double spacing = (upper - lower) / (double)(count-1);
			_cParams->SetContourSpacing(spacing);
		}
		_cParams->SetNumContours(count);
		SetIsovalues();

		_paramsMgr->EndSaveStateGroup();
	}

	void ContourAppearanceSubtab::SetContourMinimum(double min) {
		_paramsMgr->BeginSaveStateGroup("Set contour minimum.");

		double minRange = GetContourMinOrMax("min");
		double maxRange = GetContourMinOrMax("max");

		if (min < minRange) min = minRange;
		if (min > maxRange) min = maxRange;

		int numContours = _cParams->GetNumContours();
		double spacing = (maxRange - min) / (double)(numContours-1);
		_cParams->SetContourSpacing(spacing);

		double test = GetContourMinOrMax("min");

		_cParams->SetContourMin(min);

		SetIsovalues();
		_paramsMgr->EndSaveStateGroup();
	}

	void ContourAppearanceSubtab::SetContourSpacing(double spacing) {
		_paramsMgr->BeginSaveStateGroup("Set contour spacing.");

		bool locked = _cParams->GetLockToTF();
		double min = _cParams->GetContourMin();
		string varname = _cParams->GetVariableName();
		int numContours = _cParams->GetNumContours();
		double maxSpacing;

		if (numContours <= 1) {
			_cParams->SetContourSpacing(1);
			return;
		}

		int lod = _cParams->GetCompressionLevel();
		int level = _cParams->GetRefinementLevel();
		int ts = _cParams->GetCurrentTimestep();
		VAPoR::Grid* var = _dataMgr->GetVariable(ts, varname, level, lod);
		float range[2];
		var->GetRange(range);
		maxSpacing = (range[1] - range[0])/(double)(numContours-1);
		if (spacing > maxSpacing) spacing = maxSpacing;
		_cParams->SetContourSpacing(spacing);
		SetIsovalues();
		
		_paramsMgr->EndSaveStateGroup();
	}

	void ContourAppearanceSubtab::LockToTFChecked(bool checked) {
		_cParams->SetLockToTF(checked);
		if (checked) {
			EndTFChange();
		}
	}
