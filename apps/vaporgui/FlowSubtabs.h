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
#include "VaporWidgets.h"

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
    FlowVariablesSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    VAPoR::FlowParams *_params;
    VariablesWidget *  _variablesWidget;

    VLineEdit *_velocityMltp;    // Note on this widget: its name and associated functions
                                 // use the name "velocity multiplier," while it displays
                                 // "Field Scale Factor." They'll need to be reconciled
                                 // before the final merge.

    VCheckBox *_periodicX;
    VCheckBox *_periodicY;
    VCheckBox *_periodicZ;

private slots:
    // Respond to user input
    void _velocityMultiplierChanged();
    void _periodicClicked();
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
    /* Respond to user input */
    void _seedGenModeChanged(int newIdx);
    void _fileReaderChanged();
    void _fileWriterChanged();
    void _flowDirectionChanged(int newIdx);

    void _rakeGeometryChanged();
    void _rakeNumOfSeedsChanged();
    void _rakeBiasVariableChanged(int);
    void _rakeBiasStrengthChanged();

    void _steadyGotClicked();
    void _steadyNumOfStepsChanged();
    void _pastNumOfTimeStepsChanged(int);
    void _seedInjIntervalChanged(int);

private:
    VAPoR::FlowParams *_params;

    /* Add some QT widgets */
    VCheckBox * _steady;
    VLineEdit * _steadyNumOfSteps;
    VIntSlider *_pastNumOfTimeSteps;
    VIntSlider *_seedInjInterval;

    VComboBox *  _seedGenMode;
    VFileReader *_fileReader;
    VFileWriter *_fileWriter;
    VComboBox *  _flowDirection;

    /* Rake related widgets */
    VGeometry *_rake;
    VLineEdit *_rakeXNum, *_rakeYNum, *_rakeZNum, *_rakeTotalNum;
    VComboBox *_rakeBiasVariable;
    VSlider *  _rakeBiasStrength;

    QFrame *_hline1, *_hline2;    // horizontal lines

    /* Helper functions */
    void _hideShowWidgets();    // hide and show widgets based on the current seed generation mode.
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
