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
#include "StartupParams.h"
#include "vapor/RenderParams.h"
#include "vapor/TwoDDataParams.h"
#include "vapor/GetAppPath.h"
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
}

void TFWidget::Reinit(Flags flags) {
    _flags = flags;
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

void TFWidget::setCMVar(const QString &qvar) {
    _paramsMgr->BeginSaveStateGroup("TFWidget::setCMVar(), "
                                    "set colormapped variable");

    string var = qvar.toStdString();
    _rParams->SetColorMapVariableName(var);

    if (_flags & CONSTCOLOR) {
        if (var == "Default") {
            colorDisplay->setEnabled(false);
            colorSelectButton->setEnabled(false);
            _rParams->SetUseSingleColor(false);
        } else {
            colorDisplay->setEnabled(true);
            colorSelectButton->setEnabled(true);
            _rParams->SetUseSingleColor(true);
            setSingleColor();
        }
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
        if (!_rParams->GetMapperFunc(var))
            _rParams->MakeMapperFunc(var);

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

    TransferFunction *tf = _rParams->GetTransferFunc(varname);
    if (!tf) {
        tf = _rParams->MakeTransferFunc(varname);
        assert(tf);
    }

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

    TransferFunction *tf = _rParams->GetTransferFunc(varname);
    if (!tf) {
        tf = _rParams->MakeTransferFunc(varname);
        assert(tf);
    }

    int rc = tf->SaveToFile(s.toStdString());
    if (rc < 0) {
        MSG_ERR("Failed to write output file");
        return;
    }
}

void TFWidget::getRange(float range[2],
                        float values[2]) {

    string varName;
    if (_flags & COLORMAPPED) {
        varName = _rParams->GetColorMapVariableName();
        if (varName == "") {
            return;
        }
    } else {
        varName = _rParams->GetVariableName();
    }

    size_t ts = _rParams->GetCurrentTimestep();
    int ref = _rParams->GetRefinementLevel();
    int cmp = _rParams->GetCompressionLevel();

    vector<double> minExt, maxExt;
    Box *myBox = _rParams->GetBox();
    myBox->GetExtents(minExt, maxExt);

    StructuredGrid *myVar;
    myVar = _dataMgr->GetVariable(ts, varName, ref, cmp, minExt, maxExt);
    myVar->GetRange(range);

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    values[0] = tf->getMinMapValue();
    values[1] = tf->getMaxMapValue();
}

void TFWidget::updateColorInterpolation() {
    string varName;
    if (_flags & COLORMAPPED) {
        varName = _rParams->GetColorMapVariableName();
    } else {
        varName = _rParams->GetVariableName();
    }

    if (varName == "") {
        return;
    }

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    if (tf == NULL) {
        tf = _rParams->MakeMapperFunc(varName);
    }

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
    string varName;
    if (_flags & COLORMAPPED) {
        varName = _rParams->GetColorMapVariableName();
    } else {
        varName = _rParams->GetVariableName();
    }
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
    if (_flags & COLORMAPPED) {
        varName = _rParams->GetColorMapVariableName();
        // If we are using a single color instead of a
        // color mapped variable, disable the transfer function
        //
        if (varName == "") {
            enableTFWidget(false);
            return;
        }

        // Otherwise enable it and continue on to updating the
        // min/max sliders in the transfer function
        //
        else {
            enableTFWidget(true);
        }
    } else {
        collapseColorVarSettings();
        collapseConstColorSettings();
    }

    updateSliders();
}

void TFWidget::updateColorVarCombo() {
    int index = colormapVarCombo->currentIndex();

    if (_flags & CONSTCOLOR) {
        colormapVarCombo->clear();
        colormapVarCombo->addItem(QString("Default"));
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
    if (_flags & COLORMAPPED) {
        varName = _rParams->GetColorMapVariableName();
    } else {
        varName = _rParams->GetVariableName();
    }
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
    if (_flags & COLORMAPPED) {
        varName = _rParams->GetColorMapVariableName();
    } else {
        varName = _rParams->GetVariableName();
    }
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
    if (_flags & COLORMAPPED) {
        varname = _rParams->GetColorMapVariableName();
    } else {
        varname = _rParams->GetVariableName();
    }

    //Ignore TF's in session, for now.

    GUIStateParams *p;
    p = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());
    string path = p->GetCurrentTFPath();

    fileLoadTF(varname, p->GetCurrentTFPath().c_str(), true);

    Update(_dataMgr, _paramsMgr, _rParams);
}

void TFWidget::loadInstalledTF(string varname) {
    //Get the path from the environment:
    vector<string> paths;
    paths.push_back("palettes");
    string palettes = GetAppPath("VAPOR", "share", paths);

    QString installPath = palettes.c_str();
    fileLoadTF(varname, (const char *)installPath.toAscii(), false);

    Update(_dataMgr, _paramsMgr, _rParams);
    updateHisto();
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
