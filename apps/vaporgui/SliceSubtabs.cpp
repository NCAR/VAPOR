#include <QButtonGroup>
#include "SliceSubtabs.h"
#include "TFEditor.h"
#include "VLineItem.h"

#define MIN_SAMPLES         1
#define MAX_SAMPLES         2000
#define MIN_QUALITY         1
#define MAX_QUALITY         10
#define DEFAULT_QUALITY     1
#define SAMPLES_PER_QUALITY 200

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
    QComboBox *refinementCombo = _variablesWidget->_fidelityWidget->refinementCombo;
    connect(refinementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_setDefaultSampleRate()));
}

void SliceVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::SliceParams *>(rParams);
    VAssert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);
}

void SliceVariablesSubtab::_setDefaultSampleRate()
{
    int defaultRate = _params->GetDefaultSampleRate();
    int quality = defaultRate / SAMPLES_PER_QUALITY;
    if (quality < 1) quality = 1;
    int adjustedRate = quality * SAMPLES_PER_QUALITY;
    _params->SetSampleRate(adjustedRate);
}

SliceAppearanceSubtab::SliceAppearanceSubtab(QWidget *parent)
{
    setupUi(this);
    verticalLayout->insertWidget(0, _tfe = new TFEditor);

    _sampleRateWidget->SetLabel(QString("Quality"));
    _sampleRateWidget->SetIntType(true);
    _sampleRateWidget->SetExtents(MIN_QUALITY, MAX_QUALITY);

    connect(_sampleRateWidget, SIGNAL(valueChanged(int)), this, SLOT(_qualityChanged(int)));

    _params = NULL;
}

void SliceAppearanceSubtab::_qualityChanged(int quality)
{
    int sampleRate = quality * SAMPLES_PER_QUALITY;
    _params->SetSampleRate(sampleRate);
}

void SliceAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _params = dynamic_cast<VAPoR::SliceParams *>(rParams);
    VAssert(_params);
    _tfe->Update(dataMgr, paramsMgr, rParams);

    std::vector<double> minExt, maxExt;
    _params->GetBox()->GetExtents(minExt, maxExt);

    int sampleRate = _params->GetSampleRate();
    int quality = sampleRate / SAMPLES_PER_QUALITY;
    _sampleRateWidget->SetValue(quality);
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
    VAssert(_params);

    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    _copyRegionWidget->Update(paramsMgr, rParams);
    _transformTable->Update(rParams->GetTransform());
}

void SliceGeometrySubtab::_orientationChanged(int plane) { _params->GetBox()->SetOrientation(plane); }

SliceAnnotationSubtab::SliceAnnotationSubtab(QWidget *parent) { setupUi(this); }

void SliceAnnotationSubtab::Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
