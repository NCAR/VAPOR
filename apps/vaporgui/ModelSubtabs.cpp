#include "ModelSubtabs.h"
#include "TFEditor.h"

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
