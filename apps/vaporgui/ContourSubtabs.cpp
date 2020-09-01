#include "ContourSubtabs.h"
#include "TFEditor.h"
#include "PSection.h"
#include "PFidelitySection.h"
#include <vapor/glutil.h>

#include "VStringLineEdit.h"
#include "VIntLineEdit.h"
#include "VDoubleLineEdit.h"
#include "VIntSliderEdit.h"
#include "VDoubleSliderEdit.h"

ContourVariablesSubtab::ContourVariablesSubtab(QWidget* parent) {
    setLayout( new QVBoxLayout );
    ((QVBoxLayout*)layout())->insertWidget(1, pg = new PGroup);
    PSection *vars = new PSection("Variable Selection");
    vars->Add(new PScalarVariableSelector2DHLI);
    vars->Add(new PHeightVariableSelectorHLI);
    pg->Add(vars);
    pg->Add(new PFidelitySection);

    VLineItem* vli;

    /*VStringLineEdit* vsle = new VStringLineEdit( "woot" );
    vsle->setToolTip( "VSLE" );
    vli = new VLineItem( "VStringLineEdit", vsle );
    vli->setToolTip( "VLI" );
    layout()->addWidget( vli );
    connect( vsle, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( testVStringLineEdit( const std::string& ) ) );

    VDoubleLineEdit* vdle = new VDoubleLineEdit();
    vli = new VLineItem( "VDoubleLineEdit", vdle );
    layout()->addWidget( vli );
    connect( vdle, SIGNAL( ValueChanged( double ) ),
        this, SLOT( testVDoubleLineEdit( double ) ) );

    VIntLineEdit* vile3 = new VIntLineEdit();
    vli = new VLineItem( "VIntLineEdit", vile3 );
    layout()->addWidget( vli );
    connect( vile3, SIGNAL( ValueChanged( int ) ),
        this, SLOT( testVIntLineEdit( int ) ) );    */

/*VIntSliderEdit* visei = new VIntSliderEdit(0, 10, 3, false );
vli = new VLineItem( "VIntSliderEdit immutable", visei );
layout()->addWidget( vli );
connect( visei, SIGNAL( ValueChanged( int ) ),
    this, SLOT( testVIntSliderEdit( int ) ) );  */

    /*VIntSliderEdit* vise = new VIntSliderEdit(0, 10, 3, true );
    vli = new VLineItem( "VIntSliderEdit", vise );
    layout()->addWidget( vli );
    connect( vise, SIGNAL( ValueChanged( int ) ),
        this, SLOT( testVIntSliderEdit( int ) ) );  */

VDoubleSliderEdit* vdsei = new VDoubleSliderEdit( 0, 10, 0, false );
vli = new VLineItem( "VDoubleSliderEdit immutable", vdsei );
layout()->addWidget( vli );
connect( vdsei, SIGNAL( ValueChanged( double ) ),
    this, SLOT( testVDoubleSliderEdit( double ) ) );

    /*VDoubleSliderEdit* vdse = new VDoubleSliderEdit( 0, 10, 0, true );
    vli = new VLineItem( "VDoubleSliderEdit", vdse );
    layout()->addWidget( vli );
    connect( vdse, SIGNAL( ValueChanged( double ) ),
        this, SLOT( testVDoubleSliderEdit( double ) ) );*/
}

void ContourVariablesSubtab::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams
) {
    pg->Update(rParams, paramsMgr, dataMgr);
}

ContourAppearanceSubtab::ContourAppearanceSubtab(QWidget* parent) {
	setupUi(this);
    
    ((QVBoxLayout*)layout())->insertWidget(0, _tfEditor = new TFEditor);

	_countCombo = new Combo(contourCountEdit, contourCountSlider, true);
	_cMinCombo = new Combo(contourMinEdit, contourMinSlider);
	_spacingCombo = new Combo(contourSpacingEdit, contourSpacingSlider);
	
	connect(_countCombo, SIGNAL(valueChanged(int)), this,
		SLOT(SetContourCount(int)));
	connect(_cMinCombo, SIGNAL(valueChanged(double)), this,
		SLOT(SetContourMinimum(double)));
	connect(_spacingCombo, SIGNAL(valueChanged(double)), this,
		SLOT(SetContourSpacing(double)));
}

void ContourAppearanceSubtab::Update(
	VAPoR::DataMgr *dataMgr,
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::RenderParams *rParams
) {
	_cParams = (VAPoR::ContourParams*)rParams;
	_dataMgr = dataMgr;
	_paramsMgr = paramsMgr;

	double count = _cParams->GetContourCount();
	_countCombo->Update(1, 50, count);

	double minBound, maxBound;
	GetContourBounds(minBound, maxBound);
	double contourMin = _cParams->GetContourMin();
	_cMinCombo->Update(minBound, maxBound, contourMin);

	double spacing = _cParams->GetContourSpacing();
	double maxSpacing = (maxBound - minBound);
	_spacingCombo->Update(0, maxSpacing, spacing);
    
    _tfEditor->Update(dataMgr, paramsMgr, _cParams);
}

void ContourAppearanceSubtab::Initialize(VAPoR::ContourParams* cParams) {
	_paramsMgr->BeginSaveStateGroup("Initialize ContourAppearanceSubtab");

	_cParams = cParams;
	string varname = _cParams->GetVariableName();
    if (varname.empty()) {
        _paramsMgr->EndSaveStateGroup();
        return;
    }

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

	double minBound, maxBound;
	GetContourBounds(minBound, maxBound);
	if (maxBound < cVals[contourCount-1]) {
		contourCountEdit->setStyleSheet("background-color: red; "
			"color: white;");
		contourCountEdit->setToolTip(
			"If the Contour Count display is red, one or\n"
			"more contours have exeeded the bounds of the\n"
			"current variable data range.  Not all contours\n"
			"indicated by Count are being rendered.");
	}
	else {
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
	string varname = _cParams->GetVariableName();
	
	size_t ts = _cParams->GetCurrentTimestep();
	int level = _cParams->GetRefinementLevel();
	int lod = _cParams->GetCompressionLevel();
	vector<double> minMax(2,0);

    vector<double> minExt, maxExt;
    _cParams->GetBox()->GetExtents(minExt, maxExt);

	_dataMgr->GetDataRange(ts, varname, level, lod, minExt, maxExt, minMax);
	min = minMax[0];
	max = minMax[1];
}

void ContourAppearanceSubtab::SetContourCount(int count) {
	double spacing = _cParams->GetContourSpacing();
	double contourMin = _cParams->GetContourMin();

	SetContourValues(count, contourMin, spacing);
}

void ContourAppearanceSubtab::SetContourMinimum(double min) {
	double spacing = _cParams->GetContourSpacing();
	int count = _cParams->GetContourCount();

	SetContourValues(count, min, spacing);
}

void ContourAppearanceSubtab::SetContourSpacing(double spacing) {
	int count = _cParams->GetContourCount();
	if (count == 1) return;

	double min = _cParams->GetContourMin();

	SetContourValues(count, min, spacing);
}

ContourGeometrySubtab::ContourGeometrySubtab(QWidget* parent) {
	setupUi(this);
	_geometryWidget->Reinit(
		(DimFlags)TWOD,
		(VariableFlags)SCALAR
	);

	_orientationAngles->hide();
}
