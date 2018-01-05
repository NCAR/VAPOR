
//															*
//			 Copyright (C)  2014										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//					
//	File:		RenderHolder.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		October 2014
//
//	Description:	Implements the RenderHolder class

#include <cassert>
#include <qcombobox.h>
#include <QStringList>
#include <QTableWidget>
#include <QCheckBox>
#include <QList>
#include <qpushbutton.h>
#include <sstream>
#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include "qdialog.h"
#include "ui_NewRendererDialog.h"
#include "VizSelectCombo.h"
#include "ErrorReporter.h"
#include "RenderEventRouter.h"
#include "RenderHolder.h"
#include <vapor/GetAppPath.h>

using namespace VAPoR;

namespace {
	const string DuplicateInStr = "Duplicate in:";
};

const std::string NewRendererDialog::barbDescription = "Displays an "
	"array of arrows with the users domain, with custom dimensions that are "
	"defined by the user in the X, Y, and Z axes.  The arrows represent a vector "
	"whos direction is determined by up to three user-defined variables.\n\nBarbs "
	"can have a constant color applied to them, or they may be colored according "
	"to an additional user-defined variable.\n\n [hyperlink to online doc]";

const std::string NewRendererDialog::contourDescription = "Displays "
	"a series of user defined contours along a two dimensional plane within the "
	"user's domain.\n\nContours may hae constant coloration, or may be colored "
	"according to a secondary variable.\n\nContours may be displaced by a height "
	"variable.\n\n [hyperlink to online doc]";

const std::string NewRendererDialog::imageDescription = "Displays a "
	"georeferenced image that is automatically reprojected and fit to the user's"
	"data, as long as the data contains georeference metadata.  The image "
	"renderer may be offset by a height variable to show bathymetry or mountainous"
	" terrain.\n\n [hyperlink to online doc]";

const std::string NewRendererDialog::twoDDataDescription = "Displays "
	"the user's 2D data variables along the plane described by the source data "
	"file.\n\nThese 2D variables may be offset by a height variable.\n\n"
	"[hyperlink to online doc]";

CBWidget::CBWidget(QWidget* parent, QString text) : 
	QWidget(parent), QTableWidgetItem(text) {};

NewRendererDialog::NewRendererDialog(QWidget* parent, ControlExec* controlExec) :
	QDialog(parent), Ui_NewRendererDialog() {
	setupUi(this);

	rendererNameEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9_]{1,64}")));
	dataMgrCombo->clear();

	initializeImages();
	initializeDataSources(controlExec);

	connect(barbButton, SIGNAL(toggled(bool)), this, SLOT(barbChecked(bool)));
	connect(contourButton, SIGNAL(toggled(bool)), this, SLOT(contourChecked(bool)));
	connect(imageButton, SIGNAL(toggled(bool)), this, SLOT(imageChecked(bool)));
	connect(twoDDataButton, SIGNAL(toggled(bool)), this, SLOT(twoDDataChecked(bool)));
};

void NewRendererDialog::initializeDataSources(ControlExec *ce) {
	ParamsMgr *paramsMgr = ce->GetParamsMgr();
	vector <string> dataSetNames = paramsMgr->GetDataMgrNames();

	dataMgrCombo->clear();
	for (int i = 0; i<dataSetNames.size(); i++){
		dataMgrCombo->addItem(
			QString::fromStdString(dataSetNames[i])
		);  
	}  
}

void NewRendererDialog::initializeImages() {
	setUpImage("Barbs.png", bigDisplay);
	titleLabel->setText("\nBarb Renderer");
	descriptionLabel->setText(QString::fromStdString(barbDescription));
	_selectedRenderer = "Barb";

	setUpImage("Barbs_small.png", barbLabel);
	setUpImage("Contours_small.png", contourLabel);
	setUpImage("Image_small.png", imageLabel);
	setUpImage("TwoDData_small.png", twoDDataLabel);

}

void NewRendererDialog::setUpImage(std::string imageName, QLabel *label) {
	std::vector<std::string> imagePath = std::vector<std::string>();
	imagePath.push_back("Images");
	imagePath.push_back(imageName);
	QPixmap thumbnail(GetAppPath("VAPOR", "share", imagePath).c_str());
	label->setPixmap(thumbnail);
}

void NewRendererDialog::barbChecked(bool state) {
	uncheckAllButtons();
	barbButton->blockSignals(true);
	barbButton->setChecked(true);
	barbButton->blockSignals(false);
	setUpImage("Barbs.png", bigDisplay);
	titleLabel->setText("\nBarb Renderer");
	descriptionLabel->setText(QString::fromStdString(barbDescription));
	_selectedRenderer = "Barb";
}

