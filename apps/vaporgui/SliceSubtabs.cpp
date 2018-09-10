#include "SliceSubtabs.h"

#define MIN_SAMPLES 25
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

    connect(_xSamplingRate, SIGNAL(valueChanged(int)), this, SLOT(xSamplingRateChanged(int)));
    connect(_ySamplingRate, SIGNAL(valueChanged(int)), this, SLOT(ySamplingRateChanged(int)));
    connect(_zSamplingRate, SIGNAL(valueChanged(int)), this, SLOT(zSamplingRateChanged(int)));
}

void SliceAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _TFWidget->Update(dataMgr, paramsMgr, rParams);

    Box *               box = rParams->GetBox();
    std::vector<double> minExt, maxExt;
    box->GetExtents(minExt, maxExt);

    if (minExt[X] == maxExt[X])
        _xSamplingRate->setEnabled(false);
    else
        _xSamplingRage->setEnabled(true);
    if (minExt[Y] == maxExt[Y])
        _ySamplingRate->setEnabled(false);
    else
        _ySamplingRage->setEnabled(true);
    if (minExt[Z] == maxExt[Z])
        _zSamplingRate->setEnabled(false);
    else
        _zSamplingRage->setEnabled(true);
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
