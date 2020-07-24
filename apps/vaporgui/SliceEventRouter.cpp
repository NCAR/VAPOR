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
#include "vapor/SliceParams.h"
#include "vapor/SliceRenderer.h"
#include "VariablesWidget.h"
#include "SliceEventRouter.h"
#include "EventRouter.h"
#include "PGroup.h"
#include "PSection.h"
#include "PVariableWidgets.h"
#include "PFidelitySection.h"

#define MIN_QUALITY 1
#define MAX_QUALITY 10
#define SAMPLES_PER_QUALITY 200

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<SliceEventRouter> registrar(
    SliceEventRouter::GetClassType()
);


SliceEventRouter::SliceEventRouter( QWidget *parent, ControlExec *ce) 
                    : QTabWidget(parent),
	                    RenderEventRouter( ce, SliceParams::GetClassType())
{
    PSection* varSection = new PSection("Variable Selection");
    varSection->Add( new PScalarVariableSelector3DHLI() );
    _pVarGroup = new PGroup;
    _pVarGroup->Add( varSection );
    _pVarGroup->Add( new PFidelitySection);
	QScrollArea *qsvar = new QScrollArea(this);
	qsvar->setWidgetResizable(true);
	qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsvar->setWidget( _pVarGroup );
    qsvar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );
    addTab( qsvar, "Variables" );
     

	_appearance = new SliceAppearanceSubtab(this);
	QScrollArea* qsapp = new QScrollArea(this);
	qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsapp->setWidget(_appearance);
	qsapp->setWidgetResizable(true);
	addTab(qsapp,"Appearance");

	_geometry = new SliceGeometrySubtab(this);
	QScrollArea *qsgeo = new QScrollArea(this);
	qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsgeo->setWidget(_geometry);
	qsgeo->setWidgetResizable(true);
	addTab(qsgeo, "Geometry");

	_annotation = new SliceAnnotationSubtab(this);
	QScrollArea *qsAnnotation = new QScrollArea(this);
	qsAnnotation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsAnnotation->setWidget(_annotation);
	qsAnnotation->setWidgetResizable(true);
	addTab(qsAnnotation, "Annotations");

#if 0	
	QScrollArea *qsimg = new QScrollArea(this);
	qsimg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_image = new SliceImageGUI(this);
	qsimg->setWidget(_image);
	addTab(qsimg,"Image");
	_image->adjustSize();

	ImageFrame *imageFrame = _image->imageFrame;
	QGLFormat fmt;
	fmt.setAlpha(true);
	fmt.setRgba(true);
	fmt.setDoubleBuffer(true);
	fmt.setDirectRendering(true);
	_glSliceImageWindow = new GLSliceImageWindow(fmt,imageFrame,"gltwoddatawindow",imageFrame);
	imageFrame->attachRenderWindow(_glSliceImageWindow, this);
#endif
}

void SliceEventRouter::_setDefaultSampleRate() {
    VAPoR::SliceParams* sParams = dynamic_cast<VAPoR::SliceParams*>( GetActiveParams() );
    int defaultRate  = sParams->GetDefaultSampleRate();
    int quality      = defaultRate / SAMPLES_PER_QUALITY;
    if (quality < 1) quality = 1;
    int adjustedRate = quality * SAMPLES_PER_QUALITY;
    sParams->SetSampleRate(adjustedRate);
}

void SliceEventRouter::GetWebHelp(
	vector <pair <string, string> > &help
) const {
	help.clear();

	help.push_back(make_pair(
		"Slice Overview",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#SliceOverview"
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
		"Slice geometry options",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#SliceGeometry"
	));

	help.push_back(make_pair(
		"Slice Appearance settings",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#HelloAppearance"
	));

}

void SliceEventRouter::_updateTab(){

	_pVarGroup->Update(
		GetActiveParams(),
		_controlExec->GetParamsMgr(),
		GetActiveDataMgr()
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
	_annotation->Update(
		_controlExec->GetParamsMgr(),
		GetActiveDataMgr(),
		GetActiveParams()
	);
}

string SliceEventRouter::_getDescription() const {
	return(
	"Displays an axis-aligned slice or cutting plane through"
	"a 3D variable.  Slices are sampled along the plane's axes according"
    " to a sampling rate define by the user.\n\n"
	);
}

