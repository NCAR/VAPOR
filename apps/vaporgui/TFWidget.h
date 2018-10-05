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

    void  getVariableRange(float range[2], float values[2], bool colorVar);
    float getOpacity();

private slots:
    void fileSaveTF();
    void setRange();
    void setRange(double min, double max);
    void updateHisto();
    void refreshHistograms();
    void autoUpdateHistoChecked(int state);
    void colorInterpChanged(int index);
    void loadTF();
    void emitTFChange();
    void opacitySliderChanged(int value);
    void setSingleColor();
    void setUsingSingleColor(int checkState);
    void setUseWhitespace(int state);
    void setColorMapMinRange(double min);
    void setColorMapMaxRange(double max);

private:
    void   collapseConstColorWidgets();
    void   showConstColorWidgets();
    void   showWhitespaceFrame();
    void   hideWhitespaceFrame();
    void   connectWidgets();
    string getCurrentVarName();

    void updateMainTransferFunction();
    void updateColorTransferFunction();
    void updateSliders();
    void updateAutoUpdateHistoCheckbox();
    void updateColorInterpolation();
    void updateMappingFrames();
    void updateColorMapMappingFrame();
    void updateConstColorWidgets();
    void enableTFWidget(bool state);

    void checkForExternalChangesToHisto();
    void checkForBoxChanges();
    void checkForCompressionChanges();
    void checkForMapperRangeChanges();
    void checkForTimestepChanges();

    bool                   autoUpdateHisto();
    VAPoR::MapperFunction *getMainMapperFunction();
    VAPoR::MapperFunction *getColorMapMapperFunction();
    VAPoR::MapperFunction *getColorMapperFunction();

    int confirmMinRangeEdit(VAPoR::MapperFunction *tf, float *range);
    int confirmMaxRangeEdit(VAPoR::MapperFunction *tf, float *range);

    std::vector<double> _minExt;
    std::vector<double> _maxExt;
    int                 _cLevel;
    int                 _refLevel;
    int                 _timeStep;
    string              _varName;
    bool                _somethingChanged;

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
