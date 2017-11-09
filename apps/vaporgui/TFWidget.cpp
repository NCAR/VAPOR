//************************************************************************
//							  *
//	   Copyright (C)  2017					*
//	 University Corporation for Atmospheric Research		  *
//	   All Rights Reserved					*
//							  *
//************************************************************************/
//
//  File:	   TFWidget.cpp
//
//  Author:	 Scott Pearse
//	  National Center for Atmospheric Research
//	  PO 3000, Boulder, Colorado
//
//  Date:	   March 2017
//
//  Description:	Implements the TFWidget class.  This provides
//	  a widget that is inserted in the "Appearance" tab of various Renderer GUIs
//
#include <GL/glew.h>
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include <QFontDatabase>
#include <QStringList>
#include <qradiobutton.h>
#include <qcolordialog.h>
#include "TwoDSubtabs.h"
#include "RenderEventRouter.h"
#include "vapor/RenderParams.h"
#include "vapor/TwoDDataParams.h"
#include "MainForm.h"
#include "TFWidget.h"
#include "ErrorReporter.h"

using namespace VAPoR;

string TFWidget::_nDimsTag = "ActiveDimension";

TFWidget::TFWidget(QWidget *parent)
    : QWidget(parent), Ui_TFWidgetGUI() {

    setupUi(this);

    _myRGB[0] = _myRGB[1] = _myRGB[2] = 1.f;

    _minCombo = new Combo(minRangeEdit, minRangeSlider);
    _maxCombo = new Combo(maxRangeEdit, maxRangeSlider);
    _rangeCombo = new RangeCombo(_minCombo, _maxCombo);

    connectWidgets();

    collapseAutoUpdateHistoCheckbox();
}

void TFWidget::collapseAutoUpdateHistoCheckbox() {
    autoUpdateHistoLabel->hide();
    autoUpdateHistoLabel->resize(0, 0);
    autoUpdateHistoCheckbox->hide();
    autoUpdateHistoCheckbox->resize(0, 0);
    //autoUpdateHistoFrame->hide();
    autoUpdateHistoFrame->resize(0, 7);
    adjustSize();
}

void TFWidget::Reinit(Flags flags) {
    _flags = flags;

    // If const-color selection is a priority for the user,
    // as is the case in barbs or isosurfaces, move the const
    // color selector to the top.
    //
    if (_flags & PRIORITY_COLORVAR) {
        QVBoxLayout *myLayout = (QVBoxLayout *)layout();

        myLayout->removeWidget(constColorFrame);
        myLayout->insertWidget(0, constColorFrame);

        myLayout->removeWidget(colorMappingFrame);
        myLayout->insertWidget(0, colorMappingFrame);
    }

    if (!(_flags & CONSTCOLOR)) {
        collapseConstColorSettings();
    }
}

TFWidget::~TFWidget() {
    if (_minCombo) {
        delete _minCombo;
        _minCombo = NULL;
    }
    if (_maxCombo) {
        delete _maxCombo;
        _maxCombo = NULL;
    }
    if (_rangeCombo) {
        delete _rangeCombo;
        _rangeCombo = NULL;
    }
}

void TFWidget::setNativeTransferFunction(string var) {
    TransferFunction *tf = GetTransferFunc(var);
    if (tf == NULL) {
        _rParams->MakeMapperFunc(var);
    }
    _rParams->SetTransferFunc(var, tf);
}

void TFWidget::configureConstColorWidgets(string var) {
    if (var == "Map to var") {
        colorDisplay->setEnabled(false);
        colorSelectButton->setEnabled(false);

        colorInterpCombo->setEnabled(true);
        colorInterpLabel->setEnabled(true);

        setNativeTransferFunction(var);

        _rParams->SetUseSingleColor(false);
    } else if (var == "Constant") {
        colorDisplay->setEnabled(true);
        colorSelectButton->setEnabled(true);

        colorInterpCombo->setEnabled(false);
        colorInterpLabel->setEnabled(false);

        _rParams->SetUseSingleColor(true);
    }
}

