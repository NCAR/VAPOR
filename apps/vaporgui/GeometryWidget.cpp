//************************************************************************
//						      *
//	   Copyright (C)  2017				    *
//     University Corporation for Atmospheric Research		  *
//	   All Rights Reserved					*
//						      *
//************************************************************************/
//
//  File:       GeometryWidget.cpp
//
//  Author:     Scott Pearse
//	  National Center for Atmospheric Research
//	  PO 3000, Boulder, Colorado
//
//  Date:       March 2017
//
//  Description:    Implements the GeometryWidget class.  This provides
//  a widget that is inserted in the "Appearance" tab of various Renderer GUIs
//
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include "vapor/RenderParams.h"
#include "MainForm.h"
#include "GeometryWidget.h"

using namespace VAPoR;

GeometryWidget::GeometryWidget(QWidget* parent) : 
			QWidget(parent), Ui_GeometryWidgetGUI() {
	setupUi(this);

	_paramsMgr = NULL;
	_dataMgr = NULL;
	_rParams = NULL;

	_minXCombo = new Combo(minXEdit, minXSlider);
	_maxXCombo = new Combo(maxXEdit, maxXSlider);
	_xRangeCombo = new RangeCombo(_minXCombo, _maxXCombo);

	_minYCombo = new Combo(minYEdit, minYSlider);
	_maxYCombo = new Combo(maxYEdit, maxYSlider);
	_yRangeCombo = new RangeCombo(_minYCombo, _maxYCombo);

	_minZCombo = new Combo(minZEdit, minZSlider);
	_maxZCombo = new Combo(maxZEdit, maxZSlider);
	_zRangeCombo = new RangeCombo(_minZCombo, _maxZCombo);

	connectWidgets();
}

void GeometryWidget::Reinit(Flags flags) {
	_flags = flags;

	if (_flags & TWOD) {
		zMinMaxGroupBox->hide();
	}
}

GeometryWidget::~GeometryWidget() {
	if (_minXCombo) {
		delete _minXCombo;
		_minXCombo = NULL;
	}
	if (_maxXCombo) {
		delete _maxXCombo;
		_maxXCombo = NULL;
	}
	if (_xRangeCombo) {
		delete _xRangeCombo;
		_xRangeCombo = NULL;
	}

	if (_minYCombo) {
		delete _minYCombo;
		_minYCombo = NULL;
	}
	if (_maxYCombo) {
		delete _maxYCombo;
		_maxYCombo = NULL;
	}
	if (_yRangeCombo) {
		delete _yRangeCombo;
		_yRangeCombo = NULL;
	}

	if (_minZCombo) {
		delete _minZCombo;
		_minZCombo = NULL;
	}
	if (_maxZCombo) {
		delete _maxZCombo;
		_maxZCombo = NULL;
	}
	if (_zRangeCombo) {
		delete _zRangeCombo;
		_zRangeCombo = NULL;
	}
}

size_t GeometryWidget::getCurrentTimestep() {
    GUIStateParams* p = MainForm::getInstance()->GetStateParams();
    string visName = p->GetActiveVizName();

    size_t ts = _paramsMgr->GetAnimationParams()->GetCurrentTimestep();
    return ts; 
}

void GeometryWidget::updateRangeLabels(
							vector<double> minExt,
							vector<double> maxExt) {
	QString xTitle = QString("X Coordinates         Min:") + 
		QString::number(minExt[0]) + 
		QString("         Max:") + 
		QString::number(maxExt[0]);
	xMinMaxGroupBox->setTitle(xTitle);

	QString yTitle = QString("Y Coordinates         Min:") + 
		QString::number(minExt[1]) +
		QString("         Max:") + 
		QString::number(maxExt[1]);
	yMinMaxGroupBox->setTitle(yTitle);

	QString zTitle = QString("Z Coordinates         Min:") + 
		QString::number(minExt[2]) +
		QString("         Max:") + 
		QString::number(maxExt[2]);
	zMinMaxGroupBox->setTitle(zTitle);
}

