#include <QButtonGroup>
#include "SliceSubtabs.h"
#include "TFEditor.h"
#include "VLineItem.h"
#include "PGroup.h"
#include "PSection.h"
#include "PVariableWidgets.h"
#include "PFidelitySection.h"

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
    setLayout(new QVBoxLayout);
    ((QVBoxLayout *)layout())->insertWidget(1, _pg = new PGroup);
    PSection *vars = new PSection("Variable Selection");
    vars->Add(new PScalarVariableSelectorHLI);
    _pg->Add(vars);
    _pg->Add(new PFidelitySection);
}

void SliceVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _pg->Update(rParams, paramsMgr, dataMgr); }

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