void TFWidget::setCMVar(const QString &qvar) {
    _paramsMgr->BeginSaveStateGroup("TFWidget::setCMVar(), "
                                    "set colormapped variable");

    string var = qvar.toStdString();
    _rParams->SetColorMapVariableName(var);

    if (_flags & CONSTCOLOR) {
        configureConstColorWidgets(var);
        _paramsMgr->EndSaveStateGroup();
        return;
    }

    if (var == "Constant" || var == "") {
        var = "";
        _rParams->SetColorMapVariableName(var);
        _rParams->SetUseSingleColor(true);
        _rParams->SetConstantColor(_myRGB);

        colorSelectButton->setEnabled(true);
        minRangeSlider->setEnabled(false);
        minRangeEdit->setEnabled(false);
        maxRangeSlider->setEnabled(false);
        maxRangeEdit->setEnabled(false);
        colorInterpCombo->setEnabled(false);
    } else {
        _rParams->SetColorMapVariableName(var);
        _rParams->SetUseSingleColor(false);

        colorSelectButton->setEnabled(false);
        minRangeSlider->setEnabled(true);
        minRangeEdit->setEnabled(true);
        maxRangeSlider->setEnabled(true);
        maxRangeEdit->setEnabled(true);
        colorInterpCombo->setEnabled(true);
    }

    _paramsMgr->EndSaveStateGroup();
}

void TFWidget::collapseColorVarSettings() {
    colormapVarCombo->hide();
    colormapVarCombo->resize(0, 0);
    colorVarLabel->hide();
    colorVarLabel->resize(0, 0);
}

void TFWidget::collapseConstColorSettings() {
    colorDisplay->hide();
    colorDisplay->resize(0, 0);
    constColorLabel->hide();
    constColorLabel->resize(0, 0);
    colorSelectButton->hide();
    colorSelectButton->resize(0, 0);
    constColorFrame->hide();
    constColorFrame->resize(0, 0);
    adjustSize();
}

void TFWidget::setSingleColor() {
    _paramsMgr->BeginSaveStateGroup("TFWidget::setSingleColor()");
    QPalette palette(colorDisplay->palette());
    QColor color = QColorDialog::getColor(palette.color(QPalette::Base), this);
    if (!color.isValid())
        return;

    palette.setColor(QPalette::Base, color);
    colorDisplay->setPalette(palette);

    qreal rgb[3];
    color.getRgbF(&rgb[0], &rgb[1], &rgb[2]);
    _myRGB[0] = rgb[0];
    _myRGB[1] = rgb[1];
    _myRGB[2] = rgb[2];

    _rParams->SetConstantColor(_myRGB);
    _rParams->SetUseSingleColor(true);
    if (_flags & CONSTCOLOR) {
        colormapVarCombo->setCurrentIndex(1);
    } else {
        colormapVarCombo->setCurrentIndex(0);
    }
    _paramsMgr->EndSaveStateGroup();
}

void TFWidget::enableTFWidget(bool state) {
    loadButton->setEnabled(state);
    saveButton->setEnabled(state);
    tfFrame->setEnabled(state);
    minRangeEdit->setEnabled(state);
    maxRangeEdit->setEnabled(state);
    opacitySlider->setEnabled(state);
    updateHistoButton->setEnabled(state);
    autoUpdateHistoCheckbox->setEnabled(state);
    colorInterpCombo->setEnabled(state);
}

void TFWidget::loadTF(string varname) {
    GUIStateParams *p;
    p = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());

    string path = p->GetCurrentTFPath();
    fileLoadTF(varname, path.c_str(), true);
}

void TFWidget::fileLoadTF(
    string varname, const char *startPath, bool savePath) {
    QString s = QFileDialog::getOpenFileName(0,
                                             "Choose a transfer function file to open",
                                             startPath,
                                             "Vapor 3 Transfer Functions (*.tf3)");

    // Null string indicates nothing selected
    if (s.length() == 0)
        return;

    // Force name to end with .tf3
    if (!s.endsWith(".tf3")) {
        s += ".tf3";
    }

    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    assert(tf);

    int rc = tf->LoadFromFile(s.toStdString());
    if (rc < 0) {
        MSG_ERR("Error loading transfer function");
    }
}

void TFWidget::fileSaveTF() {
    //Launch a file save dialog, open resulting file
    GUIStateParams *p;
    p = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());
    string path = p->GetCurrentTFPath();

    QString s = QFileDialog::getSaveFileName(0,
                                             "Choose a filename to save the transfer function",
                                             path.c_str(), "Vapor 3 Transfer Functions (*.tf3)");
    //Did the user cancel?
    if (s.length() == 0)
        return;
    //Force the name to end with .tf3
    if (!s.endsWith(".tf3")) {
        s += ".tf3";
    }

    string varname = _rParams->GetVariableName();
    if (varname.empty())
        return;

    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    assert(tf);

    int rc = tf->SaveToFile(s.toStdString());
    if (rc < 0) {
        MSG_ERR("Failed to write output file");
        return;
    }
}