void GeometryWidget::GetVectorExtents(size_t ts, int level,
										vector<double> minFullExt,
										vector<double> maxFullExt) {
	
	vector<string> varNames = _rParams->GetFieldVariableNames();
	vector<double> minVarExt, maxVarExt;

	// Calculate the union of all field variable extents
	// by iterating over the variables one at a time, indexed by i
	//
	for (int i=0; i<3; i++) {
		if (varNames[i] != "") {
			int rc = _dataMgr->GetVariableExtents(ts, varNames[i], 
								level, minFullExt, maxFullExt);
			if (rc<0) {
				char bufr[50];
		sprintf(bufr, "Error: DataMgr could "
					"not return valid values from GetVariableExtents()"
					" for variable %s", varNames[i].c_str());
				MyBase::SetErrMsg(bufr);
			}
			for (int j=0; j<3; j++) {
				// If we are on the extents of the first
				// variable, just apply those extents as
				// our initial condition...
				//
				if (i==0) {
					minVarExt[j] = minFullExt[j];
					maxVarExt[j] = maxFullExt[j];
				}
				// ...Otherwise run our comparisons
				//
				else {
					if (minVarExt[j] < minFullExt[j]) {
						minFullExt[j] = minVarExt[j];
					}
					if (maxVarExt[j] > maxFullExt[j]) {
						maxFullExt[j] = maxVarExt[j];
					}
				}
			}
		}
	}
}

void GeometryWidget::updateVisCombo() {

	// Add new visualizer names, if any
	//
	vector<string> visNames = _paramsMgr->GetVisualizerNames();
	for (int i=0; i<visNames.size(); i++) {
		QString visName = QString::fromStdString(visNames[i]);

		if (copyVisCombo->findText(visName) < 0) {
			copyVisCombo->addItem(visName);
		}
	}

	// Remove deleted visualizer names, if any
	//
	for (int i=0; i<copyVisCombo->count(); i++) {
		string visName = copyVisCombo->itemText(i).toStdString();
		if (std::find(visNames.begin(), visNames.end(), visName) == visNames.end() &&
			(visName != "Visualizer")) {
			copyVisCombo->removeItem(i);
			i=0; 	// Redo search since our indices are now screwed up :(
		} 
	}
}

void GeometryWidget::updateRenTypeCombo() {

	// Now update the renderer selector menu based on
	// whatever visualizer is selected
	//

	// Default case where no visualizer is selected
	//
	string visName = copyVisCombo->currentText().toStdString();
	if (visName == "Visualizer") {
		copyRenTypeCombo->clear();
		copyRenTypeCombo->addItem(QString("Type"));
		return;
	}

	// Now add new renderer names pertaining to our current visualizer
	// if they weren't in the comboBox already
	//
	vector<string> typeNames = _paramsMgr->GetRenderParamsClassNames(visName);
	for (int i=0; i<typeNames.size(); i++) {
		QString typeName = QString::fromStdString(typeNames[i]);
		if (copyRenTypeCombo->findText(typeName) < 0) {
			copyRenTypeCombo->addItem(typeName);
		}
	}

	// Finally removed renderer names that may have been removed from
	//
	for (int i=0; i<copyRenTypeCombo->count(); i++) {
		string typeName = copyRenTypeCombo->itemText(i).toStdString();
		if ((std::find(typeNames.begin(), typeNames.end(), \
			typeName) == typeNames.end()) && (typeName != "Type")) {
			copyRenTypeCombo->removeItem(i);
			i=0; 	// Redo search since our indices are now screwed up :(
		} 
	}
}

void GeometryWidget::updateRenNameCombo() {

	// Now update the renderer selector menu based on
	// whatever visualizer is selected
	//

	// Default case where no visualizer is selected
	//
	string visName = copyVisCombo->currentText().toStdString();
	string typeName = copyRenTypeCombo->currentText().toStdString();
	if (typeName == "Type") {
		copyRenNameCombo->clear();
		copyRenNameCombo->addItem(QString("Name"));
		return;
	}

	// Now add new renderer names pertaining to our current visualizer
	// if they weren't in the comboBox already
	//
	vector<string> renNames = _paramsMgr->GetRenderParamInstances(visName,
																typeName);
	for (int i=0; i<renNames.size(); i++) {
		QString renName = QString::fromStdString(renNames[i]);
		if (copyRenNameCombo->findText(renName) < 0) {
			copyRenNameCombo->addItem(renName);
		}
	}

	// Finally removed renderer names that may have been removed from
	//
	for (int i=0; i<copyRenNameCombo->count(); i++) {
		string renName = copyRenNameCombo->itemText(i).toStdString();
		if ((std::find(renNames.begin(), renNames.end(), renName) 
			== renNames.end()) && (renName != "Name")) {
			copyRenNameCombo->removeItem(i);
			i=0; 	// Redo search since our indices are now screwed up :(
		} 
	}
}


