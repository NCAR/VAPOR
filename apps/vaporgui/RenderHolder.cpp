
//															*
//		     Copyright (C)  2014										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
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
#include <qpushbutton.h>
#include <sstream>
#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include "qdialog.h"
#include "ui_newRendererDialog.h"
#include "VizSelectCombo.h"
#include "MessageReporter.h"
#include "RenderEventRouter.h"
#include "RenderHolder.h"

using namespace VAPoR;

namespace {
	const string DuplicateInStr = "Duplicate in:";
};

RenderHolder::RenderHolder(QWidget* parent, ControlExec *ce)
	: QWidget(parent),Ui_RenderSelector()
{
	setupUi(this);
	_controlExec = ce;

	tableWidget->setColumnCount(4);
	QStringList headerText;
	headerText << " Name " << " Type " << " Data Set " << "Enabled";
	tableWidget->setHorizontalHeaderLabels(headerText);
	tableWidget->verticalHeader()->hide();
	tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableWidget->setFocusPolicy(Qt::ClickFocus);
	
	tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);
	tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	connect(newButton, SIGNAL(clicked()), this, SLOT(newRenderer()));
	connect(deleteButton, SIGNAL(clicked()),this,SLOT(deleteRenderer()));
	connect(
		dupCombo, SIGNAL(activated(int)),
		this, SLOT(copyInstanceTo(int))
	);
	connect(
		tableWidget,SIGNAL(cellChanged(int,int)), this, 
		SLOT(changeChecked(int,int))
	);
	connect(
		tableWidget, SIGNAL(itemSelectionChanged()), 
		this, SLOT(selectInstance())
	);
	connect(
		tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
		this, SLOT(itemTextChange(QTableWidgetItem*))
	);

	connect(
		tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
		this, SLOT(itemChangeHack(QTableWidgetItem*))
	);
	
	//Remove any existing widgets:
	for (int i = stackedWidget->count()-1; i>=0; i--){
		QWidget* wid = stackedWidget->widget(i);
		stackedWidget->removeWidget(wid);
		delete wid;
	}
	
}

int RenderHolder::AddWidget(QWidget* wid, const char* name, string tag){

	// rc indicates position in the stacked widget.  It will 
	// be needed to change "active" renderer
	//
	int rc = stackedWidget->addWidget(wid);
	stackedWidget->setCurrentIndex(rc);

	return rc;
}

// 
// Slots: 
//
void RenderHolder::newRenderer() {
	ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
	vector <string> dataSetNames = paramsMgr->GetDataMgrNames();


	vector <string> renderClasses = _controlExec->GetAllRenderClasses();

	// Launch a dialog to select a renderer type, visualizer, name
	// Then insert a horizontal line with text and checkbox.
	// The new line becomes selected.
	
	QDialog nDialog(this);
	Ui_NewRendererDialog rDialog;
	rDialog.setupUi(&nDialog);
	rDialog.rendererNameEdit->setText(" ");

	// Set up the list of data set names in the dialog:
	//
	rDialog.dataMgrCombo->clear();
	for (int i = 0; i<dataSetNames.size(); i++){
		rDialog.dataMgrCombo->addItem(
			QString::fromStdString(dataSetNames[i])
		);
	}

	// Set up the list of renderer names in the dialog:
	//
	rDialog.rendererCombo->clear();
	for (int i = 0; i<renderClasses.size(); i++){
		rDialog.rendererCombo->addItem(
			QString::fromStdString(renderClasses[i])
		);
	}
	if (nDialog.exec() != QDialog::Accepted) return;
	
	int selection = rDialog.rendererCombo->currentIndex();
	string renderClass = renderClasses[selection];

	selection = rDialog.dataMgrCombo->currentIndex();
	string dataSetName = dataSetNames[selection];

	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();
	
	// figure out the name
	//
	QString qname = rDialog.rendererNameEdit->text();
	string renderInst = qname.toStdString();

	// Check that it's not all blanks:
	//
	if (renderInst.find_first_not_of(' ') == string::npos) {
		renderInst = renderClass;
	}

	renderInst = uniqueName(renderInst);
	qname = QString(renderInst.c_str());

	int rc = _controlExec->ActivateRender(
		activeViz, dataSetName, renderClass, renderInst, false
	);
	if (rc<0) {
		MessageReporter::errorMsg(
			"Can't create renderer class %s", renderClass.c_str()
		);
		return;
	}

	// Save current instance to state
	//
	p->SetActiveRenderer(activeViz, renderClass, renderInst);

	Update();

	emit newRenderer(activeViz, renderClass, renderInst);

}

