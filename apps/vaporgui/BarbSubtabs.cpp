#include "BarbSubtabs.h"
#include "vapor/BarbParams.h"

BarbGeometrySubtab::BarbGeometrySubtab(QWidget* parent) {
	setupUi(this);
	_geometryWidget->Reinit((GeometryWidget::Flags)
	((GeometryWidget::VECTOR) | (GeometryWidget::THREED)));

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

void BarbGeometrySubtab::Update(VAPoR::ParamsMgr* paramsMgr,
							VAPoR::DataMgr* dataMgr,
							VAPoR::RenderParams* rParams) {
	_rParams = (VAPoR::BarbParams*)rParams;
	_geometryWidget->Update(paramsMgr, dataMgr, rParams);

	vector<long> grid = _rParams->GetGrid();
	_xDimCombo->Update(1, 50, grid[0]);
	_yDimCombo->Update(1, 50, grid[1]);
	_zDimCombo->Update(1, 50, grid[2]);

	double length = _rParams->GetLengthScale();
	_lengthCombo->Update(.1, 4, length);

	double thickness = _rParams->GetLineThickness();
	_thicknessCombo->Update(.1, 1.5, thickness);
}

void BarbGeometrySubtab::xDimChanged(int i) {
	vector<long> longDims = _rParams->GetGrid();
	int dims[3];
	
	dims[0] = i;  
	dims[1] = (int)longDims[1];
	dims[2] = (int)longDims[2];
	_rParams->SetGrid(dims);
}

void BarbGeometrySubtab::yDimChanged(int i) {
	vector<long> longDims = _rParams->GetGrid();
	int dims[3];

	dims[0] = (int)longDims[0];
	dims[1] = i;
	dims[2] = (int)longDims[2];
	_rParams->SetGrid(dims);
}

void BarbGeometrySubtab::zDimChanged(int i) {
	vector<long> longDims = _rParams->GetGrid();
	int dims[3];

	dims[0] = (int)longDims[0];
	dims[1] = (int)longDims[1];
	dims[2] = i;
	_rParams->SetGrid(dims);
}

void BarbGeometrySubtab::lengthChanged(double d) {
	_rParams->SetLengthScale(d);
}

void BarbGeometrySubtab::thicknessChanged(double d) {
	_rParams->SetLineThickness(d);
}

void BarbAppearanceSubtab::Initialize(VAPoR::BarbParams* rParams) {
	float rgb[] = {1.f, 1.f, 1.f};
	rParams->SetConstantColor(rgb);
	rParams->SetColorMapVariableName("Constant");
	_TFWidget->setCMVar("Constant");
}
