#include "SliceSubtabs.h"

#define MIN_SAMPLES 1
#define MAX_SAMPLES 8000

#define X 0
#define Y 1
#define Z 2

SliceVariablesSubtab::SliceVariablesSubtab(QWidget *parent)
{
    setupUi(this);
    _variablesWidget->Reinit((VariableFlags)(SCALAR | HEIGHT), (DimFlags)(THREED));
}

void SliceVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }

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
}

void SliceAppearanceSubtab::_xSampleRateChanged(int rate)
{
    std::vector<int> rates = _params->GetSampleRates();
    rates[X] = rate;
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
    rParams->GetBox()->GetExtents(minExt, maxExt);

    if (minExt[X] == maxExt[X])
        _xSampleRate->setEnabled(false);
    else
        _xSampleRate->setEnabled(true);
    if (minExt[Y] == maxExt[Y])
        _ySampleRate->setEnabled(false);
    else
        _ySampleRate->setEnabled(true);
    if (minExt[Z] == maxExt[Z])
        _zSampleRate->setEnabled(false);
    else
        _zSampleRate->setEnabled(true);
}

SliceGeometrySubtab::SliceGeometrySubtab(QWidget *paremt)
{
    setupUi(this);
    _geometryWidget->Reinit((DimFlags)THREED, (VariableFlags)SCALAR, (GeometryFlags)PLANAR);
}

void SliceGeometrySubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
{
    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    _copyRegionWidget->Update(paramsMgr, rParams);
    _transformTable->Update(rParams->GetTransform());
}

SliceAnnotationSubtab::SliceAnnotationSubtab(QWidget *parent) { setupUi(this); }

void SliceAnnotationSubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
