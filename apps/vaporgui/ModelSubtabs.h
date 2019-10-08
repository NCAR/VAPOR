#pragma once

#include "vapor/ModelParams.h"
#include "ui_ModelAppearanceGUI.h"
#include "ui_ModelVariablesGUI.h"
#include "ui_ModelGeometryGUI.h"
#include "ui_ModelAnnotationGUI.h"
#include "RangeCombos.h"
#include "Flags.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class SpacingCombo;
class TFEditor;
class VSection;
class ParamsWidget;

class ModelVariablesSubtab : public QWidget, public Ui_ModelVariablesGUI {

	Q_OBJECT

public:
    ModelVariablesSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
                );
    
private:
    VSection *_modelSettings;
    vector<ParamsWidget *> _pw;
    
    void addPW(ParamsWidget *w);
};

class ModelAppearanceSubtab : public QWidget, public Ui_ModelAppearanceGUI {

	Q_OBJECT

public:
	ModelAppearanceSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

	VAPoR::ModelParams* _cParams;
	VAPoR::DataMgr* _dataMgr;
	VAPoR::ParamsMgr* _paramsMgr;

private slots:
};

class ModelGeometrySubtab : public QWidget, public Ui_ModelGeometryGUI {

	Q_OBJECT

public:
	ModelGeometrySubtab(QWidget* parent);
	
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

class ModelAnnotationSubtab : public QWidget, public Ui_ModelAnnotationGUI {

	Q_OBJECT

public:
	ModelAnnotationSubtab(QWidget* parent) {
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
