#include "SliceSubtabs.h"

#define MIN_SAMPLES 25
#define MAX_SAMPLES 8000

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

    // connect(_xSamplingRate
}

void SliceAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _TFWidget->Update(dataMgr, paramsMgr, rParams); }

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
