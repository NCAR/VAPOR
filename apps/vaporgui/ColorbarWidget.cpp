//************************************************************************
//															*
//		     Copyright (C)  2016										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		ColorbarWidget.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016
//
//	Description:	Implements the ColorbarWidget class.  This provides
//		a frame that contains buttons and text for controlling a color bar
//		Intended to be used with colorbarframe.ui for event routers that
//		embed a colorbarframe
//
#include "vapor/RenderParams.h"
#include "ColorbarWidget.h"
#include "vapor/ColorbarPbase.h"
#include <QFrame>
#include <qwidget.h>
#include <vector>
#include <QString>
#include <qcolordialog.h>
#include "vapor/DataStatus.h"

using namespace VAPoR;

ColorbarWidget::ColorbarWidget( QWidget * parent) 
	: QFrame(parent), Ui_ColorbarWidgetGUI(){
		setupUi(this);
		_eventRouter = NULL;

	_xPosCombo = new Combo(xPosEdit, xPosSlider);
	_yPosCombo = new Combo(yPosEdit, yPosSlider);
	_xSizeCombo = new Combo(xSizeEdit, xSizeSlider);
	_ySizeCombo = new Combo(ySizeEdit, ySizeSlider);
	_fontSizeCombo = new Combo(fontSizeEdit, fontSizeSlider, true);
	_fontDigitsCombo = new Combo(numDigitsEdit, numDigitsSlider, true);
	_numTicksCombo = new Combo(numTicksEdit, numTicksSlider, true);

	connectWidgets();
}

void ColorbarWidget::connectWidgets() {
	connect(enableCheckbox, SIGNAL(stateChanged(int)), this,
		SLOT(enableDisable(int)));
	connect(titleEdit, SIGNAL(returnPressed()), this,
		SLOT(titleChanged()));
	connect(bgSelectButton, SIGNAL(pressed()), this,
		SLOT(setBackgroundColor()));
	connect(_xPosCombo, SIGNAL(valueChanged(double)), this,
		SLOT(xPosChanged(double)));
	connect(_yPosCombo, SIGNAL(valueChanged(double)), this,
		SLOT(yPosChanged(double)));
	connect(_xSizeCombo, SIGNAL(valueChanged(double)), this,
		SLOT(xSizeChanged(double)));
	connect(_ySizeCombo, SIGNAL(valueChanged(double)), this,
		SLOT(ySizeChanged(double)));
	connect(_fontSizeCombo, SIGNAL(valueChanged(int)), this,
		SLOT(fontSizeChanged(int)));
	connect(_fontDigitsCombo, SIGNAL(valueChanged(int)), this,
		SLOT(fontDigitsChanged(int)));
	connect(_numTicksCombo, SIGNAL(valueChanged(int)), this,
		SLOT(numTicksChanged(int))); 
}

ColorbarWidget::~ColorbarWidget() {
}

void ColorbarWidget::titleChanged() {
	string title = titleEdit->text().toStdString();
	_cbPbase->SetTitle(title);
}

void ColorbarWidget::xPosChanged(double xPos) {
	vector<double> pos = _cbPbase->GetCornerPosition();
	pos[0] = xPos;
	_cbPbase->SetCornerPosition(pos);
}

void ColorbarWidget::yPosChanged(double yPos) {
	vector<double> pos = _cbPbase->GetCornerPosition();
	pos[1] = yPos;
	_cbPbase->SetCornerPosition(pos);
}

void ColorbarWidget::xSizeChanged(double xSize) {
	vector<double> size = _cbPbase->GetSize();
	size[0] = xSize;
	_cbPbase->SetSize(size);
}

void ColorbarWidget::ySizeChanged(double ySize) {
	vector<double> size = _cbPbase->GetSize();
	size[1] = ySize;
	_cbPbase->SetSize(size);
}

void ColorbarWidget::fontSizeChanged(int fSize) {
	_cbPbase->SetFontSize(fSize);
}

void ColorbarWidget::fontDigitsChanged(int digits) {
	_cbPbase->SetNumDigits(digits);
}

void ColorbarWidget::numTicksChanged(int numTicks) {
	_cbPbase->SetNumTicks(numTicks);
}

void ColorbarWidget::Update(DataMgr* dataMgr,
							ParamsMgr* paramsMgr,
							RenderParams* rParams) {
    assert(paramsMgr);
    assert(dataMgr);
    assert(rParams);

	_paramsMgr = paramsMgr;
	_dataMgr = dataMgr;
	_cbPbase = rParams->GetColorbarPbase();

	string title = _cbPbase->GetTitle();
	titleEdit->blockSignals(true);
	titleEdit->setText(QString::fromStdString(title));
	titleEdit->blockSignals(false);

	vector<double> pos = _cbPbase->GetCornerPosition();
	_xPosCombo->Update(0, 1, pos[0]);
	_yPosCombo->Update(0, 1, pos[1]);

	vector<double> size = _cbPbase->GetSize();
	_xSizeCombo->Update(0, 1, size[0]);
	_ySizeCombo->Update(0, 1, size[1]);

	int fSize = _cbPbase->GetFontSize();
	_fontSizeCombo->Update(1, 100, fSize);

	int digits = _cbPbase->GetNumDigits();
	_fontDigitsCombo->Update(1, 8, digits);

	int ticks = _cbPbase->GetNumTicks();
	_numTicksCombo->Update(1, 20, ticks);
}

void ColorbarWidget::enableDisable(int state){
	if (!_cbPbase) return;
	if (state==0) {
		_cbPbase->SetEnabled(0);
	}
	else if (state==2) {
		_cbPbase->SetEnabled(1);
	}
}

void ColorbarWidget::setBackgroundColor(){
	QPalette pal(bgColorEdit->palette());
	QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
	if (!newColor.isValid()) return;
	pal.setColor(QPalette::Base, newColor);
	bgColorEdit->setPalette(pal);
	qreal rgb[3];
	newColor.getRgbF(rgb,rgb+1,rgb+2);
	vector<double> rgbd;
	for (int i = 0; i<3; i++) rgbd.push_back((double)rgb[i]);
	_cbPbase->SetBackgroundColor(rgbd);
}
void ColorbarWidget::applyToAll(){
#ifdef	DEAD
	_paramsMgr->CopyColorBarSettings((RenderParams*)_params);
#endif
}
