#ifdef WIN32
#pragma warning( disable : 4100 )
#endif

#include <vapor/glutil.h>
#include <vapor/ImageParams.h>
#include <ImageEventRouter.h>
#include <EventRouter.h>
#include <qlineedit.h>
#include <QFileDialog>
#include <qscrollarea.h>

#include "PVariableWidgets.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<ImageEventRouter> registrar(
    ImageEventRouter::GetClassType()
); 

ImageEventRouter::ImageEventRouter( QWidget* parent, ControlExec* ce) 
                : QTabWidget(parent),
	                RenderEventRouter( ce, ImageParams::GetClassType() )
{

  sizePolicy().setVerticalPolicy(QSizePolicy::Maximum);

  _variablesGroup->AddVar( new PHeightVariableSelectorHLI );
  addTab( _variablesGroup->GetScrollArea(), "Variables" );

  _appearance = new ImageAppearanceSubtab(this);
  QScrollArea* qsapp = new QScrollArea(this);
  qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  qsapp->setWidget(_appearance);
  qsapp->setWidgetResizable(true);
  addTab(qsapp,"Appearance");

  _geometry = new ImageGeometrySubtab(this);
  QScrollArea *qsgeo = new QScrollArea(this);
  qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  qsgeo->setWidget(_geometry);
  qsgeo->setWidgetResizable(true);
  addTab(qsgeo, "Geometry");
  
}

void ImageEventRouter::GetWebHelp( vector <pair <string, string> > &help) const 
{
	help.clear();
}

void ImageEventRouter::_updateTab()
{
  _variablesGroup->Update(
      GetActiveParams(),
      _controlExec->GetParamsMgr(),
      GetActiveDataMgr()
  );
  
  _appearance->Update( GetActiveDataMgr(),
                       _controlExec->GetParamsMgr(),
                       GetActiveParams());
  _geometry->Update( _controlExec->GetParamsMgr(),
                     GetActiveDataMgr(),
                     GetActiveParams());
}


string ImageEventRouter::_getDescription() const {
	return(
	"Displays a "
    "georeferenced image that is automatically reprojected and fit to the user's"
    "data, as long as the data contains georeference metadata.  The image "
    "renderer may be offset by a height variable to show bathymetry or mountainous"
    " terrain.\n\n "
	);
}

