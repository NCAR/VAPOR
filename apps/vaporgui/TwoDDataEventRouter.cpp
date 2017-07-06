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

TwoDDataEventRouter::TwoDDataEventRouter(
	QWidget *parent, ControlExec *ce
) : QTabWidget(parent),
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

#ifdef DEAD
//
// Method to be invoked after the user has moved the right or left bounds
// (e.g. From the MapEditor. ) 
// Make the textboxes consistent with the new left/right bounds, but
// don't trigger a new undo/redo event
//
void TwoDDataEventRouter::UpdateMapBounds(){
	RenderParams* rParams = GetActiveParams();
	QString strn;
	string varname = rParams->GetVariableName();
	TransferFunction* tFunc = rParams->GetTransferFunc(varname);
	if (tFunc){
		//_appearance->leftMappingEdit->setText(strn.setNum(tFunc->getMinMapValue(),'g',4));
		//_appearance->rightMappingEdit->setText(strn.setNum(tFunc->getMaxMapValue(),'g',4));
	}   
	setEditorDirty();
}
#endif

void TwoDDataEventRouter::_updateTab(){

	// The variable tab updates itself:
	//
	_variables->Update(
		_controlExec->GetDataMgr(),
		_controlExec->GetParamsMgr(),
		GetActiveParams()
	);

	_appearance->Update(
		_controlExec->getDataStatus(),
		_controlExec->GetParamsMgr(),
		GetActiveParams()
	);
	_geometry->Update(
		_controlExec->GetParamsMgr(),
		_controlExec->GetDataMgr(),
		GetActiveParams()
	);
}

/******************************************************************************
 * Slots associated with TwoDTab.
 * Any EventRouter needs the first two slots here.
 * Plus, each EventRouter needs a slot to respond to events in every non-text
 * widget, e.g. combo boxes, sliders, pushbuttons
 *****************************************************************************/

#ifdef DEAD
//Whenever a lineEdit changes, set the textChanged flag.
void TwoDDataEventRouter::twodLoadTF(){
	TwoDDataParams* rParams = (TwoDDataParams*)GetActiveParams();
	
	loadTF(rParams->GetVariableName());
}

void TwoDDataEventRouter::twodLoadInstalledTF(){
	TwoDDataParams* rParams = (TwoDDataParams*)GetActiveParams();
	string varName = rParams->GetVariableName();
	MapperFunction* tf = rParams->MakeMapperFunc(varName);
	if (!tf) return;
	float minb = tf->getMinMapValue();
	float maxb = tf->getMaxMapValue();
	if (minb >= maxb){ minb = 0.0; maxb = 1.0;}
	loadInstalledTF(rParams->GetVariableName());
	tf = (TransferFunction*) rParams->MakeMapperFunc(varName);
	tf->setMinMaxMapValue(minb, maxb);
	
	setEditorDirty();
}

void TwoDDataEventRouter::twodSaveTF(){
	fileSaveTF();
}
#endif

void TwoDDataEventRouter::startChangeMapFcn(QString){
}

void TwoDDataEventRouter::endChangeMapFcn(){
}
