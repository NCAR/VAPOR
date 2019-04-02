#ifndef FLOWSUBTABS_H
#define FLOWSUBTABS_H

#include "Flags.h"

#include <QWidget>
#include <QVBoxLayout>
#include <vapor/FlowParams.h>

#include <QLineEdit>

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
class VFileReader;
class VCheckBox;

class QVaporSubtab : public QWidget {
    Q_OBJECT

public:
    QVaporSubtab(QWidget* parent);

protected:
    QVBoxLayout* _layout;
};

//
//================================
//
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
    VAPoR::FlowParams*  _params;
    VariablesWidget*    _variablesWidget;

    VCheckBox*          _steady;

    // Sam's attempt to add more widgets
    //   TODO: add validator/mask so that only numerical input 
    //   between 0.001 and 1000 are valid.
    QLineEdit*          _xMultiplier;
    QLineEdit*          _yMultiplier;
    QLineEdit*          _zMultiplier;
    

private slots:
    // Respond to user input
    void _steadyGotClicked();

    void _velocityMultiplierChanged();
};

//
//================================
//
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

//
//================================
//
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
    VFileReader*            _fileReader;
};

//
//================================
//
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

//
//================================
//
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
