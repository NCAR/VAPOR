//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//					
//	File:		VizSelectCombo.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2004
//
//	Description:	Defines the VizSelectCombo class
//		This fits in the main toolbar, enables the user to select
//		the visualizer to activate.  
//
#ifndef VIZSELECTCOMBO_H
#define VIZSELECTCOMBO_H
#include <qcombobox.h>

class QToolBar;

namespace VAPoR {
	class ControlExec;
}

class VizSelectCombo : public QComboBox{
	
	Q_OBJECT

public:
	VizSelectCombo(QWidget* parent);
	int getNumWindows() {return (count() - 1);}
	QString getWinName(int i){ return itemText(i);}

public slots:
	// Remove a window from the combobox
	//
	void removeWindow(const QString& winName);
private:
	
	
protected slots:
	// Add a new window to the combobox
	// Use number to maintain order.
	void addWindow(const QString &winName);
	
	void setWindowActive(const QString &winName);

	// activated window , and notify the vizWinManager
	void activeWin(const QString& winName);

signals:
	//send message that a window number was activated.
	//This results from "activated(index)" signal of parent

	void winActivated(const QString &winName);
	void newWin();

};
#endif //VIZSELECTCOMBO_H

