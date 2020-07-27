#pragma once

#include "ui_VolumeAppearanceGUI.h"
#include "ui_VolumeGeometryGUI.h"
#include "ui_VolumeAnnotationGUI.h"
#include "Flags.h"
#include <vapor/MapperFunction.h>
#include <vapor/VolumeParams.h>
#include "PVariableWidgets.h"
#include "PFidelitySection.h"
#include "PGroup.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class TFEditorVolume;
class QSliderEdit;
class PGroup;

class VolumeAppearanceSubtab : public QWidget, public Ui_VolumeAppearanceGUI {
	Q_OBJECT

public:
    VolumeAppearanceSubtab(QWidget* parent);
	void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
private:
    PGroup *_pg;
};

class VolumeGeometrySubtab : public QWidget, public Ui_VolumeGeometryGUI {

	Q_OBJECT

public:
	VolumeGeometrySubtab(QWidget* parent) {
		setupUi(this);
		_geometryWidget->Reinit(
			(DimFlags)THREED,
			(VariableFlags)SCALAR
		);
	}
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	) {
		_geometryWidget->Update(paramsMgr, dataMgr, rParams);
		_copyRegionWidget->Update(paramsMgr, rParams);
		_transformTable->Update(rParams->GetTransform());
	}
};

class VolumeAnnotationSubtab : public QWidget, public Ui_VolumeAnnotationGUI {

	Q_OBJECT

public:
	VolumeAnnotationSubtab(QWidget* parent) {
		setupUi(this);
	}
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	) {
		_colorbarWidget->Update(dataMgr, paramsMgr, rParams);
	}
};