void GeometryWidget::copyRegion() {
	cout << "Copying region " << copyRenTypeCombo->currentText().toStdString() << 
	" " << copyVisCombo->currentText().toStdString() << " " << 
	copyRenNameCombo->currentText().toStdString() << endl;

	string visualizer = copyVisCombo->currentText().toStdString();
	if (visualizer == "Visualizer") {	
			MyBase::SetErrMsg("You must select a Visualizer, Renderer Type, and "
					"Renderer Name before copying extents");
	}
	string renType = copyRenTypeCombo->currentText().toStdString();
	if (visualizer == "Type") {	
			MyBase::SetErrMsg("You must select a Renderer Type and "
					"Renderer Name before copying extents");
	}
	string renderer = copyRenNameCombo->currentText().toStdString();             
	if (visualizer == "Name") {	
			MyBase::SetErrMsg("You must select a "
					"Renderer Name before copying extents");
	}
	
	RenderParams* copyParams = _paramsMgr->GetRenderParams(
												visualizer, 
												renType, 
												renderer);
	assert(copyParams);

	Box* copyBox = copyParams->GetBox();
	vector<double> minExtents, maxExtents;
	copyBox->GetExtents(minExtents, maxExtents);

	Box* myBox = _rParams->GetBox();
	myBox->SetExtents(minExtents, maxExtents);
}

void GeometryWidget::Update(ParamsMgr *paramsMgr,
							DataMgr* dataMgr,
							RenderParams* rParams) {

	assert(paramsMgr);
	assert(dataMgr);
	assert(rParams);

	_paramsMgr = paramsMgr;
	_dataMgr = dataMgr;
	_rParams = rParams;

	// Get current domain extents
	//
	size_t ts = getCurrentTimestep();
	int level = _rParams->GetRefinementLevel();
	vector<double> minFullExt, maxFullExt;

	if (_flags & VECTOR) {
		GetVectorExtents(ts, level, minFullExt, maxFullExt);
	}
	else {
		string varName = _rParams->GetVariableName();
		int rc = _dataMgr->GetVariableExtents(ts, varName, 
							level, minFullExt, maxFullExt);
		if (rc<0) {
			MyBase::SetErrMsg("Error: DataMgr could "
				"not return valid values from GetVariableExtents()");
		}
	}

	updateRangeLabels(minFullExt, maxFullExt);
	updateRenTypeCombo();
	updateVisCombo();

	// Get current user selected extents
	//
	Box* box = _rParams->GetBox();
	vector<double> minExt, maxExt;
	box->GetExtents(minExt, maxExt);
	
	// Verify that the user extents comply with the domain extents
	//
	size_t extSize = box->IsPlanar() ? 2 : 3;
	for (int i=0; i<extSize; i++) {
		if (minExt[i] < minFullExt[i]) minExt[i] = minFullExt[i];
		if (maxExt[i] > maxFullExt[i]) maxExt[i] = maxFullExt[i];
	}

	_xRangeCombo->Update(minFullExt[0], maxFullExt[0], minExt[0], maxExt[0]);
	_yRangeCombo->Update(minFullExt[1], maxFullExt[1], minExt[1], maxExt[1]);
	if (!box->IsPlanar()) {
		_zRangeCombo->Update(minFullExt[2], maxFullExt[2],
							 minExt[2], maxExt[2]);
	}
}


void GeometryWidget::connectWidgets() {
	connect(_xRangeCombo, SIGNAL(valueChanged(double,double)), 
		this, SLOT(setRange(double, double)));
	connect(_yRangeCombo, SIGNAL(valueChanged(double,double)), 
		this, SLOT(setRange(double, double)));
	connect(_zRangeCombo, SIGNAL(valueChanged(double,double)), 
		this, SLOT(setRange(double, double)));
	connect(copyVisCombo, SIGNAL(currentIndexChanged(int)), 
		this, SLOT(updateRenTypeCombo()));
	connect(copyRenTypeCombo, SIGNAL(currentIndexChanged(int)), 
		this, SLOT(updateRenNameCombo()));
	connect(copyButton, SIGNAL(released()), this, SLOT(copyRegion()));
}	

void GeometryWidget::setRange(double min, double max) {
	size_t dimension;
	if (QObject::sender() == _xRangeCombo) dimension = 0;
	else if (QObject::sender() == _yRangeCombo) dimension = 1;
	else dimension = 2;

	vector<double> minExt, maxExt;
	Box* box = _rParams->GetBox();
	box->GetExtents(minExt, maxExt);

	// Apply the extents that changed into minExt and maxExt
	//
	minExt[dimension] = min;
	maxExt[dimension] = max;

	box->SetExtents(minExt, maxExt);
}