void NewRendererDialog::contourChecked(bool state) {
	uncheckAllButtons();
	contourButton->blockSignals(true);
	contourButton->setChecked(true);
	contourButton->blockSignals(false);
	setUpImage("Contours.png", bigDisplay);
	titleLabel->setText("\nContour Renderer");
	descriptionLabel->setText(QString::fromStdString(contourDescription));
	_selectedRenderer = "Contour";
}

void NewRendererDialog::imageChecked(bool state) {
	uncheckAllButtons();
	imageButton->blockSignals(true);
	imageButton->setChecked(true);
	imageButton->blockSignals(false);
	setUpImage("Image.png", bigDisplay);
	titleLabel->setText("\nImage Renderer");
	descriptionLabel->setText(QString::fromStdString(imageDescription));
	_selectedRenderer = "Image";
}

void NewRendererDialog::twoDDataChecked(bool state) {
	uncheckAllButtons();
	twoDDataButton->blockSignals(true);
	twoDDataButton->setChecked(true);
	twoDDataButton->blockSignals(false);
	setUpImage("TwoDData.png", bigDisplay);
	titleLabel->setText("\nTwoDData Renderer");
	descriptionLabel->setText(QString::fromStdString(twoDDataDescription));
	_selectedRenderer = "TwoDData";
}

void NewRendererDialog::uncheckAllButtons() {
	barbButton->blockSignals(true);
	contourButton->blockSignals(true);
	imageButton->blockSignals(true);
	twoDDataButton->blockSignals(true);

	barbButton->setChecked(false);
	contourButton->setChecked(false);
	imageButton->setChecked(false);
	twoDDataButton->setChecked(false);
	
	barbButton->blockSignals(false);
	contourButton->blockSignals(false);
	imageButton->blockSignals(false);
	twoDDataButton->blockSignals(false);
}

RenderHolder::RenderHolder(QWidget* parent, ControlExec *ce)
	: QWidget(parent),Ui_LeftPanel()
{
	setupUi(this);
	_controlExec = ce;
	_newRendererDialog = new NewRendererDialog(this, ce);
	_vaporTable = new VaporTable(tableWidget, false, true);
	_currentRow = 0;

	makeConnections();
	clearStackedWidget();
	initializeSplitter();
}

void RenderHolder::makeConnections() {
	connect(_vaporTable, SIGNAL(cellClicked(int, int)),
		this, SLOT(activeRendererChanged(int, int)));
	connect(_vaporTable, SIGNAL(valueChanged(int, int)),
		this, SLOT(tableValueChanged(int, int)));
	connect(newButton, SIGNAL(clicked()), this, SLOT(showNewRendererDialog()));
	connect(deleteButton, SIGNAL(clicked()),this,SLOT(deleteRenderer()));
	connect(
		dupCombo, SIGNAL(activated(int)),
		this, SLOT(copyInstanceTo(int))
	);
}

void RenderHolder::clearStackedWidget() {
	for (int i = stackedWidget->count()-1; i>=0; i--){
		QWidget* wid = stackedWidget->widget(i);
		stackedWidget->removeWidget(wid);
		delete wid;
	}
}

void RenderHolder::initializeSplitter() {
	QList <int> proportions;
	int topHeight = deleteButton->height() + newButton->height() + newButton->height();
	int bottomHeight = height() - topHeight;
	proportions.append(topHeight);
	proportions.append(bottomHeight);
	mainSplitter->setSizes(proportions);
}

int RenderHolder::AddWidget(QWidget* wid, const char* name, string tag){
	// rc indicates position in the stacked widget.  It will 
	// be needed to change "active" renderer
	//
	int rc = stackedWidget->addWidget(wid);
	stackedWidget->setCurrentIndex(rc);

	return rc;
}

void RenderHolder::initializeNewRendererDialog(vector<string> datasetNames) {
	_newRendererDialog->dataMgrCombo->clear();
	for (int i = 0; i<datasetNames.size(); i++){
		_newRendererDialog->dataMgrCombo->addItem(
			QString::fromStdString(datasetNames[i])
		);
	}
}

