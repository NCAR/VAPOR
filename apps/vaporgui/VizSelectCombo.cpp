//************************************************************************
//															*
//		     Copyright (C)  2004										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//					
//	File:		VizSelectCombo.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2004
//
//	Description:	Implements the VizSelectCombo class
//		This fits in the main toolbar, enables the user to select
//		the visualizer to activate.  
//
#include <string>
#include <cassert>
#include <qcombobox.h>
#include <QToolBar>
#include <qtooltip.h>
#include "VizSelectCombo.h"

namespace {
	const std::string CreateNewStr = "Create New Visualizer";
};

VizSelectCombo::VizSelectCombo(
	QWidget* parent
) : QComboBox(parent) {

	setEditable(false);

	insertItem(0,QString::fromStdString(CreateNewStr));


	//This initially has just the CreateNewStr entry.
	//Hookup signals and slots:
	connect (
		this, SIGNAL(activated(const QString &)), 
		this, SLOT(activeWin(const QString &))
	);

	setMinimumWidth(150);
	setToolTip("Select Active Visualizer or create new one");
}

void VizSelectCombo::AddWindow(const QString& winName) {

	//First look to find the right place; insert it in a gap if necessary.

	assert(count() >= 1);	// Last item is always CreateNewStr

	// Insert in alphabetical order, but leave CreateNewStr in
	// last position
	//
	int index = count() - 1;	// Default is replace CreateNewStr
	for (int i = 0; i<count()-1; i++){
		if (itemText(i) > winName) {
			index = i;
			break;
		} 
	}

	//Insert name at the specified place:
	//
	insertItem(index, winName);
	SetWindowActive(winName);
}

// 
// Remove specified window from the combobox
//

void VizSelectCombo::RemoveWindow(const QString & winName){

	int index = -1;
	for (int i=0; i<count(); i++) {
		if (itemText(i) == winName) {
			index = i;
			break;
		}
	}
	if (index < 0) return;
	
	removeItem(index);

	// Make first window active
	//
	if (count()) {
		SetWindowActive(itemText(0));
	}
}

// 
// Select a window when it's been made active
//
void VizSelectCombo::SetWindowActive(const QString & winName){

	int index = -1;
	for (int i=0; i<count()-1; i++) {
		if (winName == itemText(i)) {
			index = i;
			break;
		}
	}
	if (index < 0) return;

	// Avoid generating an event unless there really is an change.
	//
	if (currentIndex() != index){
		setCurrentIndex(index);
		emit (winActivated(winName));
	}
}


//
//Convert the active index to the active winNum,
// or launch a visualizer
//
void VizSelectCombo::activeWin(const QString &qS){
	
	// If they clicked the end, just create a new visualizer:
	//
	if (qS.toStdString() == CreateNewStr) {
		emit (newWin());
		return;
	}

	// Otherwise notify that a new window has been selected
	//
	emit (winActivated(qS));
}