void RenderHolder::deleteRenderer() {

	// Check if there is anything to delete:
	//
	if (tableWidget->rowCount() == 0) return;

	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	// Get the currently selected renderer.
	//
	string renderInst, renderClass, dataSetName;
	bool enabled;
	getRow(renderInst, renderClass, dataSetName, enabled);

	int rc = _controlExec->ActivateRender(
		activeViz, dataSetName, renderClass, renderInst, false
	);
	assert(rc == 0);

	_controlExec->RemoveRenderer(
		activeViz, dataSetName, renderClass, renderInst
	);

	// Make the renderer in the first row the active renderer
	//
	getRow(0, renderInst, renderClass, dataSetName, enabled);
	p->SetActiveRenderer(activeViz, renderClass, renderInst);

	// Update will rebuild the TableWidget with the updated state
	//
	Update();
}

//Respond to check/uncheck enabled checkbox
//
void RenderHolder::changeChecked(int row, int col) {

	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	if ( col != 3) return;

	// Get the currently selected renderer.
	//
	string renderInst, renderClass, dataSetName;
	bool enabled;
	getRow(renderInst, renderClass, dataSetName, enabled);

	// Save current instance to state
	//
	p->SetActiveRenderer(activeViz, renderClass, renderInst);

	int rc = _controlExec->ActivateRender(
		activeViz, dataSetName, renderClass, renderInst, enabled
	);
	if (rc<0) {
		MessageReporter::errorMsg(
			"Can't create renderer class %s", renderClass.c_str()
		);
		return;
	}
	
}

void RenderHolder::selectInstanceHelper(
	string activeViz, string renderClass, string renderInst
) {

	// Save current instance to state
	//

	GUIStateParams *p = getStateParams();
	p->SetActiveRenderer(activeViz, renderClass, renderInst);

	emit activeChanged(activeViz, renderClass, renderInst);

}

void RenderHolder::selectInstance() {
	
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	// Get the currently selected renderer.
	//
	string renderInst, renderClass, dataSetName;
	bool enabled;
	getRow(renderInst, renderClass, dataSetName, enabled);

	selectInstanceHelper(activeViz, renderClass, renderInst);

}

// It is possible to select the check box without changing the currently
// selected row/column in the table widget. This is probably a bug 
// in Qt. This slot works around the problem by forcing the two to agree
//
void RenderHolder::itemChangeHack(QTableWidgetItem* item) {
	int itemRow = item->row();
	int tableRow = tableWidget->currentRow();

	if (itemRow != tableRow) {
		tableWidget->setCurrentItem(item);
	}
}

void RenderHolder::itemTextChange(QTableWidgetItem* item) {
	
	
#ifdef	DEAD
	int row = item->row();
	int col = item->column();
	if (col != 0) return;
	int viznum = _controlExec->GetActiveVizIndex();
	//avoid responding to item creation:
	if (InstanceParams::GetNumInstances(viznum) <= row) return;
	
	QString newtext = item->text();
	string stdtext = newtext.toStdString();
	RenderParams* rP = InstanceParams::GetRenderParamsInstance(viznum, row);
	if (stdtext == rP->GetRendererName()) return;
	string stdtext1 = uniqueName(stdtext);

	if (stdtext1 != stdtext) item->setText(QString(stdtext1.c_str()));

	rP->SetRendererName(stdtext1);
#endif
	
}

void RenderHolder::copyInstanceTo(int item) {

	// Get target viz name to copy to
	//
	QString qS = dupCombo->itemText(item);
	if (qS.toStdString() == DuplicateInStr) return;
	string dstVizName = qS.toStdString();

	// Get active params from GUI state. Need  active (source) vizName
	//
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	string activeRenderClass, activeRenderInst;
 	p->GetActiveRenderer(
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

	// figure out the name
	//
	string renderInst = uniqueName(activeRenderInst);

	int rc = _controlExec->ActivateRender(
		dstVizName, dataSetName, rParams, renderInst, false
	);
	if (rc<0) {
		MessageReporter::errorMsg(
			"Can't create renderer class %s", activeRenderClass.c_str()
		);
		return;
	}

	// Save current instance to state
	//
	p->SetActiveRenderer(activeViz, activeRenderClass, renderInst);

	Update();

	emit newRenderer(activeViz, activeRenderClass, renderInst);

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
			vector <string> renderInsts = _controlExec->GetRenderInstances(
				vizNames[i], classNames[j]
			);

			allInstNames.insert(
				allInstNames.begin(), renderInsts.begin(), renderInsts.end()
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
				//endchars = std::to_string((unsigned long long)termInt);
				newname.replace(lastnonint+1,string::npos, endchars);
			} else {
				newname = newname + "_1";
			}
		}
		if (!match) break;
	}
	return newname;
}


