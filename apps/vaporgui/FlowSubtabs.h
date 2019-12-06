#ifndef FLOWSUBTABS_H
#define FLOWSUBTABS_H

#include "Flags.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <vapor/FlowParams.h>

#include <QLineEdit>

#include "TFEditor.h"
#include "VariablesWidget.h"
#include "GeometryWidget.h"
#include "CopyRegionWidget.h"
#include "TransformTable.h"
#include "ColorbarWidget.h"

class VLineEdit;
class VCheckBox;
class VComboBox;
class VSlider;
class VSliderEdit;
class VFileReader;
class VFileWriter;
class VGeometry;
class VFrame;
class VSpinBox;
class VGeometry2;

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

using VAPoR::FlowSeedMode;
using VAPoR::FlowDir;

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
    VariablesWidget*    _variablesWidget;
    
	FlowVariablesSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

private:
    VAPoR::FlowParams*  _params;
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
    TFEditor*   _TFEditor;
};

//
//================================
//
class FlowSeedingSubtab : public QVaporSubtab {

	Q_OBJECT

public:
	FlowSeedingSubtab(QWidget* parent);

	void Update ( VAPoR::DataMgr*         dataMgr,
		          VAPoR::ParamsMgr*       paramsMgr,
		          VAPoR::RenderParams*    rParams );

private slots:
    void _configureFlowType( const std::string& value );
    void _configureSeedType( const std::string& value );

    void _streamlineLengthChanged( int length );
    void _streamlineStartTimeChanged( int startTime );
    void _streamlineEndTimeChanged( int endTime );
    void _streamlineLifetimeChanged( int lifeTime );

    void _pathlineDirectionChanged( int index );
    void _pathlineSamplesChanged( int length );

    void _seedInjIntervalChanged( int interval );

    void _periodicClicked();
    void _velocityMultiplierChanged( const std::string& multiplier );

    void _seedGenModeChanged( int newIdx );
    void _rakeNumOfSeedsChanged();
    void _seedListFileChanged( const std::string& file );
    void _biasVariableChanged( const std::string& variable );
    void _biasStrengthChanged( double bias );

    void _rakeGeometryChanged( const std::vector<float>& range );

    void _geometryWriterFileChanged( const std::string& file );

    void _selectedTabChanged( int index );

private:
    void _createIntegrationSection();
    void _createSeedingSection( QWidget* parent );
    void _updateSteadyFlowWidgets( VAPoR::DataMgr* dataMgr);
    void _updateUnsteadyFlowWidgets( VAPoR::DataMgr* dataMgr);

    VAPoR::FlowParams*      _params;
    VAPoR::ParamsMgr *      _paramsMgr;

    // Integration options
    VSection*               _integrationSection;
    VComboBox*              _flowTypeCombo;

    //  Pathline integration options
    VFrame*                 _pathlineFrame;
    VSliderEdit*            _pathlineSamplesSliderEdit;
    VComboBox*              _pathlineDirectionCombo;

    //  Streamline integration options
    VFrame*                 _streamlineFrame;
    VSliderEdit*            _streamlineLengthSliderEdit;
    VSliderEdit*            _streamlineInjIntervalSliderEdit;
    VSliderEdit*            _streamlineStartSliderEdit;
    VSliderEdit*            _streamlineEndSliderEdit;
    VSliderEdit*            _streamlineLifetimeSliderEdit;

    //  Universal integration options
    VCheckBox*              _periodicXCheckBox;
    VCheckBox*              _periodicYCheckBox;
    VCheckBox*              _periodicZCheckBox;
    VLineEdit*              _velocityMultiplierLineEdit;
   
    // Seed distribution options
    VSection*               _seedDistributionSection;
    VComboBox*              _seedTypeCombo;

    //  Gridded seed distribution
    VFrame*                 _griddedSeedsFrame;
    VSpinBox*               _xSeedSpinBox;
    VSpinBox*               _ySeedSpinBox;
    VSpinBox*               _zSeedSpinBox;
    VSliderEdit*            _xSeedSliderEdit;
    VSliderEdit*            _ySeedSliderEdit;
    VSliderEdit*            _zSeedSliderEdit;

    //  Rake region selection
    VSection*               _rakeRegionSection;
    VGeometry2*             _rakeWidget;

    //  Seeds read from a text file
    VFrame*                 _listOfSeedsFrame;
    VFileReader*            _listOfSeedsFileReader;

    //  Random seed distribution 
    VFrame*                 _randomSeedsFrame;
    VSliderEdit*            _randomSeedsSliderEdit;
    VComboBox*              _biasVariableComboBox;
    VSliderEdit*            _biasWeightSliderEdit;

    VFileWriter*            _geometryWriter;
    VFrame*                 _geometryWriterFrame;
    VSection*               _geometryWriterSection;
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
    VAPoR::FlowParams*      _params;
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
