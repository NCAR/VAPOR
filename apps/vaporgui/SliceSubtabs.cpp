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
    _variablesWidget->Reinit((VariableFlags)(SCALAR), (DimFlags)(THREED));

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
    int rate = _params->GetDefaultSampleRate();
    _params->SetSampleRate(rate);
}

SliceAppearanceSubtab::SliceAppearanceSubtab(QWidget *parent)
{
    setupUi(this);
    _TFWidget->Reinit((TFFlags)(0));

    _TFWidget->mappingFrame->SetIsSlicing(true);

    _sampleRateWidget->SetLabel(QString::fromAscii("Sample rate"));
    _sampleRateWidget->SetIntType(true);
    _sampleRateWidget->SetExtents(MIN_SAMPLES, MAX_SAMPLES);

    connect(_sampleRateWidget, SIGNAL(valueChanged(int)), this, SLOT(_sampleRateChanged(int)));

    _params = NULL;
}

void SliceAppearanceSubtab::_sampleRateChanged(int rate)
{
    _params->SetSampleRate(rate);
    _TFWidget->SetAutoUpdateParamChanged(true);
    //_TFWidget->mappingFrame->RefreshHistogram(true);
}

void SliceAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::SliceParams *>(rParams);
    assert(_params);

    _TFWidget->Update(dataMgr, paramsMgr, rParams);

    std::vector<double> minExt, maxExt;
    _params->GetBox()->GetExtents(minExt, maxExt);

    int sampleRate = _params->GetSampleRate();
    _sampleRateWidget->SetValue(sampleRate);

    //_TFWidget->mappingFrame->RefreshHistogram(true);
    //_TFWidget->RefreshHistogram();
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

void SliceGeometrySubtab::_orientationChanged(int plane) { _params->GetBox()->SetOrientation(plane); }

SliceAnnotationSubtab::SliceAnnotationSubtab(QWidget *parent) { setupUi(this); }

void SliceAnnotationSubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