void RenderHolder::showNewRendererDialog() {
	ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
	vector <string> dataSetNames = paramsMgr->GetDataMgrNames();
	vector <string> rendererTypees = _controlExec->GetAllRenderClasses();
	
	initializeNewRendererDialog(dataSetNames);
	if (_newRendererDialog->exec() != QDialog::Accepted) {
		return;
	}

	string rendererType = _newRendererDialog->getSelectedRenderer();

	int selection = _newRendererDialog->dataMgrCombo->currentIndex();
	string dataSetName = dataSetNames[selection];

	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();
	
	// figure out the name
	//
	QString qname = _newRendererDialog->rendererNameEdit->text();
	string rendererName = qname.toStdString();

	// Check that it's not all blanks:
	//
	if (rendererName.find_first_not_of(' ') == string::npos) {
		rendererName = rendererType;
	}

	rendererName = uniqueName(rendererName);
	qname = QString(rendererName.c_str());

	paramsMgr->BeginSaveStateGroup("Create new renderer");

	int rc = _controlExec->ActivateRender(
		activeViz, dataSetName, rendererType, rendererName, false
	);
	if (rc<0) {
		paramsMgr->EndSaveStateGroup();
		MSG_ERR("Can't create renderer");
		return;
	}

	// Save current instance to state
	//
	p->SetActiveRenderer(activeViz, rendererType, rendererName);

	Update();

	emit newRendererSignal(activeViz, rendererType, rendererName);

	paramsMgr->EndSaveStateGroup();
}


void RenderHolder::deleteRenderer() {

	// Check if there is anything to delete:
	//
	if (tableWidget->rowCount() == 0) {
		return;
	}

	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	// Get the currently selected renderer.
	//
	string rendererName, rendererType, dataSetName;
	getRow(_currentRow, rendererName, rendererType, dataSetName);

	ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
	paramsMgr->BeginSaveStateGroup("Delete renderer");

	int rc = _controlExec->ActivateRender(
		activeViz, dataSetName, rendererType, rendererName, false
	);
	assert(rc == 0);

	_controlExec->RemoveRenderer(
		activeViz, dataSetName, rendererType, rendererName
	);
	
	// Update will rebuild the TableWidget with the updated state
	//
	p->SetActiveRenderer(activeViz, "", "");
	Update();

	// Make the renderer in the first row the active renderer
	//
	getRow(0, rendererName, rendererType, dataSetName);
	p->SetActiveRenderer(activeViz, rendererType, rendererName);
	emit activeChanged(activeViz, rendererType, rendererName);

	paramsMgr->EndSaveStateGroup();
}

void RenderHolder::activeRendererChanged(int row, int col) {
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();
	string rendererName = _vaporTable->GetValue(row, 0);
	string rendererType = _vaporTable->GetValue(row, 1);
	p->SetActiveRenderer(activeViz, rendererType, rendererName);
	emit activeChanged(activeViz, rendererType, rendererName);
	highlightActiveRow(row);
}

void RenderHolder::highlightActiveRow(int row) {
	QString selectionColor = "{ background: rgb(0, 255, 255);"
		" selection-background-color: rgb(233, 99, 0); }";
	QString normalColor = "{ background: rgb(255,255,255);"
		" selection-background-color: rgb(233, 99, 0); }";

	for (int i=0; i<_vaporTable->rowCount(); i++) {
		for (int j=0; j<_vaporTable->columnCount(); j++) {
			QWidget* cell = _vaporTable->cellWidget(i,j);
			QLineEdit *le = qobject_cast<QLineEdit *>(cell);
			if (le) {
				if (i == row) 
					le->setStyleSheet("QLineEdit " + selectionColor);
				else 
					le->setStyleSheet("QLineEdit " + normalColor);
			}   
			else {
				if (i == row) 
					cell->setStyleSheet("QWidget " + selectionColor);
				else 
					cell->setStyleSheet("QWidget " + normalColor);
			}   
		}   
	}

	_currentRow = row;
}

VAPoR::RenderParams* RenderHolder::getRenderParamsFromCell(int row, int col) {
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();
	string activeRenderClass, activeRenderInst;
 	p->GetActiveRenderer(
		activeViz, activeRenderClass, activeRenderInst
	);
	
	string rendererName = _vaporTable->GetValue(row, 0);
	string rendererType = _vaporTable->GetValue(row, 1);
	string dataSetName = _vaporTable->GetValue(row, 2);
	
	RenderParams *rParams = _controlExec->GetRenderParams(
		activeViz, dataSetName, activeRenderClass, activeRenderInst
	);
	return rParams;
}

