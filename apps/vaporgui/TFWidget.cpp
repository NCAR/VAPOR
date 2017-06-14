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
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include <qradiobutton.h>
#include "TwoDSubtabs.h"
#include "RenderEventRouter.h"
#include "StartupParams.h"
#include "vapor/RenderParams.h"
#include "vapor/TwoDDataParams.h"
#include "MainForm.h"
#include "TFWidget.h"

using namespace VAPoR;

TFWidget::TFWidget(QWidget* parent) 
	: QWidget(parent), Ui_TFWidgetGUI() {
	setupUi(this);

	_minCombo = new Combo(minRangeEdit, minRangeSlider);
	_maxCombo = new Combo(maxRangeEdit, maxRangeSlider);
	_rangeCombo = new RangeCombo(_minCombo, _maxCombo);
	
	connectWidgets();

	/*colormapVarCombo->setVisible(false);
	colormapVarLabel->setVisible(false);
	constantColorLabel->setVisible(false);
	colorSelectEdit->setVisible(false);
	colorSelectButton->setVisible(false);*/
	//colormapVarSpacer->hide();
	//colormapVarCombo->setMaximumHeight(0);
	//colormapVarLabel->setMaximumHeight(0);
	//colormapVarSpacer->setMaximumHeight(0);
	//colormapVarLayout->hide();
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

void TFWidget::setEventRouter(RenderEventRouter* e) {
	_eventRouter = e;
	mappingFrame->hookup(
		_eventRouter,
		updateHistoButton,
		opacitySlider);	
}

void TFWidget::getRange(float range[2], 
						float values[2]) {

	string varName;
	if (_flags & COLORMAPPED) {
		varName = _rParams->GetColorMapVariableName();
	}	
	else {
		varName = _rParams->GetVariableName();
	}

	size_t ts = getCurrentTimestep(_paramsMgr);
	int ref = _rParams->GetRefinementLevel();
	int cmp = _rParams->GetCompressionLevel();

	vector<double> minExt, maxExt;
	Box* myBox = _rParams->GetBox();
	myBox->GetExtents(minExt, maxExt);

	StructuredGrid* myVar;
	myVar = _dataMgr->GetVariable(ts, varName, ref, cmp, minExt, maxExt);
	myVar->GetRange(range);

	MapperFunction* tf = _rParams->GetMapperFunc(varName);
	values[0] = tf->getMinMapValue();
	values[1] = tf->getMaxMapValue();
}

void TFWidget::updateColorInterpolation() {
	string varName;
	if (_flags & COLORMAPPED) {
		varName = _rParams->GetColorMapVariableName();
	}	
	else {
		varName = _rParams->GetVariableName();
	}
	MapperFunction* tf = _rParams->GetMapperFunc(varName);
	
	TFInterpolator::type t = tf->getColorInterpType();
	colorInterpCombo->blockSignals(true);
	if (t == TFInterpolator::diverging) {
		colorInterpCombo->setCurrentIndex(0);
	}
	else if (t == TFInterpolator::discrete) {
		colorInterpCombo->setCurrentIndex(1);
	}
	else {
		colorInterpCombo->setCurrentIndex(2);
	}
	colorInterpCombo->blockSignals(false);
}

void TFWidget::updateAutoUpdateHistoCheckbox() {
	string varName;
	if (_flags & COLORMAPPED) {
		varName = _rParams->GetColorMapVariableName();
	}	
	else {
		varName = _rParams->GetVariableName();
	}
	return;
	cout << "VARNAME " << varName << endl;

	MapperFunction* tf = _rParams->GetMapperFunc(varName);
	assert(tf);

	// Update the state of autoUpdateHisto according to params
	//
	autoUpdateHistoCheckbox->blockSignals(true);
	if (tf->getAutoUpdateHisto()) {
		autoUpdateHistoCheckbox->setCheckState(Qt::Checked);
	}
	else {
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
	mappingFrame->Update(_rParams);
	mappingFrame->fitToView();
	//mappingFrame->updateHisto();
}

void TFWidget::Update(ParamsMgr *paramsMgr,
					DataMgr *dataMgr,
					RenderParams *rParams) {

	cout << "Updating TFWidget" << endl;

	assert(paramsMgr);
	assert(dataMgr);
	assert(rParams);

	_paramsMgr = paramsMgr;
	_dataMgr = dataMgr;
	_rParams = rParams;


	updateAutoUpdateHistoCheckbox();
	updateMappingFrame();
	updateColorInterpolation();
	updateSliders();
}

void TFWidget::connectWidgets() {
	connect(_rangeCombo, SIGNAL(valueChanged(double, double)),
		this, SLOT(setRange(double, double)));
	connect(updateHistoButton, SIGNAL(pressed()), 
		this, SLOT(updateHisto()));
	connect(autoUpdateHistoCheckbox, SIGNAL(stateChanged(int)), 
		this, SLOT(autoUpdateHistoChecked(int)));
	connect(colorInterpCombo, SIGNAL(currentIndexChanged(int)), 
		this, SLOT(colorInterpChanged(int)));
	connect(loadButton, SIGNAL(pressed()), 
		this, SLOT(loadTF()));
	connect(saveButton, SIGNAL(pressed()), 
		this, SLOT(saveTF()));
}	

void TFWidget::setRange(double min, double max) {
	string varName;
	if (_flags & COLORMAPPED) {
		varName = _rParams->GetColorMapVariableName();
	}	
	else {
		varName = _rParams->GetVariableName();
	}
	MapperFunction* tf = _rParams->GetMapperFunc(varName);

	tf->setMinMapValue(min);
	tf->setMaxMapValue(max);

	if (_autoUpdateHisto) {
		updateHisto();
	}
	else mappingFrame->fitToView(); 
}

void TFWidget::makeItRed(QLineEdit* edit) {
	QPalette p;
	p.setColor(QPalette::Base, QColor(255,150,150));
	edit->setPalette(p);
}

void TFWidget::makeItWhite(QLineEdit* edit) {
	QPalette p;
	p.setColor(QPalette::Base, QColor(255,255,255));
	edit->setPalette(p);
}

void TFWidget::makeItGreen(QLineEdit* edit) {
	QPalette p;
	p.setColor(QPalette::Base, QColor(150,255,150));
	edit->setPalette(p);
}

void TFWidget::makeItYellow(QLineEdit* edit) {
	QPalette p;
	p.setColor(QPalette::Base, QColor(255,255,150));
	edit->setPalette(p);
}

void TFWidget::textChanged() {
	_textChanged = true;
	makeItGreen((QLineEdit*)sender());
}

size_t TFWidget::getCurrentTimestep(ParamsMgr* paramsMgr) {
	GUIStateParams* p = MainForm::getInstance()->GetStateParams();
	string vizName = p->GetActiveVizName();

	size_t ts = paramsMgr->GetAnimationParams()->GetCurrentTimestep();
	return ts;
}

void TFWidget::updateHisto() {

	mappingFrame->fitToView();
	mappingFrame->updateMap();
	mappingFrame->Update();
}

void TFWidget::autoUpdateHistoChecked(int state) {
	if (state==0) _autoUpdateHisto = false;
	else _autoUpdateHisto = true;
	updateHisto();
}

void TFWidget::colorInterpChanged(int index) {
	string varName;
	if (_flags & COLORMAPPED) {
		varName = _rParams->GetColorMapVariableName();
	}	
	else {
		varName = _rParams->GetVariableName();
	}
	MapperFunction* tf = _rParams->GetMapperFunc(varName);

	if (index==0) {
		tf->setColorInterpType(TFInterpolator::diverging);
	}
	else if (index==1) {
		tf->setColorInterpType(TFInterpolator::discrete);
	}
	else if (index==2) {
		tf->setColorInterpType(TFInterpolator::linear);
	}
	updateHisto();	
}

void TFWidget::loadTF() {
	string varName;
	if (_flags & COLORMAPPED) {
		varName = _rParams->GetColorMapVariableName();
	}	
	else {
		varName = _rParams->GetVariableName();
	}
	dynamic_cast<RenderEventRouter*>(_eventRouter)->loadInstalledTF(varName);
	updateHisto();
}

void TFWidget::saveTF() {
	dynamic_cast<RenderEventRouter*>(_eventRouter)->fileSaveTF();
}
