#ifndef RENDERHOLDER_H
#define RENDERHOLDER_H

#include <qobject.h>
#include "qstackedwidget.h"
#include "qpushbutton.h"
#include "qtableview.h"
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_renderselector.h"

QT_USE_NAMESPACE

namespace VAPoR {
	class ControlExec;
	class ParamsMgr;
}

class CBWidget : public QWidget, public QTableWidgetItem {
public:
	CBWidget(QWidget* parent, QString type);
};

//! \class RenderHolder
//! \ingroup Public_GUI
//! \brief A class that manages the display of Renderer parameters
//! \author Alan Norton
//! \version 3.0
//! \date April 2015

//! This is class manages a QTableWidget that indicates the currently 
//! available Renderers, and a
//! QStackedWidget that displays the various parameters associated 
//! with the selected renderer.
//!
class RenderHolder : public QWidget, public Ui_RenderSelector {

	Q_OBJECT

public: 

 //! Constructor:  
 //
 RenderHolder(QWidget *parent, VAPoR::ControlExec *ce);

 virtual ~RenderHolder() {}

 //! Make the tableWidget match the currently displayed RenderParams
 //!
 void Update();

 //! Specify the index of the page to be displayed of the stackedWidget.
 //! \param[in] indx page index
 void SetCurrentIndex(int indx){
	stackedWidget->setCurrentIndex(indx);
	stackedWidget->show();
 }

 //! Add a widget (EventRouter) to the QStackedWidget. 
 //! \param[in] QWidget* Widget to be added
 //! \param[in] name Name of the renderer to be displayed
 //! \param[in] tag indicating type of renderer to be displayed
 //! \return index of widget in the stackedWidget
 int AddWidget(QWidget*, const char* name, string tag);

#ifndef DOXYGEN_SKIP_THIS
private:

 RenderHolder() {}

 GUIStateParams *getStateParams() const {
	assert(_controlExec != NULL);
	return (
		(GUIStateParams *) _controlExec->
		GetParamsMgr()->GetParams(GUIStateParams::GetClassType())
	);
 }

 void updateDupCombo();

 //Convert name to a unique name (among renderer names)
 std::string uniqueName(std::string name);

private slots:
 void newRenderer();
 void deleteRenderer();
 //void changeChecked(int i, int j);
 void itemTextChange(QTableWidgetItem*);
 //void itemChangeHack(QTableWidgetItem*);
 void selectInstance();
 void copyInstanceTo(int);
 void checkboxChanged(int);

signals:
 void newRenderer(string vizName, string renderClass, string renderInst);
 void activeChanged(string vizName, string renderClass, string renderInst);
 
private:
 VAPoR::ControlExec *_controlExec;

 void getRow(
	int row, string &renderInst, string &renderClass, 
	string &dataSetName, bool &enabled
 ) const;

 void getRow(
	int row, string &renderInst, string &renderClass, 
	string &dataSetName
 ) const;

 void getRow(
	string &renderInst, string &renderClass, 
	string &dataSetName, bool &enabled
 ) const;

 void setRow(
	int row, const string &renderInst, 
	const string &renderClass, const string &dataSetName, bool enabled
 );

 void setRow(
	const string &renderInst, const string &renderClass,
	const string &dataSetName, bool enabled
 );

#endif //DOXYGEN_SKIP_THIS
};

#endif //RENDERHOLDER_H 