void RenderHolder::changeRendererName(int row, int col) {
	RenderParams* rP = getRenderParamsFromCell(row, col);
	
	string text = _vaporTable->GetValue(row, col);
	string uniqueText = uniqueName(text);
	//if (text == rP->GetRendererName()) return;

	//if (uniqueText != text) item->setText(QString(uniqueText.c_str()));

//	rP->SetRendererName(uniqueText);
}

void RenderHolder::tableValueChanged(int row, int col) {
	if (col==0) {
		changeRendererName(row, col);
		return;
	}

	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();
	string rendererName = _vaporTable->GetValue(row, 0);
	string rendererType = _vaporTable->GetValue(row, 1);
	string dataSetName = _vaporTable->GetValue(row, 2);
	int state = _vaporTable->GetValue(row, 3);

	int rc = _controlExec->ActivateRender(
		activeViz, dataSetName, rendererType, rendererName, state
	);
	if (rc<0) {
		MSG_ERR("Can't create renderer");
		return;
	}
}

void RenderHolder::itemTextChange(QTableWidgetItem* item) {
#ifdef DEAD	
	int row = item->row();
	int col = item->column();
	if (col != 0) return;
	int viznum = _controlExec->GetActiveVizIndex();
	
	//avoid responding to item creation:
	if (InstanceParams::GetNumInstances(viznum) <= row) return;
	
	string text = item->text().toStdString();
	RenderParams* rP = InstanceParams::GetRenderParamsInstance(viznum, row);
	if (stdtext == rP->GetRendererName()) return;
	string uniqueText = uniqueName(text);

	if (uniqueText != text) item->setText(QString(uniqueText.c_str()));

	rP->SetRendererName(uniqueText);
#endif	
}

void RenderHolder::copyInstanceTo(int item) {
	if (item == 0) return; // User has selected descriptor item "Duplicate in:"
	
	GUIStateParams *guiStateParams = getStateParams();

	string vizName = dupCombo->itemText(item).toStdString();
	string activeViz = guiStateParams->GetActiveVizName();

	string activeRenderClass, activeRenderInst;
 	guiStateParams->GetActiveRenderer(
		activeViz, activeRenderClass, activeRenderInst
	);

	string dataSetName, dummy1, dummy2;
	bool status = _controlExec->RenderLookup(
		activeRenderInst, dummy1, dataSetName, dummy2
	);
	assert(status);

	RenderParams *rParams = _controlExec->GetRenderParams(
		activeViz, dataSetName, activeRenderClass, activeRenderInst
	);
	assert(rParams);

	string rendererName = uniqueName(activeRenderInst);

	int rc = _controlExec->ActivateRender(
		vizName, dataSetName, rParams, rendererName, false
	);
	if (rc<0) {
		MSG_ERR("Can't create renderer");
		return;
	}

	// Save current instance to state
	//
	guiStateParams->SetActiveRenderer(activeViz, activeRenderClass, rendererName);

	Update();

	emit newRendererSignal(activeViz, activeRenderClass, rendererName);
}

std::string RenderHolder::uniqueName(std::string name) {
	string newname = name;

	
	ParamsMgr *pm = _controlExec->GetParamsMgr();

	vector <string> allInstNames;

	// Get ALL of the renderer instance names defined
	//
	vector <string> vizNames = pm->GetVisualizerNames();
	for (int i = 0; i<vizNames.size(); i++) { 
		vector <string> classNames = _controlExec->GetRenderClassNames(
			vizNames[i]
		);

		for (int j = 0; j<classNames.size(); j++) { 
			vector <string> rendererNames = _controlExec->GetRenderInstances(
				vizNames[i], classNames[j]
			);

			allInstNames.insert(
				allInstNames.begin(), rendererNames.begin(), rendererNames.end()
			);
		}
	}
	
	while(1){
		bool match = false;
		for (int i = 0; i<allInstNames.size(); i++){ 
			string usedName = allInstNames[i];

			if (newname != usedName) continue;

			match = true;

			// found a match.  Modify newname
			// If newname ends with a number, increase the number.  
			// Otherwise just append _1
			//
			size_t lastnonint = newname.find_last_not_of("0123456789");
			if (lastnonint < newname.length()-1){
				//remove terminating int
				string endchars = newname.substr(lastnonint+1);
				//Convert to int:
				int termInt = atoi(endchars.c_str());
				//int termInt = std::stoi(endchars);
				termInt++;
				//convert termInt to a string
				std::stringstream ss;
				ss << termInt;
				endchars = ss.str();	
				newname.replace(lastnonint+1,string::npos, endchars);
			} else {
				newname = newname + "_1";
			}
		}
		if (!match) break;
	}
	return newname;
}

