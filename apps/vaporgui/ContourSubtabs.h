#ifndef CONTOURSUBTABS_H
#define CONTOURSUBTABS_H

#include "vapor/ContourParams.h"
#include "ui_ContourAppearanceGUI.h"
#include "ui_ContourVariablesGUI.h"
#include "ui_ContourGeometryGUI.h"
#include "ui_ContourAnnotationGUI.h"
#include "RangeCombos.h"
#include "Flags.h"
#include "PGroup.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class SpacingCombo;
class TFEditor;

class ContourVariablesSubtab : public QWidget, public Ui_ContourVariablesGUI {

	Q_OBJECT
    PGroup *pg;

public:
	ContourVariablesSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);
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
	Combo* _countCombo;
	Combo* _cMinCombo;
	Combo* _spacingCombo;
    TFEditor *_tfEditor;

private slots:
	void SetContourValues(int numContours, double contourMin, double spacing); 
	void EndTFChange();
	void SetLineThickness(double val) {_cParams->SetLineThickness(val);}
	void SetContourCount(int count);
	void SetContourMinimum(double min);
	void SetContourSpacing(double spacing);
};

class ContourGeometrySubtab : public QWidget, public Ui_ContourGeometryGUI {

	Q_OBJECT

public:
	ContourGeometrySubtab(QWidget* parent);
	
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

class ContourAnnotationSubtab : public QWidget, public Ui_ContourAnnotationGUI {

	Q_OBJECT

public:
	ContourAnnotationSubtab(QWidget* parent) {
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

#endif //CONTOURSUBTABS_H
