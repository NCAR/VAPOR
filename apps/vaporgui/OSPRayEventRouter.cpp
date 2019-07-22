#include <qdesktopwidget.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include "GL/glew.h"
#include "qcolordialog.h"

#include <qlabel.h>
#include <QScrollArea>
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
    QVBoxLayout *GLayout = new QVBoxLayout;
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    this->setLayout(GLayout);
    
    QWidget *item = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    item->setLayout(layout);
    
    QLabel *label = new QLabel("Label 1");
    QSpacerItem *spacer = new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QComboBox *box = new QComboBox();
    box->addItem("1");
    box->addItem("2");
    box->addItem("3");
    
    //connect(box, SIGNAL(stateChanged(int)), this, SLOT(on_memoryCheckbox_stateChanged(int)));
    
    layout->addWidget(label);
    layout->addItem(spacer);
    layout->addWidget(box);
    
    this->layout()->addWidget(item);
    
    _addParamsWidget(this, new ParamsWidgetCheckbox("test", "Test Label"));
    _addParamsWidget(this, new ParamsWidgetNumber(OSPRayParams::_aoSamplesTag, "Ambient Occlusion Samples"));
    _addParamsWidget(this, new ParamsWidgetNumber(OSPRayParams::_samplesPerPixelTag));
    
    
    
    GLayout->addStretch();
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
