#ifndef TFWIDGET_H
#define TFWIDGET_H

#include "ui_TFWidgetGUI.h"
#include "EventRouter.h"
#include "RangeCombos.h"
#include "Flags.h"

namespace VAPoR {
class ControlExec;
class MapperFunction;
}    // namespace VAPoR

//    V - Composition
//    # - Association
//
//     |----------------|              |--------------|
//     |    QWidget::   |     Update() | DataMgr      |
//  |--|    TFWidget    |--------------#              |
//  |  |----------------|   |          |--------------|
//  |         |             |
//  |         | 1           |
//  |  |------V---------|   |          |--------------|
//  |  |  QGLWidget::   |   | Update() | ParamsMgr    |
//  |  |  MappingFrame  |---|----------#              |
//  |  |----------------|   |          |--------------|
//  |         |             |
//  |         | 1           |
//  |  |------V---------|   |          |--------------|
//  |--# ParamsBase::   |   | Update() | RenderParams |
//     | MapperFunction |   |----------#              |
//     |------^---------|              |--------------|
//            |                              |
//            |------------------------------|
//

class TFWidget : public QWidget, public Ui_TFWidgetGUI {
    Q_OBJECT

public:
    TFWidget(QWidget *parent = 0);

    void Reinit(TFFlags flags);

    ~TFWidget();

    QString name() const { return "TFWidget"; }
    QString includeFile() const { return "TFWidget.h"; }
    QString group() const { return tr("Transfer Function Settings Widgets"); }
    QString toolTip() const { return tr("A Transfer Function Settings Widget"); }
    QString whatsThis() const
    {
        return tr("This widget contains all widgets "
                  "necessary for making changes to a "
                  "Vapor Transfer Function.");
    }
    bool isContainer() const { return true; }
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

    void fileLoadTF(string varname, const char *path, bool savePath);

    void loadTF(string varname);

    void  getRange(float range[2], float values[2]);
    float getOpacity();
    void  RefreshHistogram();
    void  SetAutoUpdateParamChanged(bool changed);

private slots:
    void fileSaveTF();
    void setRange();
    void setRange(double min, double max);
    void updateHisto();
    void autoUpdateHistoChecked(int state);
    void colorInterpChanged(int index);
    void loadTF();
    void emitTFChange();
    void opacitySliderChanged(int value);
    void setSingleColor();
    void setUsingSingleColor(int checkState);
    void setUseWhitespace(int state);

private:
    void                   collapseConstColorWidgets();
    void                   showConstColorWidgets();
    void                   showWhitespaceFrame();
    void                   hideWhitespaceFrame();
    string                 getVariableName();
    void                   connectWidgets();
    void                   updateSliders();
    void                   updateAutoUpdateHistoCheckbox();
    void                   updateColorInterpolation();
    void                   updateMappingFrame();
    void                   enableTFWidget(bool state);
    void                   updateConstColorWidgets();
    void                   checkForExternalChangesToHisto();
    bool                   autoUpdateHisto();
    string                 getCurrentVarName();
    VAPoR::MapperFunction *getCurrentMapperFunction();

    int confirmMinRangeEdit(VAPoR::MapperFunction *tf, float *range);
    int confirmMaxRangeEdit(VAPoR::MapperFunction *tf, float *range);

    std::vector<double> _minExt;
    std::vector<double> _maxExt;
    int                 _cLevel;
    int                 _refLevel;
    int                 _timeStep;
    string              _varName;
    bool                _autoUpdateParamChanged;

    bool  _autoUpdateHisto;
    bool  _discreteColormap;
    bool  _textChanged;
    float _myRGB[3];
    float _savedMapperValues[2];

    RenderEventRouter *  _eventRouter;
    VAPoR::ParamsMgr *   _paramsMgr;
    VAPoR::DataMgr *     _dataMgr;
    VAPoR::RenderParams *_rParams;

    Combo *     _minCombo;
    Combo *     _maxCombo;
    RangeCombo *_rangeCombo;

    TFFlags _flags;

    static string _nDimsTag;

#ifdef VAPOR3_0_0_ALPHA
    void makeItRed(QLineEdit *edit);
    void makeItYellow(QLineEdit *edit);
    void makeItGreen(QLineEdit *edit);
    void makeItWhite(QLineEdit *edit);
#endif

signals:
    void emitChange();
};

#endif    // TFWIDGET_H
