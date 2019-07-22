#pragma once
#define OSPRAY_EVENT_ROUTER

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>


QT_USE_NAMESPACE

namespace VAPoR {
	class ControlExec;
}
class ParamsWidget;

class OSPRayEventRouter : public QWidget, public EventRouter {

	Q_OBJECT

public: 
	OSPRayEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
	virtual ~OSPRayEventRouter();

	virtual void GetWebHelp(std::vector <std::pair <string, string>> &help) const;

	//! Ignore wheel event in tab (to avoid confusion)
	virtual void wheelEvent(QWheelEvent*) {}

	// Get static string identifier for this router class
	//
	static string GetClassType() {
		return("OSPRayEventRouter");
	}
	string GetType() const {return GetClassType(); }


protected:
	virtual void _updateTab();
	virtual void _confirmText() {}

	private slots:

private:
    std::vector<ParamsWidget *> _paramsWidgets;
    
    void _blockSignals(bool block);
    void _addParamsWidget(QWidget *parent, ParamsWidget *widget);
};
