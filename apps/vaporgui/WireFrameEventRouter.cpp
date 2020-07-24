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
#include "vapor/WireFrameParams.h"
#include "vapor/WireFrameRenderer.h"
#include "VariablesWidget.h"
#include "WireFrameEventRouter.h"
#include "PGroup.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<WireFrameEventRouter> registrar(
    WireFrameEventRouter::GetClassType()
);


WireFrameEventRouter::WireFrameEventRouter( QWidget *parent, ControlExec *ce) 
                    : QTabWidget(parent),
	                    RenderEventRouter( ce, WireFrameParams::GetClassType())
{

    PSection* varSection = new PSection("Variable Selection");
    varSection->Add( new PDimensionSelector() );
    varSection->Add( new PScalarVariableSelectorHLI() );
    PFidelitySection* fidelitySection = new PFidelitySection();
    _pVarGroup = new PGroup;
    _pVarGroup->Add( varSection );
    _pVarGroup->Add( fidelitySection );
    QScrollArea *qsvar = new QScrollArea(this);
    qsvar->setWidgetResizable(true);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsvar->setWidget( _pVarGroup );
    qsvar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );
    addTab( qsvar, "Variables" );

	_appearance = new WireFrameAppearanceSubtab(this);
	QScrollArea* qsapp = new QScrollArea(this);
	qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsapp->setWidget(_appearance);
	qsapp->setWidgetResizable(true);
	addTab(qsapp,"Appearance");

	_geometry = new WireFrameGeometrySubtab(this);
	QScrollArea *qsgeo = new QScrollArea(this);
	qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsgeo->setWidget(_geometry);
	qsgeo->setWidgetResizable(true);
	addTab(qsgeo, "Geometry");

	_annotation= new WireFrameAnnotationSubtab(this);
	QScrollArea *qsAnnotation = new QScrollArea(this);
	qsAnnotation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	qsAnnotation->setWidget(_annotation);
	qsAnnotation->setWidgetResizable(true);
	addTab(qsAnnotation, "Annotation");
}

void WireFrameEventRouter::GetWebHelp(
	vector <pair <string, string> > &help
) const {
	help.clear();

	help.push_back(make_pair(
		"WireFrame Overview",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/WireFrame#WireFrameOverview"
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
		"WireFrame geometry options",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#TwoDGeometry"
	));

	help.push_back(make_pair(
		"WireFrame Appearance settings",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/WireFrame#WireFrameAppearance"
	));

}

void WireFrameEventRouter::_updateTab(){

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

string WireFrameEventRouter::_getDescription() const {
	return(
	"Displays a wireframe of the mesh for the selected variable"
	);
}

