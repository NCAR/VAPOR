#include <cmath>
#include "BarbSubtabs.h"
#include "vapor/BarbParams.h"

void BarbVariablesSubtab::pushVarStartingWithLetter(
	vector<string> searchVars,
	vector<string> &returnVars, 
	char letter) {

    bool foundDefaultU = false;
    for (auto & element : searchVars) {
        if (element[0]==letter || element[0]==toupper(letter)){
            returnVars.push_back(element);
            foundDefaultU = true;
            break;
        }   
    }   
    if (!foundDefaultU)
        returnVars.push_back(searchVars[0]);
}

void BarbVariablesSubtab::Initialize(VAPoR::BarbParams* bParams,
								VAPoR::DataMgr* dataMgr) {

	string nDimsTag = _variablesWidget->getNDimsTag();
	int ndim = bParams->GetValueLong(nDimsTag, 3);
	assert(ndim==2 || ndim==3);

	vector<string> varNames = dataMgr->GetDataVarNames(3);
	vector<string> defaultVars;

	if (varNames.size() < 2) return;

	pushVarStartingWithLetter(varNames, defaultVars, 'u');
	pushVarStartingWithLetter(varNames, defaultVars, 'v');

	bParams->SetFieldVariableNames(defaultVars);
}

BarbGeometrySubtab::BarbGeometrySubtab(QWidget* parent) {
	setupUi(this);
	_geometryWidget->Reinit((GeometryWidget::Flags)
	((GeometryWidget::VECTOR) | (GeometryWidget::TWOD)));
	//((GeometryWidget::VECTOR) | (GeometryWidget::THREED)));
}

BarbAppearanceSubtab::BarbAppearanceSubtab(QWidget* parent) {
	setupUi(this);
	_TFWidget->Reinit((TFWidget::Flags)
		(TFWidget::COLORVAR | TFWidget::PRIORITY_COLORVAR | TFWidget::CONSTANT));

	hideZDimWidgets();

	_xDimCombo = new Combo(xDimEdit, xDimSlider, true);
	_yDimCombo = new Combo(yDimEdit, yDimSlider, true);
	_zDimCombo = new Combo(zDimEdit, zDimSlider, true);
	_lengthCombo = new Combo(lengthScaleEdit, lengthScaleSlider);
	_thicknessCombo = new Combo(thicknessEdit, thicknessSlider);

	connect(_xDimCombo, SIGNAL(valueChanged(int)), this,
		SLOT(xDimChanged(int)));
	connect(_yDimCombo, SIGNAL(valueChanged(int)), this,
		SLOT(yDimChanged(int)));
	connect(_zDimCombo, SIGNAL(valueChanged(int)), this,
		SLOT(zDimChanged(int)));
	connect(_lengthCombo, SIGNAL(valueChanged(double)), this,
		SLOT(lengthChanged(double)));
	connect(_thicknessCombo, SIGNAL(valueChanged(double)), this,
		SLOT(thicknessChanged(double)));
}

void BarbAppearanceSubtab::hideZDimWidgets() {
	zDimLabel->hide();
	zDimSlider->hide();
	zDimEdit->hide();
	zDimLabel->resize(0,0);
	zDimSlider->resize(0,0);
	zDimEdit->resize(0,0);
	tab->adjustSize();
	BarbLayoutTab->adjustSize();
	adjustSize();
}

double BarbAppearanceSubtab::CalculateDomainLength(int ts) {
	VAPoR::StructuredGrid* grid;
	
	double domainLength = 0;

	// Is this a legitimite way to acquire animation params?
	//
	int level = _bParams->GetRefinementLevel();
	int lod = _bParams->GetCompressionLevel();
	vector<string> fieldVars = _bParams->GetFieldVariableNames();
	for (int i=0; i<3; i++) {
		string varName = fieldVars[i];
		if ((varName) == "") {
			continue;
		}

		vector<double> minExt, maxExt;
		_dataMgr->GetVariableExtents(ts, varName, level, minExt, maxExt);

		// If we're dealing with 2D vars, skip the Z element
		//
		if (i==3 && minExt.size()<2)
			continue;
		else {
			double length = maxExt[i] - minExt[i];
			domainLength += length*length;
		}
	}
	domainLength = sqrt(domainLength);
	return domainLength;	
}

void BarbAppearanceSubtab::Update(VAPoR::DataMgr* dataMgr,
							VAPoR::ParamsMgr* paramsMgr,
							VAPoR::RenderParams* bParams) {
	_dataMgr = dataMgr;
	_bParams = (VAPoR::BarbParams*)bParams;
	_paramsMgr = paramsMgr;
	_TFWidget->Update(dataMgr, paramsMgr, bParams);
	_ColorbarWidget->Update(dataMgr, paramsMgr, bParams);

	vector<long> grid = _bParams->GetGrid();
	_xDimCombo->Update(1, 50, grid[0]);
	_yDimCombo->Update(1, 50, grid[1]);
	_zDimCombo->Update(1, 50, grid[2]);

	vector<double> minExt, maxExt;
	double length = _bParams->GetLengthScale();
	//_lengthCombo->Update(.1, 4000000, length);
	AnimationParams* ap = (AnimationParams*)paramsMgr->GetParams("AnimationParams");
	int ts = ap->GetCurrentTimestep();
	float domainLength = CalculateDomainLength(ts);
	_lengthCombo->Update(0, domainLength/50.f, length);

	double thickness = _bParams->GetLineThickness();
	//_thicknessCombo->Update(.1, 1.5, thickness);
	_thicknessCombo->Update(.1, domainLength/250000.f, thickness);
}

void BarbAppearanceSubtab::xDimChanged(int i) {
	vector<long> longDims = _bParams->GetGrid();
	int dims[3];
	
	dims[0] = i;  
	dims[1] = (int)longDims[1];
	dims[2] = (int)longDims[2];
	_bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::yDimChanged(int i) {
	vector<long> longDims = _bParams->GetGrid();
	int dims[3];

	dims[0] = (int)longDims[0];
	dims[1] = i;
	dims[2] = (int)longDims[2];
	_bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::zDimChanged(int i) {
	vector<long> longDims = _bParams->GetGrid();
	int dims[3];

	dims[0] = (int)longDims[0];
	dims[1] = (int)longDims[1];
	dims[2] = i;
	_bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::lengthChanged(double d) {
	_bParams->SetLengthScale(d);
}

void BarbAppearanceSubtab::thicknessChanged(double d) {
	_bParams->SetLineThickness(d);
}
