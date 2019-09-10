#ifndef TFWIDGET_H
#define TFWIDGET_H

#include <QFileDialog>
#include "ui_TFWidgetGUI.h"
#include "EventRouter.h"
#include "RangeCombos.h"
#include "Flags.h"

namespace VAPoR {
class ControlExec;
class MapperFunction;
}    // namespace VAPoR

namespace TFWidget_ {
class LoadTFDialog;
class CustomFileDialog;
}    // namespace TFWidget_

class VPushButtonWithDoubleClick;

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
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams, bool internalUpdate = false);

    void  getVariableRange(float range[2], float values[2], bool secondaryVariable);
    float getOpacity();
    void  RefreshHistogram();
    void  SetAutoUpdateParamChanged(bool changed);
    bool  IsOpacitySupported() const;
    void  SetOpacitySupported(bool value);
    bool  IsOpacityIntegrated() const;
    void  SetOpacityIntegrated(bool value);

private slots:
    void loadTF();
    void saveTF();
    bool selectedTFFileOk(string fileName);

    void autoUpdateMainHistoChecked(int state);
    void autoUpdateSecondaryHistoChecked(int state);

    void setColorInterpolation(int index);
    void emitTFChange();
    void opacitySliderChanged(int value);
    void opacitySliderReleased();

    void setSingleColor();
    void setUsingSingleColor(int checkState);
    void setUseWhitespace(int state);

    void setRange();
    void setRange(double min, double max);
    void setSecondaryRange();
    void setSecondaryMinRange(double min);
    void setSecondaryMaxRange(double max);

    void updateMainMappingFrame();
    void updateSecondaryMappingFrame();

private:
    TFWidget_::LoadTFDialog *_loadTFDialog;

    void refreshMainDuplicateHistogram();
    void refreshSecondaryDuplicateHistogram();

    void configureConstantColorControls();
    void configureSecondaryTransferFunction();

    void connectWidgets();

    void calculateStride(string varName);
    void updateQtWidgets();
    void updateColorInterpolation();
    void updateConstColor();
    void updateMainAutoUpdateHistoCheckboxes();
    void updateMainSliders();
    void updateSecondaryAutoUpdateHistoCheckbox();
    void updateSecondarySliders();

    bool mainVariableChanged();
    bool secondaryVariableChanged();

    void enableTFWidget(bool state);

    void enableUpdateButtonsIfNeeded();
    void checkForVariableChanges();
    void checkForBoxChanges();
    void checkForCompressionChanges();
    void checkForMainMapperRangeChanges();
    void checkForSecondaryMapperRangeChanges();
    void checkForTimestepChanges();

    bool                   getAutoUpdateMainHisto();
    bool                   getAutoUpdateSecondaryHisto();
    VAPoR::MapperFunction *getMainMapperFunction();
    VAPoR::MapperFunction *getSecondaryMapperFunction();

    string getTFVariableName(bool mainTF);

    int   convertOpacityToSliderValue(float opacity) const;
    float convertSliderValueToOpacity(int value) const;

    std::vector<double> _minExt;
    std::vector<double> _maxExt;
    std::vector<double> _varRange;
    int                 _cLevel;
    int                 _refLevel;
    int                 _timeStep;
    int                 _stride;
    string              _mainVarName;
    string              _secondaryVarName;
    bool                _initialized;
    bool                _externalChangeHappened;
    bool                _mainHistoRangeChanged;
    bool                _secondaryHistoRangeChanged;
    bool                _mainHistoNeedsRefresh;
    bool                _secondaryHistoNeedsRefresh;
    bool                _isOpacitySupported;
    bool                _isOpacityIntegrated;
    bool                _wasOpacitySliderReleased;

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

class TFWidget_::LoadTFDialog : public QDialog {
    Q_OBJECT

public:
    LoadTFDialog(QWidget *parent = 0);
    ~LoadTFDialog();
    bool   GetLoadTF3OpacityMap() const;
    bool   GetLoadTF3DataRange() const;
    string GetSelectedFile() const;
    void   SetMapperFunction(const VAPoR::MapperFunction *fn);

public slots:
    void BuildColormapButtons();

private slots:
    void accept();
    void reject();
    void setLoadOpacity();
    void setLoadBounds();
    void buttonChecked();
    void buttonDoubleClicked();
    void checkSelectedColorButton(const QString &);

private:
    void                        initializeLayout();
    void                        configureLayout();
    void                        connectWidgets();
    void                        rebuildWidgets();
    VPushButtonWithDoubleClick *makeButton(const QString &path, const QString &file);

    CustomFileDialog *_fileDialog;
    QFrame *          _checkboxFrame;
    QFrame *          _fileDialogFrame;
    QFrame *          _colormapButtonFrame;
    QVBoxLayout *     _mainLayout;
    QHBoxLayout *     _checkboxLayout;
    QVBoxLayout *     _fileDialogLayout;
    QGridLayout *     _colormapButtonLayout;
    QTabWidget *      _fileDialogTab;
    QTabWidget *      _loadOptionTab;
    QTabWidget *      _colormapButtonTab;
    QSpacerItem *     _optionSpacer1;
    QSpacerItem *     _optionSpacer2;
    QSpacerItem *     _optionSpacer3;
    QCheckBox *       _loadOpacityMapCheckbox;
    QCheckBox *       _loadDataBoundsCheckbox;
    QButtonGroup *    _buttonGroup;

    std::unique_ptr<VAPoR::MapperFunction> _mapperFnCopy;
    QDir                                   _myDir;

    bool   _loadOpacityMap;
    bool   _loadDataBounds;
    string _selectedFile;
};

class TFWidget_::CustomFileDialog : public QFileDialog {
    Q_OBJECT

public:
    CustomFileDialog(QWidget *parent);

protected:
    void done(int result);
    void accept();

signals:
    void okClicked();
    void cancelClicked();
};
#endif    // TFWIDGET_H
