#ifndef TFWIDGET_H
#define TFWIDGET_H

#include "TFWidgetGUI.h"
#include "EventRouter.h"
#include "RangeCombos.h"

namespace VAPoR {
class ControlExec;
class MapperFunction;
}    // namespace VAPoR

class TFWidget : public QWidget, public Ui_TFWidgetGUI {
    Q_OBJECT

public:
    //! Bit masks to indicate what type of variables are to be supported by
    //! a particular VariablesWidget instance. These flags correspond
    //! to variable names returned by methods:
    //!
    //! SCALAR : RenderParams::GetVariableName()
    //! VECTOR : RenderParams::GetFieldVariableNames()
    //! HGT : RenderParams::GetHeightVariableName()
    //! COLOR : RenderParams::GetColorMapVariableNames()
    //!
    enum Flags {
        COLORMAPPED = (1u << 0),
    };

    TFWidget(QWidget *parent = 0);

    void Reinit(Flags flags);

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

    void setDataStatus(VAPoR::DataStatus *ds) { mappingFrame->setDataStatus(ds); }

    void fileLoadTF(string varname, const char *path, bool savePath);

    void loadTF(string varname);

    void loadInstalledTF(string varname);

private slots:
    void fileSaveTF();
    void setRange();
    void setRange(double min, double max);
    void updateHisto();
    void autoUpdateHistoChecked(int state);
    void colorInterpChanged(int index);
    void loadTF();
    void setCMVar();
    void setSingleColor();

private:
    void getRange(float range[2], float values[2]);
    void connectWidgets();
    void updateSliders();
    void updateAutoUpdateHistoCheckbox();
    void updateColorInterpolation();
    void updateMappingFrame();
    void updateColorVarCombo();
    void enableTFWidget(bool state);
    void collapseColormapSettings();

    int confirmMinRangeEdit(VAPoR::MapperFunction *tf, float *range);
    int confirmMaxRangeEdit(VAPoR::MapperFunction *tf, float *range);

    bool  _autoUpdateHisto = false;
    bool  _discreteColormap = false;
    bool  _textChanged = false;
    float _myRGB[3];

    // VAPoR::ControlExec* _controlExec;
    RenderEventRouter *  _eventRouter;
    VAPoR::ParamsMgr *   _paramsMgr;
    VAPoR::DataMgr *     _dataMgr;
    VAPoR::RenderParams *_rParams;

    Combo *     _minCombo = NULL;
    Combo *     _maxCombo = NULL;
    RangeCombo *_rangeCombo = NULL;

    Flags _flags;

    static string _nDimsTag;

#ifdef DEAD
    void makeItRed(QLineEdit *edit);
    void makeItYellow(QLineEdit *edit);
    void makeItGreen(QLineEdit *edit);
    void makeItWhite(QLineEdit *edit);
#endif
};

#endif    // TFWIDGET_H
