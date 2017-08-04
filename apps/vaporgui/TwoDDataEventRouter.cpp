#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning( disable : 4100 )
#endif

#include <vapor/glutil.h>
#include <qlineedit.h>
#include <qscrollarea.h>
#include <qcolordialog.h>
#include <QFileDialog>
#include <vector>
#include <string>
#include "vapor/TwoDDataParams.h"
#include "vapor/TwoDDataRenderer.h"
#include "VariablesWidget.h"
#include "TwoDDataEventRouter.h"
#include "EventRouter.h"

using namespace VAPoR;

TwoDDataEventRouter::TwoDDataEventRouter( QWidget *parent, ControlExec *ce) 
                    : QTabWidget(parent),
	                    RenderEventRouter( ce, TwoDDataParams::GetClassType())
{
	_variables = new TwoDVariablesSubtab(this);
	QScrollArea *qsvar = new QScrollArea(this);
	qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_variables->adjustSize();
	qsvar->setWidget(_variables);
	qsvar->setWidgetResizable(true);
	addTab(qsvar, "Variables");

	_appearance = new TwoDAppearanceSubtab(this);
	QScrollArea* qsapp = new QScrollArea(this);
	qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsapp->setWidget(_appearance);
	qsapp->setWidgetResizable(true);
	addTab(qsapp,"Appearance");

	_geometry = new TwoDGeometrySubtab(this);
	QScrollArea *qsgeo = new QScrollArea(this);
	qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsgeo->setWidget(_geometry);
	qsgeo->setWidgetResizable(true);
	addTab(qsgeo, "Geometry");

#ifdef DEAD	
	QScrollArea *qsimg = new QScrollArea(this);
	qsimg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_image = new TwoDDataImageGUI(this);
	qsimg->setWidget(_image);
	addTab(qsimg,"Image");
	_image->adjustSize();

	ImageFrame *imageFrame = _image->imageFrame;
	QGLFormat fmt;
	fmt.setAlpha(true);
	fmt.setRgba(true);
	fmt.setDoubleBuffer(true);
	fmt.setDirectRendering(true);
	_glTwoDDataImageWindow = new GLTwoDDataImageWindow(fmt,imageFrame,"gltwoddatawindow",imageFrame);
	imageFrame->attachRenderWindow(_glTwoDDataImageWindow, this);
#endif
}

TwoDDataEventRouter::~TwoDDataEventRouter(){
	if (_variables) delete _variables;
	if (_geometry) delete _geometry;
	if (_appearance) delete _appearance;
}

void TwoDDataEventRouter::GetWebHelp(
	vector <pair <string, string> > &help
) const {
	help.clear();

	help.push_back(make_pair(
		"TwoD Overview",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#TwoDOverview"
	));

	help.push_back(make_pair(
		"Renderer control",
		"http://www.vapor.ucar.edu/docs/vapor-how-guide/renderer-instances"
	));

	help.push_back(make_pair(
		"Data accuracy control",
		"http://www.vapor.ucar.edu/docs/vapor-how-guide/refinement-and-lod-control"
	));

	help.push_back(make_pair(
		"TwoD geometry options",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#TwoDGeometry"
	));

	help.push_back(make_pair(
		"TwoD Appearance settings",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#HelloAppearance"
	));

}

void TwoDDataEventRouter::_updateTab(){

	// The variable tab updates itself:
	//
	_variables->Update(
		GetActiveDataMgr(),
		_controlExec->GetParamsMgr(),
		GetActiveParams()
	);

	_appearance->Update(
		GetActiveDataMgr(),
		_controlExec->GetParamsMgr(),
		GetActiveParams()
	);
	_geometry->Update(
		_controlExec->GetParamsMgr(),
		GetActiveDataMgr(),
		GetActiveParams()
	);
}
