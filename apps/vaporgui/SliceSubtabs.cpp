#include <QButtonGroup>
#include "SliceSubtabs.h"
#include "TFEditor.h"
#include "VLineItem.h"
#include "VStringLineEdit.h"
#include "VIntLineEdit.h"
#include "VDoubleLineEdit.h"
#include "VIntSliderEdit.h"
#include "VDoubleSliderEdit.h"

#define MIN_SAMPLES 1 
#define MAX_SAMPLES 2000
#define MIN_QUALITY 1
#define MAX_QUALITY 10
#define DEFAULT_QUALITY 1
#define SAMPLES_PER_QUALITY 200

#define X 0
#define Y 1
#define Z 2
#define XY 0
#define XZ 1 
#define YZ 2

SliceVariablesSubtab::SliceVariablesSubtab(QWidget* parent) {
    setupUi(this);
    _variablesWidget->Reinit(
        (VariableFlags)(SCALAR),
        (DimFlags)(THREED)
    );

    QButtonGroup* fidelityButtons = _variablesWidget->_fidelityWidget->GetFidelityButtons();
    connect(fidelityButtons, SIGNAL(buttonClicked(int)),
        this, SLOT(_setDefaultSampleRate()));
	QComboBox* refinementCombo = _variablesWidget->_fidelityWidget->refinementCombo;
	connect(refinementCombo, SIGNAL(currentIndexChanged(int)),
		this, SLOT(_setDefaultSampleRate()));

    VLineItem* vli;

    VStringLineEdit* vsle = new VStringLineEdit( "woot" );
    vli = new VLineItem( "VStringLineEdit", vsle );
    layout()->addWidget( vli );
    connect( vsle, SIGNAL( ValueChanged( const std::string& ) ), 
        this, SLOT( testVStringLineEdit( const std::string& ) ) );

    VDoubleLineEdit* vdle = new VDoubleLineEdit( 9.8765 );
    vli = new VLineItem( "VDoubleLineEdit", vdle );
    layout()->addWidget( vli );
    connect( vdle, SIGNAL( ValueChanged( double ) ), 
        this, SLOT( testVDoubleLineEdit( double ) ) );

    VIntLineEdit* vile3 = new VIntLineEdit( 5 );
    vli = new VLineItem( "VIntLineEdit", vile3 );
    layout()->addWidget( vli );
    connect( vile3, SIGNAL( ValueChanged( int ) ), 
        this, SLOT( testVIntLineEdit( int ) ) );

    VIntSliderEdit* visei = new VIntSliderEdit(0, 10, false );
    vli = new VLineItem( "VIntSliderEdit immutable", visei );
    layout()->addWidget( vli );
    connect( visei, SIGNAL( ValueChanged( int ) ),
        this, SLOT( testVIntSliderEdit( int ) ) );

    VIntSliderEdit* vise = new VIntSliderEdit(0, 10, true );
    vli = new VLineItem( "VIntSliderEdit", vise );
    layout()->addWidget( vli );
    connect( vise, SIGNAL( ValueChanged( int ) ),
        this, SLOT( testVIntSliderEdit( int ) ) );

    VDoubleSliderEdit* vdsei = new VDoubleSliderEdit( 0, 10, 0, false );
    vli = new VLineItem( "VDoubleSliderEdit immutable", vdsei );
    layout()->addWidget( vli );
    connect( vdsei, SIGNAL( ValueChanged( double ) ),
        this, SLOT( testVDoubleSliderEdit( double ) ) );
    
    VDoubleSliderEdit* vdse = new VDoubleSliderEdit( 0, 10, 0, true );
    vli = new VLineItem( "VDoubleSliderEdit", vdse );
    layout()->addWidget( vli );
    connect( vdse, SIGNAL( ValueChanged( double ) ),
        this, SLOT( testVDoubleSliderEdit( double ) ) );
}

void SliceVariablesSubtab::testVIntLineEdit( int value ) {
    std::cout << "testVIntLineEdit " << value << std::endl;
}
void SliceVariablesSubtab::testVDoubleLineEdit( double value ) {
    std::cout << "testVDoubleLineEdit " << value << std::endl;
}
void SliceVariablesSubtab::testVStringLineEdit( const std::string& value ) {
    std::cout << "testVStringLineEdit " << value << std::endl;
}

void SliceVariablesSubtab::testVIntSliderEdit( int value ) {
    std::cout << "VIntSliderEdit value " << value << std::endl;
}

void SliceVariablesSubtab::testVDoubleSliderEdit( double value ) {
    std::cout << "VDoubleSliderEdit value " << value << std::endl;
}

void SliceVariablesSubtab::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams
) {
    _params = dynamic_cast<VAPoR::SliceParams*>(rParams);
    VAssert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);
}

void SliceVariablesSubtab::_setDefaultSampleRate() {
    int defaultRate  = _params->GetDefaultSampleRate();
    int quality      = defaultRate / SAMPLES_PER_QUALITY;
    if (quality < 1) quality = 1;
    int adjustedRate = quality * SAMPLES_PER_QUALITY;
    _params->SetSampleRate(adjustedRate);
}

SliceAppearanceSubtab::SliceAppearanceSubtab(QWidget* parent) {
    setupUi(this);
    verticalLayout->insertWidget(0, _tfe = new TFEditor);

    _sampleRateWidget->SetLabel( QString("Quality") );
    _sampleRateWidget->SetIntType(true);
    _sampleRateWidget->SetExtents(MIN_QUALITY, MAX_QUALITY);

    connect(_sampleRateWidget, SIGNAL(valueChanged(int)), 
        this, SLOT(_qualityChanged(int)));

    _params = NULL;
}

void SliceAppearanceSubtab::_qualityChanged(int quality) {
    int sampleRate = quality * SAMPLES_PER_QUALITY;
    _params->SetSampleRate(sampleRate);
}

void SliceAppearanceSubtab::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams
) {
    _params = dynamic_cast<VAPoR::SliceParams*>(rParams);
    VAssert(_params);
    _tfe->Update(dataMgr, paramsMgr, rParams);

    std::vector<double> minExt, maxExt;
    _params->GetBox()->GetExtents(minExt, maxExt);

    int sampleRate = _params->GetSampleRate();
    int quality = sampleRate / SAMPLES_PER_QUALITY;
    _sampleRateWidget->SetValue(quality);
}

SliceGeometrySubtab::SliceGeometrySubtab(QWidget* parent) 
{
    setupUi(this);
    _geometryWidget->Reinit(
        (DimFlags)THREED,
        (VariableFlags)SCALAR,
        (GeometryFlags)PLANAR
    );

    connect(_geometryWidget->_planeComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(_orientationChanged(int)));

    _params = NULL;
}

void SliceGeometrySubtab::Update(
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::DataMgr *dataMgr,
    VAPoR::RenderParams *rParams
) {
    _params = dynamic_cast<VAPoR::SliceParams*>(rParams);
    VAssert(_params);

    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    _copyRegionWidget->Update(paramsMgr, rParams);
    _transformTable->Update(rParams->GetTransform());
}

void SliceGeometrySubtab::_orientationChanged(int plane) {
    _params->GetBox()->SetOrientation(plane);
}

SliceAnnotationSubtab::SliceAnnotationSubtab(QWidget* parent) {
    setupUi(this);
}

void SliceAnnotationSubtab::Update(
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::DataMgr *dataMgr,
    VAPoR::RenderParams *rParams
) {
    _colorbarWidget->Update(dataMgr, paramsMgr, rParams);
}
