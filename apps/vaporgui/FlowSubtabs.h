#ifndef FLOWSUBTABS_H
#define FLOWSUBTABS_H

#include "Flags.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <vapor/FlowParams.h>

#include <QLineEdit>

#include "VariablesWidget.h"
#include "TFWidget.h"
#include "GeometryWidget.h"
#include "CopyRegionWidget.h"
#include "TransformTable.h"
#include "ColorbarWidget.h"
#include "VaporWidgets.h"

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

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

    // Sam's attempt to add more widgets
    //   TODO: add validator/mask so that only numerical input 
    //   between 0.001 and 1000 are valid.
    QLineEdit*          _velocityMltp;

    VCheckBox*          _steady;

    // Sam's attempt to add more widgets
    //   TODO: add validator/mask so that only positive integers are accepted
    QLineEdit*          _steadyNumOfSteps;
    

private slots:
    // Respond to user input
    void _steadyGotClicked();
    void _velocityMultiplierChanged();
    void _steadyNumOfStepsChanged();
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

private slots:
    // Respond to user input
    void _seedGenModeChanged( int newIdx );
    void _fileReaderChanged();
    void _fileWriterChanged();
    void _flowDirectionChanged( int newIdx );
    void _outputButtonClicked();

private:
    VAPoR::FlowParams*      _params;

    GeometryWidget*         _geometryWidget;

    VComboBox*              _seedGenMode;
    VFileReader*            _fileReader;
    VFileWriter*            _fileWriter;
    QPushButton*            _outputButton;

    VComboBox*              _flowDirection;
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
