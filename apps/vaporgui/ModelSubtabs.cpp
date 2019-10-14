#include "ModelSubtabs.h"
#include "TFEditor.h"
#include "VSection.h"
#include "ParamsWidgets.h"



ModelVariablesSubtab::ModelVariablesSubtab(QWidget* parent) {
    setupUi(this);
    _variablesWidget->hide();
    
    layout()->setContentsMargins(0, 10, 0, 0);
    layout()->addWidget(_modelSettings = new VSection("Model"));
    
    addPW(new ParamsWidgetFile("file"));
}

void ModelVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto w : _pw)
        w->Update(rParams);
}

void ModelVariablesSubtab::addPW(ParamsWidget *w)
{
    _modelSettings->layout()->addWidget(w);
    _pw.push_back(w);
}







ModelAppearanceSubtab::ModelAppearanceSubtab(QWidget* parent) {
	setupUi(this);
}

void ModelAppearanceSubtab::Update(
	VAPoR::DataMgr *dataMgr,
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::RenderParams *rParams
) {
	_cParams = (VAPoR::ModelParams*)rParams;
	_dataMgr = dataMgr;
	_paramsMgr = paramsMgr;
}

ModelGeometrySubtab::ModelGeometrySubtab(QWidget* parent) {
	setupUi(this);
	_geometryWidget->Reinit(
		(DimFlags)THREED,
		(VariableFlags)SCALAR
	);

	_orientationAngles->hide();
}
