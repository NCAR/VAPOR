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
class VIntSpinBox;
class VGeometry2;
class VPushButton;

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

using VAPoR::FlowDir;
using VAPoR::FlowSeedMode;

class QVaporSubtab : public QWidget {
    Q_OBJECT

public:
    QVaporSubtab(QWidget *parent);

protected:
    QVBoxLayout *_layout;
};

//
//================================
//
class FlowVariablesSubtab : public QVaporSubtab {
    Q_OBJECT

public:
    VariablesWidget *_variablesWidget;

    FlowVariablesSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    VAPoR::FlowParams *_params;
};

//
//================================
//
class FlowAppearanceSubtab : public QVaporSubtab {
    Q_OBJECT

public:
    FlowAppearanceSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    VAPoR::FlowParams *_params;
    TFEditor *         _TFEditor;
};

//
//================================
//
class FlowSeedingSubtab : public QVaporSubtab {
    Q_OBJECT

public:
    FlowSeedingSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private slots:
    void _configureFlowType(const std::string &value);
    void _configureSeedType(const std::string &value);

    void _pathlineLengthChanged(int length);
    void _pathlineStartTimeChanged(int startTime);
    void _pathlineEndTimeChanged(int endTime);
    void _pathlineLifetimeChanged(int lifeTime);

    void _streamlineDirectionChanged(int index);
    void _streamlineSamplesChanged(int length);

    void _seedInjIntervalChanged(int interval);

    void _periodicClicked();
    void _velocityMultiplierChanged(const std::string &multiplier);

    void _rakeNumOfSeedsChanged();
    void _seedListFileChanged(const std::string &file);
    void _biasVariableChanged(const std::string &variable);
    void _biasStrengthChanged(double bias);

    void _rakeGeometryChanged(const std::vector<float> &range);

    void _geometryWriterClicked();

    void _selectedTabChanged(int index);

private:
    void _createIntegrationSection();
    void _createSeedingSection(QWidget *parent);
    void _updateStreamlineWidgets(VAPoR::DataMgr *dataMgr);
    void _updatePathlineWidgets(VAPoR::DataMgr *dataMgr);

    VAPoR::FlowParams *_params;
    VAPoR::ParamsMgr * _paramsMgr;

    // Integration options
    VSection * _integrationSection;
    VComboBox *_flowTypeCombo;

    //  Streamline integration options
    VFrame *     _streamlineFrame;
    VSliderEdit *_streamlineSamplesSliderEdit;
    VComboBox *  _streamlineDirectionCombo;

    //  Pathline integration options
    VFrame *     _pathlineFrame;
    VSliderEdit *_pathlineLengthSliderEdit;
    /*VSliderEdit*            _pathlineInjIntervalSliderEdit;
    VSliderEdit*            _pathlineStartSliderEdit;
    VSliderEdit*            _pathlineEndSliderEdit;
    VSliderEdit*            _pathlineLifetimeSliderEdit;*/

    //  Universal integration options
    VCheckBox *_periodicXCheckBox;
    VCheckBox *_periodicYCheckBox;
    VCheckBox *_periodicZCheckBox;
    VLineEdit *_velocityMultiplierLineEdit;

    // Seed distribution options
    VSection * _seedDistributionSection;
    VComboBox *_seedTypeCombo;

    //  Gridded seed distribution
    VFrame *     _griddedSeedsFrame;
    VIntSpinBox *_xSeedSpinBox;
    VIntSpinBox *_ySeedSpinBox;
    VIntSpinBox *_zSeedSpinBox;
    VSliderEdit *_xSeedSliderEdit;
    VSliderEdit *_ySeedSliderEdit;
    VSliderEdit *_zSeedSliderEdit;

    //  Rake region selection
    VSection *  _rakeRegionSection;
    VGeometry2 *_rakeWidget;

    //  Seeds read from a text file
    VFrame *     _listOfSeedsFrame;
    VFileReader *_listOfSeedsFileReader;

    //  Random seed distribution
    VFrame *     _randomSeedsFrame;
    VSliderEdit *_randomSeedsSliderEdit;
    VComboBox *  _biasVariableComboBox;
    VSliderEdit *_biasWeightSliderEdit;

    VFileWriter *_geometryWriterSelector;
    VPushButton *_geometryWriterExecutor;
    VSection *   _geometryWriterSection;
};

//
//================================
//
class FlowGeometrySubtab : public QVaporSubtab {
    Q_OBJECT

public:
    FlowGeometrySubtab(QWidget *parent);

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams);

private:
    VAPoR::FlowParams *_params;
    GeometryWidget *   _geometryWidget;
    CopyRegionWidget * _copyRegionWidget;
    TransformTable *   _transformTable;
};

//
//================================
//
class FlowAnnotationSubtab : public QVaporSubtab {
    Q_OBJECT

public:
    FlowAnnotationSubtab(QWidget *parent);

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams);

private:
    ColorbarWidget *_colorbarWidget;
};

#endif    // FLOWSUBTABS_H