string TFWidget::getVariableName() {
    string varName = "";
    if (_flags & SECONDARY_COLORVAR) {
        varName = _rParams->GetColorMapVariableName();
    } else {
        varName = _rParams->GetVariableName();
    }
    return varName;
}

void TFWidget::getRange(float range[2],
                        float values[2]) {

    range[0] = range[1] = 0.0;
    values[0] = values[1] = 0.0;
    string varName = getVariableName();
    if (varName.empty())
        return;

    size_t ts = _rParams->GetCurrentTimestep();
    int ref = _rParams->GetRefinementLevel();
    int cmp = _rParams->GetCompressionLevel();

    vector<double> minExt, maxExt;
    Box *myBox = _rParams->GetBox();
    myBox->GetExtents(minExt, maxExt);

    Grid *myVar;
    myVar = _dataMgr->GetVariable(ts, varName, ref, cmp, minExt, maxExt);
    myVar->GetRange(range);

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    values[0] = tf->getMinMapValue();
    values[1] = tf->getMaxMapValue();
}

void TFWidget::updateColorInterpolation() {
    string varName = getVariableName();
    if (varName == "") {
        return;
    }

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    assert(tf);

    TFInterpolator::type t = tf->getColorInterpType();
    colorInterpCombo->blockSignals(true);
    if (t == TFInterpolator::diverging) {
        colorInterpCombo->setCurrentIndex(0);
    } else if (t == TFInterpolator::discrete) {
        colorInterpCombo->setCurrentIndex(1);
    } else {
        colorInterpCombo->setCurrentIndex(2);
    }
    colorInterpCombo->blockSignals(false);
}

void TFWidget::updateAutoUpdateHistoCheckbox() {
    string varName = getVariableName();

    return;

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    assert(tf);

    // Update the state of autoUpdateHisto according to params
    //
    autoUpdateHistoCheckbox->blockSignals(true);
    if (tf->GetAutoUpdateHisto()) {
        autoUpdateHistoCheckbox->setCheckState(Qt::Checked);
    } else {
        autoUpdateHistoCheckbox->setCheckState(Qt::Unchecked);
    }
    autoUpdateHistoCheckbox->blockSignals(false);
}

void TFWidget::updateSliders() {
    // Update min/max transfer function sliders/lineEdits
    //
    float range[2], values[2];
    getRange(range, values);
    _rangeCombo->Update(range[0], range[1], values[0], values[1]);
}

void TFWidget::updateMappingFrame() {
    mappingFrame->Update(_dataMgr, _paramsMgr, _rParams);
    mappingFrame->fitToView();
}

void TFWidget::Update(DataMgr *dataMgr,
                      ParamsMgr *paramsMgr,
                      RenderParams *rParams) {

    assert(paramsMgr);
    assert(dataMgr);
    assert(rParams);

    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    _rParams = rParams;

    updateAutoUpdateHistoCheckbox();
    updateMappingFrame();
    updateColorInterpolation();
    updateColorVarCombo();

    string varName;
    if (_flags & SECONDARY_COLORVAR) {
        varName = _rParams->GetColorMapVariableName();
        // If we are using a single color instead of a
        // color mapped variable, disable the transfer function
        //
        if (varName == "") {
            enableTFWidget(false);
        }

        // Otherwise enable it and continue on to updating the
        // min/max sliders in the transfer function
        //
        else {
            enableTFWidget(true);
        }
    }

    //	if (_flags & CONSTCOLOR) {
    //	}
    //	else {
    //		collapseConstColorSettings();
    //	}

    updateSliders();
}

