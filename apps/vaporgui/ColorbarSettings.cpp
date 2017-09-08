//************************************************************************
//															*
//		     Copyright (C)  2016										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		ColorbarSettings.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016
//
//	Description:	Implements the ColorbarSettings class.  This provides
//		a frame that contains buttons and text for controlling a color bar
//		Intended to be used with colorbarframe.ui for event routers that
//		embed a colorbarframe
//
#include "vapor/RenderParams.h"
#include "ColorbarSettings.h"
#include "ui_colorbarframe.h"
#include "vapor/ColorbarPbase.h"
#include <QFrame>
#include <qwidget.h>
#include <vector>
#include <QString>
#include <qcolordialog.h>

using namespace VAPoR;

ColorbarSettings::ColorbarSettings(QWidget *parent)
    : QFrame(parent), Ui_ColorbarFrame() {
    setupUi(this);
    _eventRouter = NULL;
    _showSettings = false;
    _textChangedFlag = false;

    connect(colorbarXposEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));
    connect(colorbarYposEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));
    connect(colorbarXsizeEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));
    connect(colorbarYsizeEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));
    connect(colorbarFontsizeEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));
    connect(colorbarDigitsEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));
    connect(colorbarNumTicsEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));
    connect(colorbarTitleEdit, SIGNAL(returnPressed()),
            this, SLOT(colorbarReturnPressed()));

    connect(colorbarEnableCheckbox, SIGNAL(released()),
            this, SLOT(enabledChange()));
    connect(colorbarBackgroundSelectButton, SIGNAL(clicked()),
            this, SLOT(setBackgroundColor()));
    connect(colorbarApplyAllButton, SIGNAL(clicked()),
            this, SLOT(applyToAll()));
}

ColorbarSettings::~ColorbarSettings() {
}

void ColorbarSettings::Update(ParamsMgr *paramsMgr,
                              DataMgr *dataMgr,
                              RenderParams *rParams) {
    assert(paramsMgr);
    assert(dataMgr);
    assert(rParams);

    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    _rParams = rParams;
}

void ColorbarSettings::updateGUI() {

    ColorbarPbase *cbpb = GetActiveParams();

    vector<double> pos = cbpb->GetCornerPosition();
    colorbarXposEdit->setText(QString::number(pos[0]));
    colorbarYposEdit->setText(QString::number(pos[1]));

    vector<double> size = cbpb->GetCornerPosition();
    colorbarXsizeEdit->setText(QString::number(size[0]));
    colorbarYsizeEdit->setText(QString::number(size[1]));

    long fontSize = cbpb->GetFontSize();
    colorbarFontsizeEdit->setText(QString::number(fontSize));

    long numDigits = cbpb->GetNumDigits();
    colorbarDigitsEdit->setText(QString::number(numDigits));

    long numTics = cbpb->GetNumTics();
    colorbarNumTicsEdit->setText(QString::number(numTics));

    string title = cbpb->GetTitle();
    colorbarTitleEdit->setText(QString::fromStdString(title));

    if (cbpb->IsEnabled()) {
        colorbarEnableCheckbox->setCheckState(Qt::Checked);
    } else {
        colorbarEnableCheckbox->setCheckState(Qt::Unchecked);
    }
}

void ColorbarSettings::disableSignals(bool state) {
    QList<QWidget *> widgetList = this->findChildren<QWidget *>();
    QList<QWidget *>::const_iterator widgetIter(widgetList.begin());
    QList<QWidget *>::const_iterator lastWidget(widgetList.end());

    while (widgetIter != lastWidget) {
        (*widgetIter)->blockSignals(state);
        ++widgetIter;
    }
}

void ColorbarSettings::confirmText() {
    if (!_textChangedFlag)
        return;
    ColorbarPbase *cbPbase = GetActiveParams();

    vector<double> dvec;
    dvec.push_back(colorbarXposEdit->text().toDouble());
    dvec.push_back(colorbarYposEdit->text().toDouble());
    cbPbase->SetCornerPosition(dvec);
    dvec.clear();
    dvec.push_back(colorbarXsizeEdit->text().toDouble());
    dvec.push_back(colorbarYsizeEdit->text().toDouble());
    cbPbase->SetSize(dvec);
    cbPbase->SetTitle(colorbarTitleEdit->text().toStdString());
    cbPbase->SetNumTics(colorbarNumTicsEdit->text().toInt());
    cbPbase->SetNumDigits(colorbarDigitsEdit->text().toInt());
    cbPbase->SetFontSize(colorbarFontsizeEdit->text().toInt());

    _textChangedFlag = false;
}
void ColorbarSettings::updateTab() {

    ColorbarPbase *cbPbase = GetActiveParams();
    if (!cbPbase)
        return;
    vector<double> dvec = cbPbase->GetCornerPosition();
    colorbarXposEdit->setText(QString::number(dvec[0]));
    colorbarYposEdit->setText(QString::number(dvec[1]));
    dvec = cbPbase->GetSize();
    colorbarXsizeEdit->setText(QString::number(dvec[0]));
    colorbarYsizeEdit->setText(QString::number(dvec[1]));
    colorbarTitleEdit->setText(QString::fromStdString(cbPbase->GetTitle()));
    colorbarNumTicsEdit->setText(QString::number(cbPbase->GetNumTics()));
    colorbarDigitsEdit->setText(QString::number(cbPbase->GetNumDigits()));
    colorbarFontsizeEdit->setText(QString::number(cbPbase->GetFontSize()));
    //Set the background color edit
    const vector<double> &clr = cbPbase->GetBackgroundColor();
    QColor newColor;
    newColor.setRgbF((qreal)clr[0], (qreal)clr[1], (qreal)clr[2]);
    QPalette pal(ColorbarBackgroundColorEdit->palette());
    pal.setColor(QPalette::Base, newColor);
    ColorbarBackgroundColorEdit->setPalette(pal);
    colorbarEnableCheckbox->setChecked(cbPbase->IsEnabled());
    _textChangedFlag = false;
}

// slots:
void ColorbarSettings::colorbarTextChanged(const QString &) {
    _textChangedFlag = true;
}

void ColorbarSettings::colorbarReturnPressed() {
    confirmText();
}
//void ColorbarSettings::enabledChange(bool onoff){
void ColorbarSettings::enabledChange() {
    confirmText();
    Qt::CheckState state = colorbarEnableCheckbox->checkState();
    ColorbarPbase *cbPbase = GetActiveParams();
    if (!cbPbase)
        return;
    if (state == Qt::Unchecked) {
        cbPbase->SetEnabled(0);
    } else {
        cbPbase->SetEnabled(1);
    }
}
void ColorbarSettings::showSettings(bool onOff) {
    _showSettings = onOff;
    adjustSize();
    updateTab();
}
void ColorbarSettings::setBackgroundColor() {
    confirmText();
    QPalette pal(ColorbarBackgroundColorEdit->palette());
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
    if (!newColor.isValid())
        return;
    pal.setColor(QPalette::Base, newColor);
    ColorbarBackgroundColorEdit->setPalette(pal);
    qreal rgb[3];
    newColor.getRgbF(rgb, rgb + 1, rgb + 2);
    vector<double> rgbd;
    for (int i = 0; i < 3; i++)
        rgbd.push_back((double)rgb[i]);
    GetActiveParams()->SetBackgroundColor(rgbd);
}
void ColorbarSettings::applyToAll() {
#ifdef DEAD
    _paramsMgr->CopyColorBarSettings((RenderParams *)_params);
#endif
}
