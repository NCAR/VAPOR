#ifndef FLOWSUBTABS_H
#define FLOWSUBTABS_H

#include "Flags.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <vapor/FlowParams.h>

#include <QLineEdit>

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class VariablesWidget;
class TFEditor;
class GeometryWidget;
class CopyRegionWidget;
class TransformTable;
class ColorbarWidget;
class VFileReader;
class VFileWriter;
class VPushButton;
class VCheckBox;
class VComboBox;
class VLineEdit;
class VSpinBox;
class VTabWidget;
class QSliderEdit;

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

/*
    // Sam's attempt to add more widgets
    //   TODO: add validator/mask so that only numerical input 
    //   between 0.001 and 1000 are valid.
    QLineEdit*          _velocityMltp;

    VCheckBox*          _steady;

    // Sam's attempt to add more widgets
    //   TODO: add validator/mask so that only positive integers are accepted
    QLineEdit*          _steadyNumOfSteps;

    VCheckBox*          _periodicX;
    VCheckBox*          _periodicY;
    VCheckBox*          _periodicZ;
    
private slots:
    // Respond to user input
    void _steadyGotClicked();
    void _velocityMultiplierChanged();
    void _steadyNumOfStepsChanged();
    void _periodicClicked();
*/
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
   
    VTabWidget*  _streamlineAppearanceTab; 
    VComboBox*   _shapeCombo;
    VComboBox*   _colorCombo;
    VSpinBox*    _lengthSpinBox;
    QSliderEdit* _smoothnessSliderEdit;
    VSpinBox*    _sizeSpinBox;

    TFEditor*   _TFEditor;
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
    void _seedInputFileChanged();
    void _configureRakeType();
//    void _seedGenModeChanged( int newIdx );
    void _exportGeometryPathChanged();
    void _seedpointReaderPathChanged();
    void _exportButtonClicked();

private:
    VAPoR::DataMgr*         _dataMgr;
    VAPoR::ParamsMgr*       _paramsMgr;
    VAPoR::FlowParams*      _params;

    GeometryWidget*         _geometryWidget;

    VTabWidget*             _seedSettingsTab;
    VComboBox*              _distributionCombo;
    VSpinBox*               _randomCountSpinBox;
    VComboBox*              _biasVariableCombo;
    QSliderEdit*            _biasSliderEdit;
    VSpinBox*               _xDistributionSpinBox;
    VSpinBox*               _yDistributionSpinBox;
    VSpinBox*               _zDistributionSpinBox;
    VFileReader*            _seedpointFileReader;
    VFileWriter*            _exportGeometryWriter;
    VPushButton*            _exportGeometryButton;    
    QPushButton*            _outputButton;
};

//
//================================
//
class FlowIntegrationSubtab: public QVaporSubtab {

	Q_OBJECT

public:
	FlowIntegrationSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

protected slots:
    void _configureIntegrationType();

private slots:
    void _multiplierChanged();
    void _integrationDirectionChanged();
    void _integrationLengthChanged();
    void _periodicityChanged();

private:
    void _initialize();

    VAPoR::DataMgr*         _dataMgr;
    VAPoR::ParamsMgr*       _paramsMgr;
    VAPoR::FlowParams*      _params;
    VTabWidget*             _integrationSettingsTab;
    VLineEdit*              _integrationLengthEdit;
    VComboBox*              _integrationTypeCombo;
    VComboBox*              _directionCombo;
    VCheckBox*              _periodicBoundaryComboX;
    VCheckBox*              _periodicBoundaryComboY;
    VCheckBox*              _periodicBoundaryComboZ;
    VLineEdit*              _multiplierLineEdit;
    VSpinBox*               _startSpinBox;
    VSpinBox*               _endSpinBox;
    VSpinBox*               _lifespanSpinBox;
    VSpinBox*               _intervalSpinBox;

    bool                    _initialized;
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
