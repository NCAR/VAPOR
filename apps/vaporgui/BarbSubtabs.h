#ifndef BARBSUBTABS_H
#define BARBSUBTABS_H

#include "ui_BarbAppearanceGUI.h"
#include "ui_BarbVariablesGUI.h"
#include "ui_BarbGeometryGUI.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
	class BarbParams;
}

class BarbVariablesSubtab : public QWidget, public Ui_BarbVariablesGUI {

	Q_OBJECT

public:
	BarbVariablesSubtab(QWidget* parent) {
		setupUi(this);
		_variablesWidget->Reinit((VariablesWidget::DisplayFlags)
			(VariablesWidget::VECTOR | VariablesWidget::HGT |
			VariablesWidget::COLOR),
			(VariablesWidget::DimFlags)(VariablesWidget::TWOD),
			(VariablesWidget::ColorFlags)
			(VariablesWidget::COLORVAR));
			//(VariablesWidget::DimFlags)(VariablesWidget::THREED));
	}

	void Initialize(VAPoR::BarbParams* bParams, VAPoR::DataMgr* dataMgr);
	void pushVarStartingWithLetter(vector<string> searchVars, 
		vector<string> &returnVars, char letter);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	) {
		_variablesWidget->Update(dataMgr, paramsMgr, rParams);
	}
};

class BarbAppearanceSubtab : public QWidget, public Ui_BarbAppearanceGUI {

	Q_OBJECT

public:
	BarbAppearanceSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

private slots:
	void xDimChanged(int i);
	void yDimChanged(int i);
	void zDimChanged(int i);
	void lengthChanged(double d);
	void thicknessChanged(double d);
	double CalculateDomainLength(int ts);

private:
	void hideZDimWidgets();

	VAPoR::BarbParams* _bParams;
	VAPoR::DataMgr* _dataMgr;
	VAPoR::ParamsMgr* _paramsMgr;
	Combo* _xDimCombo;
	Combo* _yDimCombo;
	Combo* _zDimCombo;
	Combo* _lengthCombo;
	Combo* _thicknessCombo;
};

class BarbGeometrySubtab : public QWidget, public Ui_BarbGeometryGUI {

	Q_OBJECT

public:
	BarbGeometrySubtab(QWidget* parent);
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	) {
		_bParams = (VAPoR::BarbParams*)rParams;
		_geometryWidget->Update(paramsMgr, dataMgr, rParams);
		_transformTable->Update(rParams->GetTransform());
	}

private:
	VAPoR::BarbParams* _bParams;
};

#endif //BARBSUBTABS_H
