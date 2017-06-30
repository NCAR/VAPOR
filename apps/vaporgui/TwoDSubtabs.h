#ifndef TWODSUBTABS_H
#define TWODSUBTABS_H

#include "TwoDAppearanceGUI.h"
#include "TwoDVariablesGUI.h"
#include "TwoDGeometryGUI.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class TwoDVariablesSubtab : public QWidget, public Ui_TwoDVariablesGUI {

	Q_OBJECT

public:
	TwoDVariablesSubtab(QWidget* parent) {
		setupUi(this);
		_variablesWidget->Reinit((VariablesWidget::DisplayFlags)
			(VariablesWidget::SCALAR | VariablesWidget::HGT),
			VariablesWidget::TWOD);
	}

	void Update(
		const VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		_variablesWidget->Update(dataMgr, paramsMgr, rParams);
	}
};

class TwoDAppearanceSubtab : public QWidget, public Ui_TwoDAppearanceGUI {

	Q_OBJECT

public:
	TwoDAppearanceSubtab(QWidget* parent) {
		setupUi(this);
		_TFWidget->setEventRouter(dynamic_cast<RenderEventRouter*>(parent));
	}

	void Update(
		VAPoR::DataStatus *dataStatus,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		_TFWidget->Update(dataStatus, paramsMgr, rParams);

		VAPoR::DataMgr *dataMgr = dataStatus->GetActiveDataMgr();
		_ColorbarWidget->Update(dataMgr, paramsMgr, rParams);
	}
};

class TwoDGeometrySubtab : public QWidget, public Ui_TwoDGeometryGUI {

	Q_OBJECT

public:
	TwoDGeometrySubtab(QWidget* parent) {
		setupUi(this);
		_geometryWidget->Reinit(
			GeometryWidget::TWOD);
	}
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	) {
		_geometryWidget->Update(paramsMgr, dataMgr, rParams);
	}


private:

};

#endif //TWODSUBTABS_H
