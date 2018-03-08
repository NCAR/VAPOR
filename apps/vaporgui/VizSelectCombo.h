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

class VizSelectCombo : public QComboBox{
	
	Q_OBJECT

public:
 VizSelectCombo(QWidget* parent);
 int GetNumWindows() {return (count() - 1);}
 QString GetWinName(int i){ return itemText(i);}

public slots:
 // Remove a window from the combobox
 //
 void RemoveWindow(const QString& winName);

 void SetWindowActive(const QString &winName);

 // Add a new window to the combobox
 //
 void AddWindow(const QString &winName);

	
private slots:
	
 // activated window , and notify the vizWinManager
 void activeWin(const QString& winName);


signals:

 // send message that a window number was activated.
 //
 void winActivated(const QString &winName);

 // send request for new window
 //
 void newWin();

};
#endif //VIZSELECTCOMBO_H

