#ifndef FLOWSUBTABS_H
#define FLOWSUBTABS_H

#include "Flags.h"

#include <QWidget>
#include <QVBoxLayout>
#include <vapor/FlowParams.h>

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class VariablesWidget;
class TFWidget;
class GeometryWidget;
class CopyRegionWidget;
class TransformTable;
class ColorbarWidget;
//class VPushButton;
//class VComboBox;
//class VCheckBox;

class QVaporSubtab : public QWidget {
    Q_OBJECT

public:
    QVaporSubtab(QWidget* parent);

protected:
    QVBoxLayout* _layout;
};

class FlowVariablesSubtab : public QVaporSubtab {

	Q_OBJECT

public:
	FlowVariablesSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

private:
    VAPoR::FlowParams*      _params;
    VariablesWidget* _variablesWidget;
};

class FlowAppearanceSubtab : public QVaporSubtab {

	Q_OBJECT

public:
	FlowAppearanceSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

private:
    VAPoR::FlowParams* _params;
    TFWidget*   _TFWidget;
};

class FlowSeedingSubtab : public QVaporSubtab {

	Q_OBJECT

public:
	FlowSeedingSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

protected slots:
    void _pushTestPressed();
    void _comboBoxSelected( int index );
    void _checkBoxSelected();

private:
    VAPoR::FlowParams*      _params;
    GeometryWidget*         _geometryWidget;
    VPushButton*            _pushTest;
    VComboBox*              _comboTest;
    VCheckBox*              _checkboxTest;
};

class FlowGeometrySubtab : public QVaporSubtab {

	Q_OBJECT

public:
	FlowGeometrySubtab(QWidget* parent);
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	); 

private:
    VAPoR::FlowParams*     _params;
    GeometryWidget*         _geometryWidget;
    CopyRegionWidget*       _copyRegionWidget;
    TransformTable*         _transformTable;
};

class FlowAnnotationSubtab : public QVaporSubtab {

	Q_OBJECT

public:
	FlowAnnotationSubtab(QWidget* parent);
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	);

private:
    ColorbarWidget* _colorbarWidget;
};

#endif //FLOWSUBTABS_H