void TFWidget::updateColorVarCombo() {
    colormapVarCombo->blockSignals(true);
    int index = colormapVarCombo->currentIndex();

    if (_flags & CONSTCOLOR) {
        colormapVarCombo->clear();
        colormapVarCombo->addItem(QString("Map to var"));
        colormapVarCombo->addItem(QString("Constant"));
    } else {
        int ndim = _rParams->GetValueLong(_nDimsTag, 3);
        assert(ndim == 2 || ndim == 3);

        vector<string> vars = _dataMgr->GetDataVarNames(ndim, true);

        colormapVarCombo->clear();
        colormapVarCombo->addItem(QString("Constant"));
        for (int i = 0; i < vars.size(); i++) {
            colormapVarCombo->addItem(QString::fromStdString(vars[i]));
        }
    }
    colormapVarCombo->setCurrentIndex(index);

    // Update selected color display
    //

    float rgb[3];
    _rParams->GetConstantColor(rgb);
    QColor color(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
    QPalette palette(colorDisplay->palette());
    palette.setColor(QPalette::Base, color);
    colorDisplay->setPalette(palette);

    colormapVarCombo->blockSignals(false);
}

void TFWidget::connectWidgets() {
    connect(_rangeCombo, SIGNAL(valueChanged(double, double)),
            this, SLOT(setRange(double, double)));
    connect(updateHistoButton, SIGNAL(pressed()),
            this, SLOT(updateHisto()));
    connect(autoUpdateHistoCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(autoUpdateHistoChecked(int)));
    connect(colorInterpCombo, SIGNAL(activated(int)),
            this, SLOT(colorInterpChanged(int)));
    connect(loadButton, SIGNAL(pressed()),
            this, SLOT(loadTF()));
    connect(saveButton, SIGNAL(pressed()),
            this, SLOT(fileSaveTF()));
    connect(colormapVarCombo, SIGNAL(activated(const QString &)),
            this, SLOT(setCMVar(const QString &)));
    connect(colorSelectButton, SIGNAL(pressed()),
            this, SLOT(setSingleColor()));
    connect(mappingFrame, SIGNAL(updateParams()), this,
            SLOT(setRange()));
    connect(mappingFrame, SIGNAL(endChange()), this,
            SLOT(forwardTFChange()));
}

void TFWidget::forwardTFChange() {
    emit emitChange();
}

void TFWidget::setRange() {
    float min = mappingFrame->getMinEditBound();
    float max = mappingFrame->getMaxEditBound();
    setRange(min, max);
    emit emitChange();
}

void TFWidget::setRange(double min, double max) {
    string varName;
    if (_flags & SECONDARY_COLORVAR) {
        varName = _rParams->GetColorMapVariableName();
    } else {
        varName = _rParams->GetVariableName();
    }
    if (varName.empty())
        return;

    MapperFunction *tf = _rParams->GetMapperFunc(varName);

    tf->setMinMapValue(min);
    tf->setMaxMapValue(max);

    if (_autoUpdateHisto) {
        updateHisto();
    } else
        mappingFrame->fitToView();
    emit emitChange();
}

void TFWidget::updateHisto() {
    mappingFrame->fitToView();
    mappingFrame->updateMap();
    mappingFrame->Update(_dataMgr, _paramsMgr, _rParams);
}

void TFWidget::autoUpdateHistoChecked(int state) {
    if (state == 0)
        _autoUpdateHisto = false;
    else
        _autoUpdateHisto = true;
    updateHisto();
}

void TFWidget::colorInterpChanged(int index) {
    string varName;
    if (_flags & SECONDARY_COLORVAR) {
        varName = _rParams->GetColorMapVariableName();
    } else {
        varName = _rParams->GetVariableName();
    }
    if (varName.empty())
        return;

    MapperFunction *tf = _rParams->GetMapperFunc(varName);

    if (index == 0) {
        tf->setColorInterpType(TFInterpolator::diverging);
    } else if (index == 1) {
        tf->setColorInterpType(TFInterpolator::discrete);
    } else if (index == 2) {
        tf->setColorInterpType(TFInterpolator::linear);
    }
    updateHisto();
}

void TFWidget::loadTF() {
    string varname;
    if (_flags & SECONDARY_COLORVAR) {
        varname = _rParams->GetColorMapVariableName();
    } else {
        varname = _rParams->GetVariableName();
    }
    if (varname.empty())
        return;

    //Ignore TF's in session, for now.

    GUIStateParams *p;
    p = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());
    string path = p->GetCurrentTFPath();

    fileLoadTF(varname, p->GetCurrentTFPath().c_str(), true);

    Update(_dataMgr, _paramsMgr, _rParams);
}

#ifdef DEAD
void TFWidget::makeItRed(QLineEdit *edit) {
    QPalette p;
    p.setColor(QPalette::Base, QColor(255, 150, 150));
    edit->setPalette(p);
}

void TFWidget::makeItWhite(QLineEdit *edit) {
    QPalette p;
    p.setColor(QPalette::Base, QColor(255, 255, 255));
    edit->setPalette(p);
}

void TFWidget::makeItGreen(QLineEdit *edit) {
    QPalette p;
    p.setColor(QPalette::Base, QColor(150, 255, 150));
    edit->setPalette(p);
}

void TFWidget::makeItYellow(QLineEdit *edit) {
    QPalette p;
    p.setColor(QPalette::Base, QColor(255, 255, 150));
    edit->setPalette(p);
}
#endif
