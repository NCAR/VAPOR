#include "SliceSubtabs.h"

#define MIN_SAMPLES 1
#define MAX_SAMPLES 8000

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

SliceVariablesSubtab::SliceVariablesSubtab(QWidget *parent)
{
    setupUi(this);
    _variablesWidget->Reinit((VariableFlags)(SCALAR | HEIGHT), (DimFlags)(THREED));

    QButtonGroup *fidelityButtons = _variablesWidget->_fidelityWidget->GetFidelityButtons();
    connect(fidelityButtons, SIGNAL(buttonClicked(int)), this, SLOT(_setDefaultSampleRate()));
}

void SliceVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::SliceParams *>(rParams);
    assert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);
}

void SliceVariablesSubtab::_setDefaultSampleRate()
{
    std::vector<int> lods = _variablesWidget->_fidelityWidget->GetFidelityLodIdx();
    for (int i = 0; i < lods.size(); i++) cout << "lod " << i << " " << lods[i] << endl;
    int rate = _params->GetDefaultSampleRate();
    cout << "default " << rate << endl;
    std::vector<int> rates(3, rate);
    _params->SetSampleRates(rates);
}

SliceAppearanceSubtab::SliceAppearanceSubtab(QWidget *parent)
{
    setupUi(this);
    _TFWidget->Reinit((TFFlags)(0));

    _xSampleRate->SetLabel(QString::fromAscii("X sampling rate"));
    _xSampleRate->SetIntType(true);
    _xSampleRate->SetExtents(MIN_SAMPLES, MAX_SAMPLES);

    _ySampleRate->SetLabel(QString::fromAscii("Y sampling rate"));
    _ySampleRate->SetIntType(true);
    _ySampleRate->SetExtents(MIN_SAMPLES, MAX_SAMPLES);

    _zSampleRate->SetLabel(QString::fromAscii("Z sampling rate"));
    _zSampleRate->SetIntType(true);
    _zSampleRate->SetExtents(MIN_SAMPLES, MAX_SAMPLES);

    connect(_xSampleRate, SIGNAL(valueChanged(int)), this, SLOT(_xSampleRateChanged(int)));
    connect(_ySampleRate, SIGNAL(valueChanged(int)), this, SLOT(_ySampleRateChanged(int)));
    connect(_zSampleRate, SIGNAL(valueChanged(int)), this, SLOT(_zSampleRateChanged(int)));

    _xSampleRate->setEnabled(true);
    _ySampleRate->setEnabled(false);
    _zSampleRate->setEnabled(false);

    _params = NULL;
}

void SliceAppearanceSubtab::_xSampleRateChanged(int rate)
{
    std::vector<int> rates = _params->GetSampleRates();
    rates[X] = rate;
    rates[Y] = rate;
    rates[Z] = rate;
    _params->SetSampleRates(rates);
}

void SliceAppearanceSubtab::_ySampleRateChanged(int rate)
{
    std::vector<int> rates = _params->GetSampleRates();
    rates[Y] = rate;
    _params->SetSampleRates(rates);
}

void SliceAppearanceSubtab::_zSampleRateChanged(int rate)
{
    std::vector<int> rates = _params->GetSampleRates();
    rates[Z] = rate;
    _params->SetSampleRates(rates);
}

void SliceAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::SliceParams *>(rParams);
    assert(_params);

    _TFWidget->Update(dataMgr, paramsMgr, rParams);

    std::vector<double> minExt, maxExt;
    _params->GetBox()->GetExtents(minExt, maxExt);

    std::vector<int> SR;
    SR = _params->GetSampleRates();
    _xSampleRate->SetValue(SR[X]);

    /*    if (minExt[X]==maxExt[X])
        _xSampleRate->setEnabled(false);
    else
        _xSampleRate->setEnabled(true);
    if (minExt[Y]==maxExt[Y])
        _ySampleRate->setEnabled(false);
    else
        _ySampleRate->setEnabled(true);
    if (minExt[Z]==maxExt[Z])
        _zSampleRate->setEnabled(false);
    else
        _zSampleRate->setEnabled(true);
*/
}

SliceGeometrySubtab::SliceGeometrySubtab(QWidget *parent)
{
    setupUi(this);
    _geometryWidget->Reinit((DimFlags)THREED, (VariableFlags)SCALAR, (GeometryFlags)PLANAR);

    connect(_geometryWidget->_planeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(_orientationChanged(int)));

    _params = NULL;
}

void SliceGeometrySubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::SliceParams *>(rParams);
    assert(_params);

    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    _copyRegionWidget->Update(paramsMgr, rParams);
    _transformTable->Update(rParams->GetTransform());
}

void SliceGeometrySubtab::_orientationChanged(int plane)
{
    _params->GetBox()->SetOrientation(plane);
    /*    std::vector<int> sampleRates = _params->GetSampleRates();
    if (plane == XY)
        sampleRates[Z] = 1;
    if (plane == XZ)
        sampleRates[Y] = 1;
    if (plane == YZ)
        sampleRates[X] = 1;
    _params->SetSampleRates(sampleRates);*/
}

SliceAnnotationSubtab::SliceAnnotationSubtab(QWidget *parent) { setupUi(this); }

void SliceAnnotationSubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
