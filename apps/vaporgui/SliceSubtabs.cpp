#include <QButtonGroup>
#include "SliceSubtabs.h"
#include "TFEditor.h"
#include "VLineItem.h"

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

    std::vector<std::string> values = {"foo", "bar", "baz"};
    _vcb = new VComboBox(values);
    layout()->addWidget( new VLineItem("Test", _vcb ));
    connect( _vcb, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _vcbChanged( std::string )));

    _vsb = new VSpinBox( 0, 5 );
    layout()->addWidget( new VLineItem("SpinBox", _vsb ) );
    connect( _vsb, SIGNAL( ValueChanged( int )),
        this, SLOT( _vsbChanged( int )));

    _vchb = new VCheckBox( false );
    layout()->addWidget( new VLineItem("CheckBox", _vchb ) );
    connect( _vchb, SIGNAL( ValueChanged( bool )),
        this, SLOT( _vcbChanged( bool )));
   
    _vle = new VLineEdit( "lineEidt" ); 
    layout()->addWidget( new VLineItem("LineEdit", _vle ) );
    connect( _vle, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _vleChanged( std::string )));

    _vs = new VSlider( -10.0, 15.0 );
    layout()->addWidget( new VLineItem("Slider", _vs ) );
    connect( _vs, SIGNAL( ValueChangedIntermediate( double ) ),
        this, SLOT( _vsChangedIntermediate( double ) ) );
    connect( _vs, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _vsChanged( double ) ) );

    _vse = new VSliderEdit( -10.0, 1500.0, 5.0 );
    layout()->addWidget( new VLineItem("SliderEdit", _vse ) );;
    connect( _vse, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _vseChanged( double ) ) );
    connect( _vse, SIGNAL( ValueChangedIntermediate( double ) ),
        this, SLOT( _vseChangedIntermediate( double ) ) );
    _vsei = new VSliderEdit( -10.0, 1500.0, 5.0 );
    _vsei->SetIntType(true);
    layout()->addWidget( new VLineItem("IntSliderEdit", _vsei ) );;
    connect( _vsei, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _vseChanged( double ) ) );
    connect( _vse, SIGNAL( ValueChangedIntermediate( double ) ),
        this, SLOT( _vseChangedIntermediate( double ) ) );

    _pb = new VPushButton("myButton");
    layout()->addWidget( new VLineItem(" buuuutton", _pb));
    connect( _pb, SIGNAL( ButtonClicked() ),
        this, SLOT( _bChanged() ) );

    _fr = new VFileReader("fr");
    layout()->addWidget( new VLineItem("frline", _fr) );
    connect( _fr, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _frChanged() ) );
    _fw = new VFileWriter("fw");
    layout()->addWidget( new VLineItem("fwline", _fw) );
    connect( _fw, SIGNAL( ValueChanged( const std::string& val ) ),
        this, SLOT( _fwChanged() ) );
    _ds = new VDirSelector("ds");
    layout()->addWidget( new VLineItem("dsline", _ds) );
    connect( _fw, SIGNAL( ValueChanged( const std::string& val ) ),
        this, SLOT( _dsChanged() ) );
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
