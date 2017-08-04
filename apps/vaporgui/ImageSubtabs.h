#ifndef IMAGESUBTABS_H
#define IMAGESUBTABS_H

#include "ImageAppearanceGUI.h"
#include "ImageVariablesGUI.h"
#include "ImageGeometryGUI.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class ImageVariablesSubtab : public QWidget, public Ui_ImageVariablesGUI {

	Q_OBJECT

public:
	ImageVariablesSubtab(QWidget* parent) {
		setupUi(this);
		_variablesWidget->Reinit( VariablesWidget::HGT, VariablesWidget::TWOD );
	}

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		_variablesWidget->Update(dataMgr, paramsMgr, rParams);
	}
};

class ImageAppearanceSubtab : public QWidget, public Ui_ImageAppearanceGUI {

	Q_OBJECT

public:
	ImageAppearanceSubtab(QWidget* parent) {
		setupUi(this);
		//_TFWidget->Reinit((TFWidget::Flags)(0));
	}

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		//_TFWidget->Update(dataMgr, paramsMgr, rParams);
		//_ColorbarWidget->Update(dataMgr, paramsMgr, rParams);
	}
};

class ImageGeometrySubtab : public QWidget, public Ui_ImageGeometryGUI {

	Q_OBJECT

public:
	ImageGeometrySubtab(QWidget* parent) {
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

#endif 
