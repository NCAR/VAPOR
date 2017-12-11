#ifndef CONTOURSUBTABS_H
#define CONTOURSUBTABS_H

#include "vapor/ContourParams.h"
#include "ui_ContourAppearanceGUI.h"
#include "ui_ContourVariablesGUI.h"
#include "ui_ContourGeometryGUI.h"
#include "RangeCombos.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class SpacingCombo;

class ContourVariablesSubtab : public QWidget, public Ui_ContourVariablesGUI {

	Q_OBJECT

public:
	ContourVariablesSubtab(QWidget* parent) {
		setupUi(this);
		_variablesWidget->Reinit((VariablesWidget::DisplayFlags)
			(VariablesWidget::SCALAR | VariablesWidget::HGT),
			(VariablesWidget::DimFlags)
			(VariablesWidget::TWOD),
			(VariablesWidget::ColorFlags)(0));
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
	ContourAppearanceSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

	void Initialize(VAPoR::ContourParams* cParams);

private:
	void GetContourBounds(double& min, double& max);
	void enableSpacingWidgets();	
	void disableSpacingWidgets();

	VAPoR::ContourParams* _cParams;
	VAPoR::DataMgr* _dataMgr;
	VAPoR::ParamsMgr* _paramsMgr;
	Combo* _lineWidthCombo;
	Combo* _countCombo;
	Combo* _cMinCombo;
	//Combo* _spacingCombo;
	SpacingCombo* _spacingCombo;

	int _numContours;
	double _spacing;
	double _contourMin;
	double _contourMax;
	double _lineWidth;

private slots:
	void SetContourValues(); 

	void EndTFChange();

	void SetLineThickness(double val) {
		_cParams->SetLineThickness(val);
	}

	void SetNumContours(int count);

	void SetContourMinimum(double min);

	void SetContourSpacing(double spacing);

	void ContourBoundsChanged(int index);
};

class ContourGeometrySubtab : public QWidget, public Ui_ContourGeometryGUI {

	Q_OBJECT

public:
	ContourGeometrySubtab(QWidget* parent) {
		setupUi(this);
		_geometryWidget->Reinit(
			GeometryWidget::TWOD);

		_orientationAngles->hide();
	}
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	) {
		_geometryWidget->Update(paramsMgr, dataMgr, rParams);
		_transformTable->Update(rParams->GetTransform());
	}


private:
};

// Wish I could make SpacingCombo nested within ContourAppearanceSubtab,
// however Qt does not support MetaObject features with nested classes :\
class SpacingCombo : public Combo {

class SpacingCombo : public Combo {
	Q_OBJECT

	public:
	 void Update(double min, double max, double value);

	 SpacingCombo(
		QLineEdit* edit, 
		QSlider* slider, 
		bool intType = false) : Combo(edit, slider, intType) {
			_lineEdit->disconnect();
			connect(_lineEdit, SIGNAL(returnPressed()),
				//this, SLOT(SpacingCombo::setLineEdit()));
				this, SLOT(setLineEdit()));
			// break Combo's setLineEdit connection
			// connect returnPressed with overridden setLineEdit function
		}
	private slots: 
		void setLineEdit();
};
#endif //CONTOURSUBTABS_H
