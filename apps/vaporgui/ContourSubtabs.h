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

class ContourVariablesSubtab : public QWidget, public Ui_ContourVariablesGUI {

	Q_OBJECT

public:
	ContourVariablesSubtab(QWidget* parent) {
		setupUi(this);
		_variablesWidget->Reinit((VariablesWidget::DisplayFlags)
			(VariablesWidget::SCALAR | VariablesWidget::HGT),
			(VariablesWidget::DimFlags)
			//(VariablesWidget::THREED | VariablesWidget::TWOD));
			(VariablesWidget::TWOD));
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
	void SetIsovalues(); 
	double GetContourMinOrMax(string minOrMax);

	VAPoR::ContourParams* _cParams;
	VAPoR::DataMgr* _dataMgr;
	VAPoR::ParamsMgr* _paramsMgr;
	Combo* _lineWidthCombo;
	Combo* _countCombo;
	Combo* _cMinCombo;
	Combo* _spacingCombo;


private slots:
	void MappingChanged() {
		cout << "mapping changed!" << endl;
	}

	void EndTFChange();

	void SetLineThickness(double val) {
		_cParams->SetLineThickness(val);
	}

	void SetContourCount(int count);

	void SetContourMinimum(double min);

	void SetContourSpacing(double spacing);

	void LockToTFChecked(bool checked);

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
		_transformTable->Update(rParams->GetTransform());
	}


private:
};

#endif //CONTOURSUBTABS_H