void RenderHolder::updateDupCombo() {

	ParamsMgr *pm = _controlExec->GetParamsMgr();

	// Get ALL of the defined visualizer names
	//
	vector <string> vizNames = pm->GetVisualizerNames();

	dupCombo->clear();

	// Get active params from GUI state
	//
	GUIStateParams *p = getStateParams();

	dupCombo->addItem(QString::fromStdString(DuplicateInStr));


	// Make sure active renderers available in current visualizer. Otherwise
	// punt.
	//
	string activeViz = p->GetActiveVizName();

	string activeRenderClass, activeRenderInst;
	p->GetActiveRenderer(
		activeViz, activeRenderClass, activeRenderInst
	);
	if (activeRenderClass.empty() || activeRenderInst.empty()) return;

	for (int i=0; i<vizNames.size(); i++) {

		dupCombo->addItem(QString::fromStdString(vizNames[i]));
	}

	dupCombo->setCurrentIndex(0);
		
}

void RenderHolder::Update() {

	// Get active params from GUI state
	//
	GUIStateParams *p = getStateParams();
	string activeViz = p->GetActiveVizName();

	string activeRenderClass, activeRenderInst;
 	p->GetActiveRenderer(
		activeViz, activeRenderClass, activeRenderInst
	);

	// Disable signals. Is this needed?
	//
	bool oldState = tableWidget->blockSignals(true);

	// Rebuild everything from scratch
	//
	tableWidget->clearContents();

	// Get ALL of the renderer instance names defined for this visualizer
	//
	map <string, vector <string>> renderInstsMap;
	vector <string> classNames = _controlExec->GetRenderClassNames(activeViz);
	int numRows = 0;
	for (int i = 0; i<classNames.size(); i++) {
		vector <string> renderInsts = _controlExec->GetRenderInstances(
			activeViz, classNames[i]
		);
		renderInstsMap[classNames[i]] = renderInsts;
		numRows += renderInsts.size();
	}

	// Add one row in tableWidget for each RenderParams that is associated 
	// with activeViz:
	
	tableWidget->setRowCount(numRows);
	map <string, vector <string>>::iterator itr;
	int selectedRow = -1;
	int row = 0;
	for (itr = renderInstsMap.begin(); itr != renderInstsMap.end(); ++itr) {
		vector <string> renderInsts = itr->second;

		string className = itr->first;

		for (int i=0; i<renderInsts.size(); i++) {

			string renderInst = renderInsts[i];

			string dataSetName, dummy1, dummy2;
			bool status = _controlExec->RenderLookup(
				renderInst, dummy1, dataSetName, dummy2
			);
			assert(status);

			// Is this the currently selected render instance?
			//
			if (renderInst==activeRenderInst && className==activeRenderClass) {
				selectedRow = row;
			}

			RenderParams *rParams = _controlExec->GetRenderParams(
				activeViz, dataSetName, className, renderInst
			);
			assert(rParams);

			setRow(row, renderInst, className,dataSetName,rParams->IsEnabled());

			row++;
		}
	}

	// Renable signals before calling tableWidget::selectRow(), which will
	// trigger a itemSelectionChanged signal
	//
	tableWidget->blockSignals(oldState);

	if (numRows > 0 && selectedRow >= 0) {
		tableWidget->selectRow(selectedRow);
	}

	updateDupCombo();
}

void RenderHolder::getRow(
	int row, string &renderInst, string &renderClass, 
	string &dataSetName, bool &enabled
) const {
	renderInst.clear();
	renderClass.clear();
	enabled = false;

	if (tableWidget->rowCount() == 0) return;

	QTableWidgetItem *item = tableWidget->item(row, 0);
	renderInst = item->text().toStdString();

	item = tableWidget->item(row, 1);
	renderClass = item->text().toStdString();

	item = tableWidget->item(row, 2);
	dataSetName = item->text().toStdString();

	item = tableWidget->item(row,3);
	enabled = item->checkState() == Qt::Checked;
}

void RenderHolder::getRow(
	string &renderInst, string &renderClass, string &dataSetName, bool &enabled
) const {

	int row = tableWidget->currentRow();

	getRow(row, renderInst, renderClass, dataSetName, enabled);
}

void RenderHolder::setRow(
	int row, const string &renderInst, const string &renderClass, 
	const string &dataSetName, bool enabled
) {

	int rowCount = tableWidget->rowCount();
	if (row >= rowCount) {
		tableWidget->setRowCount(rowCount+1);
	}

	QTableWidgetItem *item = new QTableWidgetItem(renderInst.c_str());
	tableWidget->setItem(row, 0, item);

	item = new QTableWidgetItem(renderClass.c_str());
	tableWidget->setItem(row, 1, item);

	item = new QTableWidgetItem(dataSetName.c_str());
	tableWidget->setItem(row, 2, item);

	item = new QTableWidgetItem(" ");
	tableWidget->setItem(row, 3, item);

	if (enabled) {
		item->setCheckState(Qt::Checked);
	}
	else {
		item->setCheckState(Qt::Unchecked);
	}
}

void RenderHolder::setRow(
	const string &renderInst, const string &renderClass, 
	const string &dataSetName, bool enabled
) {

	int row = tableWidget->currentRow();

	setRow(row, renderInst, renderClass, dataSetName, enabled);
}
