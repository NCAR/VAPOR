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
#include "vapor/DataMgrUtils.h"
#include "MainForm.h"
#include "GeometryWidget.h"

using namespace VAPoR;

const string GeometryWidget::_nDimsTag = "ActiveDimension";

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

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

	QFont myFont = font();
	xMinMaxGroupBox->setFont(myFont);
}

void GeometryWidget::Reinit(Flags flags) {
	_flags = flags;
	if (_flags & TWOD) {
		zMinMaxGroupBox->hide();
		minMaxTab->adjustSize();
		xMinMaxGroupBox->adjustSize();
		yMinMaxGroupBox->adjustSize();
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

void GeometryWidget::updateRangeLabels(
							std::vector<double> minExt,
							std::vector<double> maxExt) {
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

void GeometryWidget::updateCopyCombo() {
	copyCombo->clear();

	std::vector<string> visNames = _paramsMgr->GetVisualizerNames();
	_visNames.clear();

	_dataSetNames = _paramsMgr->GetDataMgrNames();

	for (int i=0; i<visNames.size(); i++) {
		for (int ii=0; ii<_dataSetNames.size(); ii++) {
	
			// Create a mapping of abreviated visualizer names to their
			// actual string values. 
			//
			string visAbb = "Vis" + std::to_string(i);
			_visNames[visAbb] = visNames[i];
	
			string dataSetName = _dataSetNames[ii];
	
			std::vector<string> typeNames;
			typeNames = _paramsMgr->GetRenderParamsClassNames(visNames[i]);
	
			for (int j=0; j<typeNames.size(); j++) {
	
				// Abbreviate Params names by removing 'Params" from them.
				// Then store them in a map for later reference.
				//
				string typeAbb = typeNames[j];
				int pos = typeAbb.find("Params");
				typeAbb.erase(pos,6);
				_renTypeNames[typeAbb] = typeNames[j];
	
				std::vector<string> renNames;
				renNames = _paramsMgr->GetRenderParamInstances(visNames[i],
																typeNames[j]
				);
	
				for (int k=0; k<renNames.size(); k++) {
					string displayName = visAbb + ":" + 
										dataSetName + ":" +
										typeAbb + ":" + renNames[k];
					QString qDisplayName = QString::fromStdString(displayName);
					copyCombo->addItem(qDisplayName);
				}
			}
		}
	}
}

void GeometryWidget::copyRegion() {

	string copyString = copyCombo->currentText().toStdString();
	std::vector<std::string> elems = split(copyString, ':');
	string visualizer = _visNames[elems[0]];
	string dataSetName = elems[1];
	string renType = _renTypeNames[elems[2]];
	string renderer = elems[3];

	RenderParams* copyParams = _paramsMgr->GetRenderParams(
												visualizer, 
												dataSetName,
												renType, 
												renderer);
	assert(copyParams);

	Box* copyBox = copyParams->GetBox();
	std::vector<double> minExtents, maxExtents;
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

	int ndim = _rParams->GetValueLong(_nDimsTag,3);
	assert(ndim==2 || ndim==3);
	if (ndim==2) {
		_flags = (Flags)(_flags | TWOD);
		_flags = (Flags)(_flags & ~(THREED));
	}
	else {
		_flags = (Flags)(_flags | THREED);
		_flags = (Flags)(_flags & ~(TWOD));
	}

	// Get current domain extents
	//
	size_t ts = _rParams->GetCurrentTimestep();
	int level = _rParams->GetRefinementLevel();
	std::vector<double> minFullExt, maxFullExt;

	if (_flags & VECTOR) {
		std::vector<string> varNames = _rParams->GetFieldVariableNames();
		if (varNames.empty()) return;

		vector<int> axes;
		DataMgrUtils::GetExtents(_dataMgr, ts, varNames, minFullExt, maxFullExt, axes);
	}
	else {
		string varName = _rParams->GetVariableName();
		if (varName.empty()) return;

		int rc = _dataMgr->GetVariableExtents(ts, varName, 
							level, minFullExt, maxFullExt);
		if (rc<0) {
			MyBase::SetErrMsg("Error: DataMgr could "
				"not return valid values from GetVariableExtents()");
		}
	}
	updateRangeLabels(minFullExt, maxFullExt);
	updateCopyCombo();

	// Get current user selected extents
	//
	Box* box = _rParams->GetBox();
	std::vector<double> minExt, maxExt;
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
	connect(copyButton, SIGNAL(released()), this, SLOT(copyRegion()));
}	

void GeometryWidget::setRange(double min, double max) {
	size_t dimension;
	if (QObject::sender() == _xRangeCombo) dimension = 0;
	else if (QObject::sender() == _yRangeCombo) dimension = 1;
	else dimension = 2;

	std::vector<double> minExt, maxExt;
	Box* box = _rParams->GetBox();
	box->GetExtents(minExt, maxExt);

	// Apply the extents that changed into minExt and maxExt
	//
	minExt[dimension] = min;
	maxExt[dimension] = max;

	box->SetExtents(minExt, maxExt);
}
