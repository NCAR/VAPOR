#ifndef CONTOURSUBTABS_H
#define CONTOURSUBTABS_H

#include "vapor/ContourParams.h"
#include "ContourAppearanceGUI.h"
#include "ContourVariablesGUI.h"
#include "ContourGeometryGUI.h"
#include "RangeCombos.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class ContourVariablesSubtab : public QWidget, public Ui_ContourVariablesGUI {

	Q_OBJECT

public:
	ContourVariablesSubtab(QWidget* parent) {
		setupUi(this);
		_variablesWidget->Reinit((VariablesWidget::DisplayFlags)
			(VariablesWidget::SCALAR | VariablesWidget::HGT),
			(VariablesWidget::DimFlags)
			(VariablesWidget::THREED | VariablesWidget::TWOD));
	}

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		_variablesWidget->Update(dataMgr, paramsMgr, rParams);
	}
};

class ContourAppearanceSubtab : public QWidget, public Ui_ContourAppearanceGUI {

	Q_OBJECT

public:
	ContourAppearanceSubtab(QWidget* parent) {
		setupUi(this);
		_TFWidget->Reinit((TFWidget::Flags)(0));
		_TFWidget->mappingFrame->setIsolineSliders(true);
		_TFWidget->mappingFrame->setOpacityMapping(false);

		_lineWidthCombo = new Combo(lineWidthEdit, lineWidthSlider);

		connect(_lineWidthCombo, SIGNAL(valueChanged(double)), this,
			SLOT(SetLineThickness(double)));
	}

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		_TFWidget->Update(dataMgr, paramsMgr, rParams);
		_ColorbarWidget->Update(dataMgr, paramsMgr, rParams);

		// Set up or stand-alone slider/edit combo
		//
		_cParams = (VAPoR::ContourParams*)rParams;
		GLfloat lineWidthRange[2] = {0.f, 0.f};
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
		_lineWidthCombo->Update(lineWidthRange[0], lineWidthRange[1], 
			_cParams->GetLineThickness()
		);
	}

private:
	VAPoR::ContourParams* _cParams;
	Combo* _lineWidthCombo;

private slots:
	void SetLineThickness(double val) {
		_cParams->SetLineThickness(val);
	}

};

class ContourGeometrySubtab : public QWidget, public Ui_ContourGeometryGUI {

	Q_OBJECT

public:
	ContourGeometrySubtab(QWidget* parent) {
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

#endif //CONTOURSUBTABS_H