string RenderHolder::getActiveRendererClass() {
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	string activeRenderClass, activeRenderInst;
	p->GetActiveRenderer(
		activeViz, activeRenderClass, activeRenderInst
	);

	return activeRenderClass;
}

string RenderHolder::getActiveRendererInst() {
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	string activeRenderClass, activeRenderInst;
	p->GetActiveRenderer(
		activeViz, activeRenderClass, activeRenderInst
	);

	return activeRenderInst;
}

void RenderHolder::updateDupCombo() {

	ParamsMgr *pm = _controlExec->GetParamsMgr();
	vector <string> vizNames = pm->GetVisualizerNames();

	dupCombo->clear();
	dupCombo->addItem(QString::fromStdString(DuplicateInStr));

	string activeRenderClass = getActiveRendererClass();
	string activeRenderInst = getActiveRendererInst();
	if (activeRenderClass.empty() || activeRenderInst.empty()) return;

	for (int i=0; i<vizNames.size(); i++) {
		dupCombo->addItem(QString::fromStdString(vizNames[i]));
	}

	dupCombo->setCurrentIndex(0);
}

void RenderHolder::makeRendererTableHeaders(vector<string> &tableValues) {
	tableValues.push_back("Name");
	tableValues.push_back("Type");
	tableValues.push_back("Data Set");
	tableValues.push_back("Enabled");
}

void RenderHolder::Update() {
	int rows, cols;
	std::vector<std::string> values;

	// Get active params from GUI state
	//
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	string activeRenderClass, activeRenderInst;
 	p->GetActiveRenderer(
		activeViz, activeRenderClass, activeRenderInst
	);

	// Get ALL of the renderer instance names defined for this visualizer
	//
	map <string, vector <string>> rendererNamesMap;
	vector <string> classNames = _controlExec->GetRenderClassNames(activeViz);
	int numRows = 0;
	for (int i = 0; i<classNames.size(); i++) {
		vector <string> rendererNames = _controlExec->GetRenderInstances(
			activeViz, classNames[i]
		);
		rendererNamesMap[classNames[i]] = rendererNames;
		numRows += rendererNames.size();
	}

	vector<string> tableValues, rowHeader, colHeader;
	makeRendererTableHeaders(colHeader);

	map <string, vector <string>>::iterator itr;
	int selectedRow = -1; 
	int row = 0;
	for (itr = rendererNamesMap.begin(); itr != rendererNamesMap.end(); ++itr) {
		vector <string> rendererNames = itr->second;

		string className = itr->first;

		for (int i=0; i<rendererNames.size(); i++) {

			string rendererName = rendererNames[i];

			string dataSetName, dummy1, dummy2;
			bool status = _controlExec->RenderLookup(
				rendererName, dummy1, dataSetName, dummy2
			);  
			assert(status);

			// Is this the currently selected render instance?
			//  
			if (rendererName==activeRenderInst && className==activeRenderClass) {
				selectedRow = row;
			}   

			RenderParams *rParams = _controlExec->GetRenderParams(
				activeViz, dataSetName, className, rendererName
			);  
			assert(rParams);

			string enabled = rParams->IsEnabled() ? "1" : "0";
			tableValues.push_back(rendererName);
			tableValues.push_back(className);
			tableValues.push_back(dataSetName);
			tableValues.push_back(enabled);

			row++;
		}   
	} 

	_vaporTable->Update(numRows, 4, tableValues, rowHeader, colHeader);
	highlightActiveRow(selectedRow);

	updateDupCombo();

	// If there are no rows, there are no renderers, so we now set
	// the current active renderer to be "empty"
	//
	if (numRows == 0) {
		p->SetActiveRenderer(activeViz, "", "");
		SetCurrentIndex(-1);
		stackedWidget->hide();
		deleteButton->setEnabled(false);
		dupCombo->setEnabled(false);
	}
	else {
		deleteButton->setEnabled(true);
		dupCombo->setEnabled(true);
	}
}

void RenderHolder::getRow(
	int row, string &rendererName, string &rendererType, 
	string &dataSetName
) const {
	rendererName.clear();
	rendererType.clear();

	if (tableWidget->rowCount() == 0) {
		rendererName = "";
		rendererType = "";
		dataSetName = "";
		return;
	}

	if (row == -1) row = _vaporTable->rowCount() - 1;

	rendererName = _vaporTable->GetStringValue(row, 0);
	rendererType = _vaporTable->GetStringValue(row, 1);
	dataSetName = _vaporTable->GetStringValue(row, 2);
}
