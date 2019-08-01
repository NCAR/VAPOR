#include <qdesktopwidget.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include "GL/glew.h"
#include "qcolordialog.h"

#include <qlabel.h>
#include <QGroupBox>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "TabManager.h"

#include "OSPRayEventRouter.h"
#include "vapor/ControlExecutive.h"
#include "EventRouter.h"
#include <vapor/OSPRayParams.h>
#include "ParamsWidgets.h"

using namespace VAPoR;

OSPRayEventRouter::OSPRayEventRouter(
	QWidget *parent, ControlExec *ce
) : QWidget(parent),
	EventRouter(ce, OSPRayParams::GetClassType())
{
    QVBoxLayout *layout = new QVBoxLayout;
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    this->setLayout(layout);
    
    ParamsWidgetGroup *group = new ParamsWidgetGroup("Global Config");
    
    _addParamsWidget(group, (new ParamsWidgetNumber(OSPRayParams::_aoSamplesTag, "Ambient Occlusion Samples"))->SetRange(0, 1000));
    _addParamsWidget(group, (new ParamsWidgetNumber(OSPRayParams::_samplesPerPixelTag))->SetRange(1, 100));
    _addParamsWidget(group, (new ParamsWidgetFloat(OSPRayParams::_ambientIntensity))->SetRange(0, 1));
    _addParamsWidget(group, (new ParamsWidgetFloat(OSPRayParams::_spotlightIntensity))->SetRange(0, 1));
    
    layout->addWidget(group);
    layout->addStretch();
}


OSPRayEventRouter::~OSPRayEventRouter(){
}

void OSPRayEventRouter::_blockSignals(bool block) {
	QList<QWidget *> widgetList = this->findChildren<QWidget *>();
	QList<QWidget *>::const_iterator widgetIter	(widgetList.begin());
	QList<QWidget *>::const_iterator lastWidget	(widgetList.end());
 
	while ( widgetIter !=  lastWidget) {
		(*widgetIter)->blockSignals(block);
		++widgetIter;
	}
}

void OSPRayEventRouter::_addParamsWidget(QWidget *parent, ParamsWidget *widget)
{
    parent->layout()->addWidget(widget);
    _paramsWidgets.push_back(widget);
}

//Insert values from params into tab panel
//
void OSPRayEventRouter::_updateTab() {
	_blockSignals(true);
    
    OSPRayParams *params = _controlExec->GetParamsMgr()->GetOSPRayParams();
    
    const auto end = _paramsWidgets.end();
    for (auto it = _paramsWidgets.begin(); it != end; ++it)
        (*it)->Update(params);
    
	_blockSignals(false);
}

void OSPRayEventRouter::GetWebHelp(
	vector <pair <string, string> > &help
) const {
	help.clear();

	help.push_back(make_pair(
		"Overview of the Settings tab",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/startup-tab#SettingsOverview"
	));
}
