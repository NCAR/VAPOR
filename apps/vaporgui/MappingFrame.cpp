//--MappingFrame.cpp -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// A QLGWidget that provides 1D transfer function interface. This frame can
// be used to map color and/or opacity to data values.
// 
// Note: The interface can map either opacity, color, or both opacity and 
// color. The enabledness of these mappings needs to be set BEFORE 
// initializeGL is called. Since QtDesigner does not provide a way to pass
// in constructor arguments, these are being set as properties in designer.
// If designer is not being used call setOpacityMapping and setColorMapping
// immediately after construction. 
//
//----------------------------------------------------------------------------

#include <vapor/glutil.h>	// Must be included first!!!
#include <iostream>
#include <cassert>
#include <cmath>
#include <qcursor.h>
#include <QMenu>
#include <qaction.h>
#include <qcursor.h>
#include <qlabel.h>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QToolTip>

#include <vapor/ControlExecutive.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/MapperFunction.h>
#include <vapor/OpacityMap.h>
#include <vapor/ContourParams.h>
#include "OpacityWidget.h"
#include "DomainWidget.h"
#include "GLColorbarWidget.h"
#include "ControlPointEditor.h"
#include "ErrorReporter.h"
#include "Histo.h"
#include "MappingFrame.h"

#ifndef MAX
#define MAX(a,b)        ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#endif

using namespace VAPoR;
using namespace std;

namespace {


void oglPushState() {
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void oglPopState() {
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();
}

};

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
MappingFrame::MappingFrame(QWidget* parent)
  : QGLWidget(parent),
    _NUM_BINS(256),
    _mapper(NULL),
    _histogram(NULL),
    _opacityMappingEnabled(false),
    _colorMappingEnabled(false),
	_isoSliderEnabled(false),
	_isolineSlidersEnabled(false),
	_lastSelectedIndex(-1),
	navigateButton(NULL),
	_editButton(NULL),
    _variableName(""),
    _domainSlider(new DomainWidget(this)),
	_isoSlider(new IsoSlider(this)),
    _colorbarWidget(new GLColorbarWidget(this, NULL)),
    _lastSelected(NULL),
    _texid(0),
    _texture(NULL),
    _updateTexture(true),
    _histogramScale(LINEAR),
    _contextMenu(NULL),
    _addOpacityWidgetSubMenu(NULL),
    _histogramScalingSubMenu(NULL),
    _compTypeSubMenu(NULL),
    _widgetEnabledSubMenu(NULL),
    _deleteOpacityWidgetAction(NULL),
    _addColorControlPointAction(NULL),
    _addOpacityControlPointAction(NULL),
    _deleteControlPointAction(NULL),
    _lastx(0),
    _lasty(0),
    _editMode(true),
    _clickedPos(0,0),
    _minValueStart(0.0),
    _maxValueStart(1.0),
	_isoVal(0.0),
    _button(Qt::LeftButton),
    _minX(-0.035),
    _maxX(1.035),
    _minY(-0.35),
    _maxY(1.3),
    _minValue(0.0),
    _maxValue(1.0),
    _colorbarHeight(16),
    _domainBarHeight(16),
    _domainLabelHeight(10),
    _domainHeight(_domainBarHeight + _domainLabelHeight + 3),
    _axisRegionHeight(20),
    _opacityGap(4),
    _bottomGap(10),
	_dataMgr(NULL),
	_rParams(NULL)
{
  initWidgets();
  initConnections();
  setMouseTracking(true);
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
MappingFrame::~MappingFrame()
{
	for (int i = 0; i<_isolineSliders.size(); i++) delete _isolineSliders[i];
  makeCurrent();

  //
  // Clean up GLWidgets
  //
  deleteOpacityWidgets();

  delete _domainSlider;
  _domainSlider = NULL;

  delete _colorbarWidget;
  _colorbarWidget = NULL;

  //
  // Clean up texture information
  //
  if (_texture)
  {
    glDeleteTextures(1, &_texid);

    delete [] _texture;
    _texture = NULL;
  }
  for (int i = 0; i<_axisTexts.size(); i++){
	  delete _axisTextPos[i];
  }
  _axisTexts.clear();
  _axisTextPos.clear();
}

void MappingFrame::RefreshHistogram() {
	string var = _rParams->GetColorMapVariableName();
	if (var == "") {
		var = _rParams->GetVariableName();
	}
//	string var = _rParams->GetVariableName();
	size_t ts = _rParams->GetCurrentTimestep();	

	float minRange = _rParams->MakeMapperFunc(var)->getMinMapValue();
	float maxRange = _rParams->MakeMapperFunc(var)->getMaxMapValue();
	if (!_histogram)
		_histogram = new Histo(256, minRange, maxRange);
	else 
		_histogram->reset(256, minRange, maxRange);

	int refLevel = _rParams->GetRefinementLevel();
	int lod = _rParams->GetCompressionLevel();

	vector<double> minExts, maxExts;
	_rParams->GetBox()->GetExtents(minExts, maxExts);

	Grid* grid;

	int rc = DataMgrUtils::GetGrids(
		_dataMgr, ts, var, minExts, maxExts, true,
		&refLevel, &lod, &grid
	);
	if (rc) return;

	grid->SetInterpolationOrder(0);
	

	float v;
	Grid::Iterator itr;
	Grid::Iterator enditr = grid->end();
	for (itr = grid->begin(minExts, maxExts); itr!=enditr; ++itr){
		v = *itr;
		if (v==grid->GetMissingValue()) continue;
		_histogram->addToBin(v);
	}
	delete grid;
}

//----------------------------------------------------------------------------
// Set the underlying mapper function that this frame represents
//----------------------------------------------------------------------------
void MappingFrame::setMapperFunction(MapperFunction *mapper)
{
  deleteOpacityWidgets();
  
  _mapper = mapper;
  
  if (_mapper && _opacityMappingEnabled)
  {
    //
    // Create a new opacity widget for each opacity map in the mapper function
    //
    for(int i=0; i<_mapper->getNumOpacityMaps(); i++)
    {
		
      OpacityWidget *widget = createOpacityWidget(_mapper->GetOpacityMap(i));
#ifdef	DEAD
      _mapper->GetOpacityMap(i)->setMapper(_mapper);
#endif

      connect((QObject*)widget, SIGNAL(startChange(QString)),
              this, SIGNAL(startChange(QString)));

      connect((QObject*)widget, SIGNAL(endChange()),
              this, SIGNAL(endChange()));

      connect((QObject*)widget, SIGNAL(mapChanged()),
              this, SLOT(updateMap()));
    }

    MapperFunction::CompositionType type = _mapper->getOpacityComposition();
	
	_compTypeSubMenu->actions()[MapperFunction::ADDITION]->setChecked(type==MapperFunction::ADDITION);
	_compTypeSubMenu->actions()[MapperFunction::MULTIPLICATION]->setChecked(type==MapperFunction::MULTIPLICATION);
   
  }

  if (_colorMappingEnabled)
  {
	  if (_mapper) {
#ifdef	DEAD
		  _mapper->GetColorMap()->setMapper(_mapper);
#endif
		  _colorbarWidget->setColormap(_mapper->GetColorMap());
	  } else 
		_colorbarWidget->setColormap(NULL);
  }
  
}


//----------------------------------------------------------------------------
// Opacity mapping property. This property controls the enabledness of the
// opacity mapping capabilities (default = true). 
//
// Note: This function should be called before initializeGL() is called. 
// Ideally, it would be a flag passed in the construction, but that would 
// preclude using it as custom widget in designer. Therefore, set this property
// in designer.
//----------------------------------------------------------------------------
void MappingFrame::setOpacityMapping(bool flag)
{
  if (flag == _opacityMappingEnabled)
  {
    return;
  }

  _opacityMappingEnabled = flag;

  if (!_opacityMappingEnabled)
  {
    deleteOpacityWidgets();
  }
  else
  {
    // Error condition. Can't enable opacity mapping after it has been
    // disabled. 
    //assert(0);
  }
}

//----------------------------------------------------------------------------
// Opacity mapping property. This property controls the enabledness of the
// opacity mapping capabilities (default = true). 
//
// Note: This function should be called before initializeGL() is called. 
// Ideally, it would be a flag passed in the construction, but that would 
// preclude using it as custom widget in designer. Therefore, set this property
// in designer. 
//----------------------------------------------------------------------------
void MappingFrame::setColorMapping(bool flag)
{
  if (flag == _colorMappingEnabled)
  {
    return;
  }

  _colorMappingEnabled = flag;

  if (!_colorMappingEnabled)
  {
    delete _colorbarWidget;
    _colorbarWidget = NULL;
  }
  else
  {
    // Error condition. Can't enable opacity mapping after it has been
    // disabled. 
    //assert(0);
  }    
}

//----------------------------------------------------------------------------
// Set the variable name
//----------------------------------------------------------------------------
void MappingFrame::setVariableName(std::string name)
{
  _variableName = name;
  if (_variableName.size()> 45)
	  _variableName.resize(45);

}

//----------------------------------------------------------------------------
// Synchronize the frame with the underlying params
//----------------------------------------------------------------------------
//void MappingFrame::updateTab()
void MappingFrame::Update(DataMgr *dataMgr,
						ParamsMgr *paramsMgr,
						RenderParams *rParams)
{
	assert(dataMgr);
	assert(paramsMgr);
	assert(rParams);
	
	_dataMgr = dataMgr;
	_rParams = rParams;
	_paramsMgr = paramsMgr;

	string varname = _rParams->GetColorMapVariableName();
	if (varname == "") {
		varname = _rParams->GetVariableName();
	}

	if (varname.empty()) return;

	MapperFunction *mapper;
	mapper = _rParams->GetMapperFunc(varname);
	if (!mapper) {
		mapper = _rParams->MakeMapperFunc(varname);
		assert(mapper);
	}

	setMapperFunction(mapper);

	deselectWidgets();


	_histogram = getHistogram();
	_minValue = getMinEditBound();
	_maxValue = getMaxEditBound();

	if (_isoSliderEnabled) {
		//	   _isoVal = ((ParamsIso*)params)->GetIsoValue();
		//	   _isoSlider->setIsoValue(xDataToWorld(_isoVal));
	}
	else if (_isolineSlidersEnabled){
		//Synchronize sliders with isovalues
//#ifdef	DEAD
		vector<double> isovals = ((ContourParams*) rParams)->GetIsovalues();
		setIsolineSliders(isovals);
		for (int i = 0; i<isovals.size(); i++){
			_isolineSliders[i]->setIsoValue(xDataToWorld((float)isovals[i]));
		}
//#endif
	}

	_domainSlider->setDomain(xDataToWorld(getMinDomainBound()), 
	xDataToWorld(getMaxDomainBound()));

	_updateTexture = true;

	update();
}

//----------------------------------------------------------------------------
// Return a tool tip.  Slightly different for iso Selection
//----------------------------------------------------------------------------
QString MappingFrame::tipText(const QPoint &pos, bool isIso)
{
  QString text;

  if (!isEnabled())
  {
    return "Disabled";
  }

  int histo = histoValue(pos);
      
  float variable = xVariable(pos);
  float yopacity = yVariable(pos);
  float opacity  = getOpacityData(variable);
      
  float hf = 0.0,sf = 0.0,vf = 0.0;
  int hue =0, sat=0, val=0;
  
  if (_mapper  && _colorMappingEnabled)
  {
    _mapper->hsvValue(variable, &hf, &sf, &vf);
 
      
	  hue = (int)(hf*359.99f);
	  sat = (int)(sf*255.99f);
	  val = (int)(vf*255.99f);
  }
  
  text  = _variableName.c_str();
  text += QString(": %1;\n").arg(variable, 0, 'g', 4);
  
  if (_opacityMappingEnabled && !isIso)
  {
    text += QString("Opacity: %1; ").arg(opacity, 0, 'g', 4);
    text += QString("Y-Coord: %1;\n").arg(yopacity, 0, 'g', 4);
  }
  
  if (_colorMappingEnabled)
  {
    text += QString("Color(HSV): %1 %2 %3\n")
      .arg(hue, 3)
      .arg(sat, 3)
      .arg(val, 3);
  }
  
  if (histo >= 0)
  {
    text += QString("Histogram: %1").arg(histo);
  }

  return text;
}

//----------------------------------------------------------------------------
// Return the index value of the histogram at the window position.
//----------------------------------------------------------------------------
int MappingFrame::histoValue(const QPoint &p)
{
  if (!_histogram)
  {
    return -1;
  }
  
  QPoint pos = mapFromParent(p);
 
  float x = xWorldToData(xViewToWorld(pos.x()));
	if (_histogram->getMaxData() <= _histogram->getMinData()) return 0;
  float ind = (x - _histogram->getMinData())
    / (_histogram->getMaxData() - _histogram->getMinData());

  if (ind < 0.f || ind >= 1.f)
  {
    return 0;
  }

  int index = (int)(ind*255.999f);

  return _histogram->getBinSize(index); 
}

//----------------------------------------------------------------------------
// Map the window position to x
//----------------------------------------------------------------------------
float MappingFrame::xVariable(const QPoint &pos)
{
  return xWorldToData(xViewToWorld(pos.x()));
}

//----------------------------------------------------------------------------
// Map the window position to y
//----------------------------------------------------------------------------
float MappingFrame::yVariable(const QPoint &pos)
{
  return yWorldToData(yViewToWorld(height() - pos.y()));
}

//----------------------------------------------------------------------------
// Set the scaling type of the histogram.
//----------------------------------------------------------------------------
void MappingFrame::setHistogramScale(QAction* act)
{
  _histogramScale = act->data().toInt();

  _updateTexture = true;

  _histogramScalingSubMenu->actions()[BOOLEAN]->setChecked(_histogramScale==BOOLEAN);
  _histogramScalingSubMenu->actions()[LINEAR]->setChecked(_histogramScale==LINEAR);
  _histogramScalingSubMenu->actions()[LOG]->setChecked(_histogramScale==LOG);

  updateGL();
}

//----------------------------------------------------------------------------
// Set the composition type of the transfer function
//----------------------------------------------------------------------------
void MappingFrame::setCompositionType(QAction* act)
{
  emit startChange("Opacity composition type changed");
	int type = act->data().toInt();
  _compTypeSubMenu->actions()[MapperFunction::MULTIPLICATION]->setChecked(type==MapperFunction::MULTIPLICATION);
  _compTypeSubMenu->actions()[MapperFunction::ADDITION]->setChecked(type==MapperFunction::ADDITION);
  
  _mapper->setOpacityComposition((MapperFunction::CompositionType)type);

  emit endChange();

  updateGL();
}

//----------------------------------------------------------------------------
// Enable/disable TF widget
//----------------------------------------------------------------------------
void MappingFrame::setWidgetEnabled(QAction* act)
{
  OpacityWidget *opacWidget = dynamic_cast<OpacityWidget*>(_lastSelected);
  int enabled = act->data().toInt();
  if (opacWidget)
  {
    if (enabled)
    {
      emit startChange("Opacity widget enabled");
    }
    else
    {
      emit startChange("Opacity widget disabled");
    }

    _widgetEnabledSubMenu->actions()[ENABLED]->setChecked(enabled==ENABLED);
	_widgetEnabledSubMenu->actions()[DISABLED]->setChecked(enabled==DISABLED);
   
    opacWidget->enable(enabled==ENABLED);

    emit endChange();

    updateGL();
  }
}


//----------------------------------------------------------------------------
// Enable/disable the edit mode. When edit mode is disabled, mouse clicks
// will scale and pan the mapping space. When edit mode is enabled, mouse 
// clicks operate on the GLWidgets. 
//----------------------------------------------------------------------------
void MappingFrame::setEditMode(bool flag)
{
  _editMode = flag;

  if (_editMode)
  {
    setCursor(QCursor(Qt::ArrowCursor));
  }
  else
  {
    setCursor(QCursor(Qt::SizeAllCursor));
  }
  navigateButton->setChecked(!flag);
}
  
//----------------------------------------------------------------------------
// Fit the mapping space to the current domain.
//----------------------------------------------------------------------------
void MappingFrame::fitToView()
{
  //Make sure it's current active params:

  emit startChange("Mapping window fit-to-view");
  
  _minValue = getMinDomainBound();
  _maxValue = getMaxDomainBound();
  
  setMinEditBound(_minValue);
  setMaxEditBound(_maxValue);
 
  _domainSlider->setDomain(xDataToWorld(_minValue), xDataToWorld(_maxValue));
  if(_colorbarWidget) _colorbarWidget->setDirty();
 
  updateGL();
}

//----------------------------------------------------------------------------
// Force a redraw in the vizualization window.
//----------------------------------------------------------------------------
void MappingFrame::updateMap()
{
  _colorbarWidget->setDirty();
  emit mappingChanged();
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void MappingFrame::newHsv(int h, int s, int v)
{
  if (_colorbarWidget)
  {
    _colorbarWidget->newHsv(h, s, v);

    emit mappingChanged();

    updateGL();
  }
}

//----------------------------------------------------------------------------
// Returns true if the selected color control points and opacity control 
// points can be bound. False otherwise. 
//
// Control points can be bound iff exactly one color control and exactly one 
// opacity control point are selected.
//----------------------------------------------------------------------------
bool MappingFrame::canBind()
{
  if (_selectedWidgets.size() == 2 && 
      _selectedWidgets.find(_colorbarWidget) != _selectedWidgets.end() &&
      _colorbarWidget->selectedPoints().size() == 1)
  {
    set<GLWidget*>::iterator iter;

    for (iter = _selectedWidgets.begin(); iter!=_selectedWidgets.end(); iter++)
    {
      OpacityWidget *owidget = dynamic_cast<OpacityWidget*>(*iter);

      if (owidget)
      {
        return (owidget->selectedPoints().size() == 1);
      }
    }
  }

  return false;
}

//----------------------------------------------------------------------------
// Set the selected color control point to the value of the selected opacity
// control point. 
//----------------------------------------------------------------------------
void MappingFrame::bindColorToOpacity()
{
  map<int, OpacityWidget*>::iterator iter;

  for (iter = _opacityWidgets.begin(); iter != _opacityWidgets.end(); iter++)
  {
    OpacityWidget *owidget = (*iter).second;

    if (owidget->selected())
    {
      int oIndex = owidget->selectedControlPoint();
      OpacityMap *omap = owidget->opacityMap();

      int cIndex = _colorbarWidget->selectedControlPoint();
      ColorMap   *cmap = _colorbarWidget->colormap();
      
      cmap->controlPointValue(cIndex, omap->controlPointValue(oIndex));

      updateGL();

      return;
    }
  }  
}

//----------------------------------------------------------------------------
// Set the selected opacity control point to the data value of the selected
// color control point. 
//----------------------------------------------------------------------------
void MappingFrame::bindOpacityToColor()
{
  map<int, OpacityWidget*>::iterator iter;

  for (iter = _opacityWidgets.begin(); iter != _opacityWidgets.end(); iter++)
  {
    OpacityWidget *owidget = (*iter).second;

    if (owidget->selected())
    {
      int oIndex = owidget->selectedControlPoint();
      OpacityMap *omap = owidget->opacityMap();

      int cIndex = _colorbarWidget->selectedControlPoint();
      ColorMap   *cmap = _colorbarWidget->colormap();
      
      omap->controlPointValue(oIndex, cmap->controlPointValue(cIndex));

      updateGL();

      return;
    }
  }  
}

//----------------------------------------------------------------------------
// Initialize the frame's contents
//----------------------------------------------------------------------------
void MappingFrame::initWidgets()
{
  //
  // Create the 2D histogram texture
  //
  _texture = new unsigned char[_NUM_BINS*_NUM_BINS];

  //
  // Create the context sensitive menu
  //
  _contextMenu = new QMenu(this);

  _addOpacityWidgetSubMenu = new QMenu(_contextMenu);
  QAction* act = _addOpacityWidgetSubMenu->addAction("Control Points");    
  act->setData(OpacityMap::CONTROL_POINT);
  act = _addOpacityWidgetSubMenu->addAction("Gaussian");
  act->setData(OpacityMap::GAUSSIAN);
  act = _addOpacityWidgetSubMenu->addAction("Inverted Gaussian");
  act->setData(OpacityMap::INVERTED_GAUSSIAN);
                                       

  _histogramScalingSubMenu = new QMenu(_contextMenu);
  
  QAction* histact = _histogramScalingSubMenu->addAction("Boolean");
  histact->setData(BOOLEAN);
  histact->setCheckable(true);
  histact->setChecked(false);

  histact = _histogramScalingSubMenu->addAction("Linear");
  histact->setData(LINEAR);
  histact->setCheckable(true);
  histact->setChecked(true);
 
  histact = _histogramScalingSubMenu->addAction("Log");
  histact->setData(LOG);
  histact->setCheckable(true);
  histact->setChecked(false);

  _compTypeSubMenu = new QMenu(_contextMenu);
  
  QAction* addact = _compTypeSubMenu->addAction("Addition");
  addact->setCheckable(true);
  addact->setChecked(true);
  addact->setData(MapperFunction::ADDITION);

  QAction* multact = _compTypeSubMenu->addAction("Multiplication");
  multact->setCheckable(true);
  multact->setChecked(false);
  multact->setData(MapperFunction::MULTIPLICATION);
                              
 

  _widgetEnabledSubMenu = new QMenu(_contextMenu);

  QAction* widEnabled = _widgetEnabledSubMenu->addAction("Enabled");
  widEnabled->setCheckable(true);
  widEnabled->setChecked(true);

  QAction* widDisabled = _widgetEnabledSubMenu->addAction("Disabled");
  widDisabled->setCheckable(true);
  widDisabled->setChecked(false);
 

  _deleteOpacityWidgetAction = new QAction(this);
  _deleteOpacityWidgetAction->setText("Delete Opacity Widget");

  _addColorControlPointAction = new QAction(this);
  _addColorControlPointAction->setText("New Color Control Point");

  _addOpacityControlPointAction = new QAction(this);
  _addOpacityControlPointAction->setText("New Opacity Control Point");

  _editControlPointAction = new QAction(this);
  _editControlPointAction->setText("Edit Control Point");

  _deleteControlPointAction = new QAction(this);
  _deleteControlPointAction->setText("Delete Control Point");

 
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void MappingFrame::initConnections()
{
  connect(_addOpacityControlPointAction, SIGNAL(triggered()), 
          this, SLOT(addOpacityControlPoint()));

  connect(_addColorControlPointAction, SIGNAL(triggered()), 
          this, SLOT(addColorControlPoint()));

  connect(_editControlPointAction, SIGNAL(triggered()), 
          this, SLOT(editControlPoint()));

  connect(_deleteControlPointAction, SIGNAL(triggered()), 
          this, SLOT(deleteControlPoint()));

  connect(_addOpacityWidgetSubMenu, SIGNAL(triggered(QAction*)),
          this, SLOT(addOpacityWidget(QAction*)));

  connect(_histogramScalingSubMenu, SIGNAL(triggered(QAction*)),
          this, SLOT(setHistogramScale(QAction*)));

  connect(_compTypeSubMenu, SIGNAL(triggered(QAction*)),
          this, SLOT(setCompositionType(QAction*)));

  connect(_widgetEnabledSubMenu, SIGNAL(triggered(QAction*)),
          this, SLOT(setWidgetEnabled(QAction*)));

  connect(_deleteOpacityWidgetAction, SIGNAL(triggered()), 
          this, SLOT(deleteOpacityWidget()));

  connect(_colorbarWidget, SIGNAL(mapChanged()), this, SLOT(updateMap()));

  connect(_colorbarWidget, SIGNAL(sendRgb(QRgb)), this, SIGNAL(sendRgb(QRgb)));
}

//----------------------------------------------------------------------------
// Remove all the opacity widgets
//----------------------------------------------------------------------------
void MappingFrame::deleteOpacityWidgets()
{
  makeCurrent();
  deselectWidgets();

  map<int, OpacityWidget*>::iterator iter;

  for (iter = _opacityWidgets.begin(); iter != _opacityWidgets.end(); iter++)
  {
    delete (*iter).second;
    (*iter).second = NULL;
  }

  _opacityWidgets.clear();
}

//-----------------------------------------------------------------------------
// Set up the OpenGL view port, matrix mode, etc.
//----------------------------------------------------------------------------
void MappingFrame::resizeGL(int width, int height)
{
	
	printOpenGLErrorMsg("MappingFrameResizeEvent");
	
	resize();
	/*
  // Update the size of the drawing rectangle
  //
  glViewport( 0, 0, (GLint)width, (GLint)height );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  */
  qglClearColor(QColor(0,0,0)); 
  
  printOpenGLErrorMsg("MappingFrameResizeEvent");
}
	
//----------------------------------------------------------------------------
// Draw the frame's contents
//----------------------------------------------------------------------------
void MappingFrame::paintGL()
{
  // On Mac Qt invokes paintGL when frame frame buffer isn't ready :-(
  //
  if (! FrameBufferReady()) {
    return;
  }

  if (!_mapper) return;
  resize();
  int rc = printOpenGLErrorMsg("MappingFrame::paintGL");
  if (rc < 0){
      MSG_ERR("MappingFrame::paintGL");
	  return;
  }

  oglPushState();

  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);

  glViewport( (GLint) _minX, (GLint) _minY, (GLsizei)width(),(GLsizei)height());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(_minX, _maxX, _minY, _maxY, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);

  qglClearColor(palette().color(QPalette::Background));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor3f(0.0, 0.0, 0.0);
  glBegin(GL_QUADS);
  {
    glVertex2f(_minX, _minY);
    glVertex2f(_minX, _maxY);
    glVertex2f(_maxX, _maxY);
    glVertex2f(_maxX, _minY); 
  }
  glEnd();

  //
  // Draw Histogram
  //
  if (_histogram)
  {
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _texid);

    if (_updateTexture)
    {
      updateTexture();
    }
	
    
    glColor3f(0.0, 0.784, 0.784);

	glBegin(GL_QUADS);
    {
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(xDataToWorld(_histogram->getMinData()), 0.0);
      
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(xDataToWorld(_histogram->getMinData()), 1.0);
      
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(xDataToWorld(_histogram->getMaxData()), 1.0);
      
      glTexCoord2f(1.0f, 0.0f); 
      glVertex2f(xDataToWorld(_histogram->getMaxData()), 0.0); 
    }
	glEnd();

    glDisable(GL_TEXTURE_2D);
  }

//  glEnable(GL_LIGHTING);
//  glEnable(GL_LIGHT0);

  //
  // Draw Opacity Widgets
  //
  rc = drawOpacityWidgets();
  if (rc < 0) {
      MSG_ERR("MappingFrame");
	  oglPopState();
	  return;
  }

  //
  // Draw the opacity curve
  //
  rc = drawOpacityCurve();
  if (rc < 0) {
      oglPopState();
      MSG_ERR("MappingFrame");
	  return;
  }

  //
  // Draw Domain Slider
  //
  rc = drawDomainSlider();
  if (rc < 0) {
      MSG_ERR("MappingFrame");
      oglPopState();
	  return;
  }

  if(_isoSliderEnabled) rc = drawIsoSlider();
  if (rc < 0) {
      MSG_ERR("MappingFrame");
      oglPopState();
	  return;
  }

  if(_isolineSlidersEnabled) rc = drawIsolineSliders();
  if (rc < 0) {
      MSG_ERR("MappingFrame");
      oglPopState();
	  return;
  }
  //
  // Draw Domain Variable Name.  Cannot be performed in drawDomainSlider, 
  // because
  // qglWidget::renderText() has the side-effect of recording a hit when we
  // select points
  //

  if (_variableName != "" ){
		//allow for 4 pixels per character in name:
		int wx = (width() - _variableName.size()*8)/2;
		qglColor(Qt::red);
		renderText(wx, _domainLabelHeight+15, QString::fromStdString(_variableName), QFont("Arial",10,5,false));
  }
  
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);
	
  //
  // Draw the colorbar
  //
  rc = drawColorbar();
  if (rc < 0) {
      MSG_ERR("MappingFrame");
      oglPopState();
	  return;
  }

  //
  // Draw axis region background
  //
  QColor color = parentWidget()->palette().color(QPalette::Background);
  glColor3f(color.red()/255.0, color.green()/255.0, color.blue()/255.0);

  float unitPerPixel = 1.0/(height()-totalFixedHeight());
  float maxY = _minY + (unitPerPixel * _axisRegionHeight);

  glBegin(GL_QUADS);
  {
    glVertex2f(_minX, _minY);
    glVertex2f(_minX, maxY);
    glVertex2f(_maxX, maxY);
    glVertex2f(_maxX, _minY); 
  }
  glEnd();

  //
  // Update Axis Labels
  //
  updateAxisLabels();

  //
  // If the MappingFrame widget is disabled, gray it out. 
  //
  if (!isEnabled() && !_isoSliderEnabled  &&!_isolineSlidersEnabled)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.5, 0.5, 0.5, 0.35);
    
    glBegin(GL_QUADS);
    {
      glVertex3f(_minX, maxY, 0.9);
      glVertex3f(_minX, _maxY, 0.9);
      glVertex3f(_maxX, _maxY, 0.9);
      glVertex3f(_maxX, maxY, 0.9); 
    }
    glEnd();

    glDisable(GL_BLEND);
  }
  
  swapBuffers();
  glFlush();

  oglPopState();

  printOpenGLErrorMsg("MappingFrame::paintGL");
}

//----------------------------------------------------------------------------
// Set up the OpenGL rendering state
//----------------------------------------------------------------------------
void MappingFrame::initializeGL()
{
 
  MyBase::SetDiagMsg("MappingFrame::initializeGL()");
  printOpenGLErrorMsg("MappingFrame");
  setAutoBufferSwap(false);
  qglClearColor(QColor(0,0,0)); 

  glShadeModel( GL_SMOOTH );


  //
  // Initialize the histogram texture
  //
  glGenTextures(1, &_texid);
  glBindTexture(GL_TEXTURE_2D, _texid);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  //
  // Enable lighting for opacity widget handles
  //
  static float ambient[]  = {0.2, 0.2, 0.2, 1.0};
  static float diffuse[]  = {1.0, 1.0, 1.0, 1.0};
  static float specular[] = {0.0, 0.0,  0.0, 0.0};
  static float lightpos[] = {0.0, 1.0, 5000000.0};

  glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 128);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDisable(GL_CULL_FACE);

  if (_colorMappingEnabled)
  {
    _colorbarWidget->initializeGL();
  }
  printOpenGLErrorMsg("MappingFrame");
}

//----------------------------------------------------------------------------
// Draw the opacity curve
//----------------------------------------------------------------------------
int MappingFrame::drawOpacityCurve()
{
  if (_mapper && _opacityMappingEnabled)
  {
    float step = (_maxValue - _minValue)/(_NUM_BINS-1);
    
    glColor3f(0.0, 1.0, 0.0);
    
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glBegin(GL_LINE_STRIP);
    {
      for (int i=0; i<_NUM_BINS; i++)
      {
        if (_minValue + i*step >= getMinDomainBound() &&
            _minValue + i*step <= getMaxDomainBound())
        {
		  //Normalize data values between 0 and 1
		  float nv = i*step/(_maxValue - _minValue);
          float opacity = getOpacityData(xWorldToData(nv));
          
          glVertex3f((float)i/(_NUM_BINS-1), opacity, 0.0);
        }
      }
    }
    glEnd();

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
//    glEnable(GL_LIGHT0);
 //   glEnable(GL_LIGHTING);
  }
  int rc = printOpenGLErrorMsg("DrawOpacityCurve");
  if (rc < 0) return -1;
  return 0;
}

//----------------------------------------------------------------------------
// Draw the opacity widgets
//----------------------------------------------------------------------------
int MappingFrame::drawOpacityWidgets()
{
  if (_opacityMappingEnabled)
  {
    map<int, OpacityWidget*>::iterator iter;
    
    glPushName(OPACITY_WIDGETS);
    
    for (iter = _opacityWidgets.begin(); iter != _opacityWidgets.end(); iter++)
    {
      int rc = (*iter).second->paintGL();
	  if (rc < 0) {
		  glPopName();
		  return -1;
	  }
	
    }
    
    glPopName(); // OPACITY_WIDGETS
  }
  
  return 0;
}

//----------------------------------------------------------------------------
// Draw the domain slider
//----------------------------------------------------------------------------
int MappingFrame::drawDomainSlider()
{
  glPushName(DOMAIN_WIDGET);

  int rc = _domainSlider->paintGL();
  
  glPopName();
  return rc;
  
}
//----------------------------------------------------------------------------
// Draw the iso slider
//----------------------------------------------------------------------------
int MappingFrame::drawIsoSlider()
{
	
	glPushName(ISO_WIDGET);
  
	int rc = _isoSlider->paintGL();
  
	glPopName();
	return rc;
}
//----------------------------------------------------------------------------
// Draw all the isoline sliders
//----------------------------------------------------------------------------
int MappingFrame::drawIsolineSliders()
{
	for (int i = 0; i<_isolineSliders.size(); i++){
		int sliderName = (int)(ISO_WIDGET) + i + 1;
		glPushName(sliderName);
  
		int rc = _isolineSliders[i]->paintGL();
		glPopName();
		if (rc < 0) return rc;
	}
	return 0;
}
//----------------------------------------------------------------------------
// Draw the colorbar
//----------------------------------------------------------------------------
int MappingFrame::drawColorbar()
{
  int rc = 0;
  if (_colorMappingEnabled && _colorbarWidget)
  {
    glPushName(COLORBAR_WIDGET);

    rc = _colorbarWidget->paintGL(); 

    glPopName();
  }
  return rc;
}

//----------------------------------------------------------------------------
// Rebuild the histogram texture
//----------------------------------------------------------------------------
void MappingFrame::updateTexture()
{
	if (! _histogram && _mapper) return;

	float stretch;
	stretch = _rParams->GetHistoStretch();



	for (int x=0; x<_NUM_BINS; x++)
	{
		float binValue = 0.0;

		//
		// Find the histogram value based on the current scaling
		// type.
		//
		switch (_histogramScale) {
		case LINEAR:
		{
			binValue = MIN(1.0, (stretch * _histogram->getBinSize(x) / 
			   _histogram->getMaxBinSize()));
			break;
		}

		case LOG:
		{
			binValue = logf(stretch * _histogram->getBinSize(x)) / 
			logf(_histogram->getMaxBinSize());
			break;
		}

		default: // BOOLEAN
		{
			binValue = _histogram->getBinSize(x) ? 1.0 : 0.0;
		}
		}

		int histoHeight = (int)(_NUM_BINS * binValue);

		for (int y=0; y<_NUM_BINS; y++)
		{
			int index = x + y*_NUM_BINS;

			if (y < histoHeight)
			{
				_texture[index] = 255;
			}
			else
			{
				_texture[index] = 0;
			}
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, 
	GL_UNSIGNED_BYTE, _texture);

	_updateTexture = false;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void MappingFrame::updateAxisLabels()
{
  int x;
  int y = height()-10;

  list<float> ticks;

  // Eliminate axis text and points
  for (int i = 0; i<_axisTexts.size(); i++){
	  delete _axisTextPos[i];
  }
  _axisTexts.clear();
  _axisTextPos.clear();


  OpacityWidget *opacWidget = dynamic_cast<OpacityWidget*>(_lastSelected);

  if (opacWidget)
  {
    list<float> points = opacWidget->selectedPoints();

    if (points.size())
    {
      list<float>::iterator iter;

      for (iter=points.begin(); iter!=points.end(); iter++)
      {
        x = (int)xWorldToView(xDataToWorld(*iter));
        addAxisLabel(x, y, QString("%1").arg(*iter));

        ticks.push_back(*iter);
      }
    }
    else
    {
      float mind = opacWidget->opacityMap()->minValue();
      float maxd = opacWidget->opacityMap()->maxValue();
      
      x = (int)xWorldToView(xDataToWorld(mind));
      addAxisLabel(x, y, QString("%1").arg(mind));

      x = (int)xWorldToView(xDataToWorld(maxd));
      addAxisLabel(x, y, QString("%1").arg(maxd));

      ticks.push_back(mind);
      ticks.push_back(maxd);
    }
  }

  else if (_colorbarWidget == _lastSelected && _lastSelected)
  {
    list<float> points = _colorbarWidget->selectedPoints();
    list<float>::iterator iter;

    for (iter=points.begin(); iter!=points.end(); iter++)
    {
      x = (int)xWorldToView(xDataToWorld(*iter));
      addAxisLabel(x, y, QString("%1").arg(*iter));

      ticks.push_back(*iter);
    }
  }

  else if (_domainSlider)
  {
    float mind = _domainSlider->minValue();
    float maxd = _domainSlider->maxValue();
   
    x = (int)xWorldToView(mind);
    addAxisLabel(x, y, QString("%1").arg(xWorldToData(mind)));
    
    x = (int)xWorldToView(maxd);
    addAxisLabel(x, y, QString("%1").arg(xWorldToData(maxd)));

    ticks.push_back(xWorldToData(mind));
    ticks.push_back(xWorldToData(maxd));
  }

  if (_isoSlider && _isoSliderEnabled) 
  {
	  float isoval = (_isoSlider->minValue()+_isoSlider->maxValue())*0.5;

	  x = int(xWorldToView(isoval));
	  addAxisLabel(x,y,QString("%1").arg(xWorldToData(isoval)));
  }
  if (_isolineSlidersEnabled){
	  for (int i = 0; i<_isolineSliders.size(); i++){
		  float isoval = (_isolineSliders[i]->minValue() + _isolineSliders[i]->maxValue())*0.5;
		  x = int(xWorldToView(isoval));
		  addAxisLabel(x,y-13,QString("%1").arg(xWorldToData(isoval)));
	  }
  }


  //
  // Draw ticks
  //
  glColor4f(1.0, 1.0, 1.0, 1.0);

  float unitPerPixel = 1.0 / (float)(height()-totalFixedHeight());
  float y0 = _minY + unitPerPixel * _axisRegionHeight;
  float y1 = y0 + unitPerPixel * _bottomGap;
  list<float>::iterator iter;

  glBegin(GL_LINES);
  {
    for (iter=ticks.begin(); iter!=ticks.end(); iter++)
    {
      glVertex3f(xDataToWorld(*iter), y0, 0.05);
      glVertex3f(xDataToWorld(*iter), y1, 0.05);
    }
  }
  glEnd();

  //
  // Draw axis labels
  //
  qglColor(Qt::black);
  if (!_isolineSlidersEnabled) {
	  for (int i = 0; i<_axisTexts.size(); i++){
			
			QPoint* pt = _axisTextPos[i];
			renderText(pt->x(),pt->y(), _axisTexts[i], QFont("Arial",10,5,false));
	  }
  }
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void MappingFrame::addAxisLabel(int x, int y, const QString &text)
{
	
	int xpos = x - text.length()*3;
	if (xpos < 0) xpos = 0;
	QPoint* pos = new QPoint(xpos,y+5);
	_axisTextPos.push_back(pos);
	_axisTexts.push_back(text);
	
}


//----------------------------------------------------------------------------
// Select the GLWidget(s) at the given position
//----------------------------------------------------------------------------
void MappingFrame::select(int x, int y, Qt::KeyboardModifiers state)
{
  const int length = 128;
  static GLuint selectionBuffer[length];

  GLint hits;
  GLint viewport[4];

  makeCurrent();

  glInitNames();
  glPushName(0);
  glViewport(0,0,width(),height());
  //
  // Setup selection buffer
  //
  glSelectBuffer(length, selectionBuffer);

  glGetIntegerv(GL_VIEWPORT, viewport);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();

  glRenderMode(GL_SELECT);

  //
  // Establish new clipping volume as the unit cube around the 
  // mouse position.
  //
  glLoadIdentity();
  gluPickMatrix(x, viewport[3] - y, 4,4, viewport);
  glOrtho(_minX, _maxX, _minY, _maxY, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);

  //
  // Render widgets
  //
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawOpacityWidgets();
  drawDomainSlider();
  if(_isoSliderEnabled) drawIsoSlider();
  if(_isolineSlidersEnabled) drawIsolineSliders();
  drawColorbar();

  //
  // Restore projection matrix
  //
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  
  //
  // Collect hits
  //
  hits = glRenderMode(GL_RENDER);

  if (hits)
  {
    select(hits, selectionBuffer, state);
  }
  else
  {
    deselectWidgets();
  }
  glPopName();
}

//----------------------------------------------------------------------------
// Parse the GL selection buffer
//----------------------------------------------------------------------------
void MappingFrame::select(int hits, GLuint *selectionBuffer, Qt::KeyboardModifiers state)
{
  if (!(state & (Qt::ShiftModifier|Qt::ControlModifier)))
  {
    deselectWidgets();
  }

  int offset = 0;
  int hitOffset = 0;
  int maxCount = 0;
  _lastSelectedIndex = -1;

  //
  // Find the hit with the maximum count
  //
  for (int h=0; h < hits; h++)
  {
    int count = selectionBuffer[offset];

    if (count > maxCount)
    {
      maxCount = count;
      hitOffset = offset;
    }

    offset += count+3;  
  }

  //
  // Select the leaf object of the hit with the maximum count
  //
  if (selectionBuffer[hitOffset+3] == OPACITY_WIDGETS)
  {
    _lastSelected = _opacityWidgets[ selectionBuffer[hitOffset+4] ];
  }
  else if (selectionBuffer[hitOffset+3] == DOMAIN_WIDGET)
  {
    deselectWidgets();

    _lastSelected = _domainSlider;
  }
  else if (selectionBuffer[hitOffset+3] == ISO_WIDGET)
  {
    deselectWidgets();

    _lastSelected = _isoSlider;
  }
   else if ((int)selectionBuffer[hitOffset+3] > (int)ISO_WIDGET)  //must have selected one of the isoline widgets
  {
    deselectWidgets();
	int selectedIndex = (int)selectionBuffer[hitOffset+3] - (int)ISO_WIDGET - 1;
    _lastSelected = _isolineSliders[selectedIndex];
	_lastSelectedIndex = selectedIndex;
  }
  else if (selectionBuffer[hitOffset+3] == COLORBAR_WIDGET)
  {
    _lastSelected = _colorbarWidget;
    
    if (maxCount < 2)
    {
      //
      // Clicked on the colorbar, but not on a control point. Deselect.
      //
      deselectWidgets();

      return;
    }
  }

  assert(_lastSelected);

  _lastSelected->select(selectionBuffer[hitOffset+maxCount+2], state);

  //
  // Depending on the state of _lastSelected and the button modifiers,
  // the GLWidget::select(...) call may have selected or deselected 
  // _lastSelected. We'll respond accordingly.
  //
  if (_lastSelected->selected())
  {
    _selectedWidgets.insert(_lastSelected);
  }
  else
  {
    _selectedWidgets.erase(_lastSelected);
    _lastSelected = NULL;
  }

  updateGL();

  emit canBindControlPoints(canBind());
}

//----------------------------------------------------------------------------
// Deselect all selected widgets
//----------------------------------------------------------------------------
void MappingFrame::deselectWidgets()
{
  if(_lastSelected)
  {
    _lastSelected->deselect();
    _lastSelected = NULL;
	_lastSelectedIndex = -1;
  }

  set<GLWidget*>::iterator iter;

  for (iter=_selectedWidgets.begin(); iter!=_selectedWidgets.end(); iter++)
  {
    (*iter)->deselect();
  }

  _selectedWidgets.clear();
}

//----------------------------------------------------------------------------
// Return the sum of all the fixed height areas of the mapping frame (i.e.,
// all the areas/widgets of the frame that fixed and do not scale relative
// to the size of the frame).
//----------------------------------------------------------------------------
int MappingFrame::totalFixedHeight()
{
  int total = _domainHeight + _axisRegionHeight + _bottomGap;

  if (_colorMappingEnabled)
  {
    total += _colorbarHeight;
  }

  if (_opacityMappingEnabled)
  {
    total += 2 * _opacityGap;
  }
 
  return total;
}


//----------------------------------------------------------------------------
// Handle resize events. This method adjusts the gl coordinates of the 
// glwidgets, so that, the domain widget and colorbar widget maintain
// a fixed size. 
//
//       _minX                           _maxX
//         |                               |
//         *-------------------------------* - _maxY (calculated)
//         *                               *
//         *        Domain Widget          *    fixed height
//         *                               *
//         * - - - - - - - - - - - - - - - * - 1.0
//         *                               *
//         *                               *
//         *                               *
//         *      Histogram/Opacity        *
//         *                               *    variabled height
//         *                               *
//         *                               *
//         *                               *
//         * - - - - - - - - - - - - - - - * - 0.0
//         *                               *
//         *       Colorbar Widget         *     fixed height
//         *                               *
//         * - - - - - - - - - - - - - - - * - (calculated)
//         *                               *
//         *       Annotation Space        *     fixed height
//         *                               *
//         *-------------------------------* - _maxY (calculated)
//
//
//
//----------------------------------------------------------------------------
void MappingFrame::resize()
{
  //
  // View to world coordinates factor
  //

  float unitPerPixel = 1.0 / (float)(height()-totalFixedHeight());

  //Provide extra space at bottom for 2 rows of annotation with isolines.
  if (_isolineSlidersEnabled) _axisRegionHeight = 30;
  //
  // Determine the new y extents
  //
  _minY = - unitPerPixel * (_axisRegionHeight + _bottomGap);

  if (_colorMappingEnabled)
  {
    _minY += - unitPerPixel * (_colorbarHeight);
  }

  if (_opacityMappingEnabled)
  {
    _minY += - unitPerPixel * (_opacityGap);
  }

  _maxY = 1.0 + unitPerPixel * (_domainHeight + _opacityGap);


  float domainWidth = unitPerPixel * _domainBarHeight;

  _domainSlider->setGeometry(_minX, _maxX, _maxY-domainWidth, _maxY);

  float bGap   = unitPerPixel * _bottomGap;

  if (_colorbarWidget)
  {
    float cbWidth = unitPerPixel * _colorbarHeight;

    if (!_opacityMappingEnabled)
    {
      bGap *= 0.5;
    }

    _colorbarWidget->setGeometry(_minX, _maxX, -(bGap+cbWidth), -bGap);
  }

  float ogap = unitPerPixel * _opacityGap;
  
  map<int, OpacityWidget*>::iterator iter;
  
  for (iter = _opacityWidgets.begin(); iter != _opacityWidgets.end(); iter++)
  {
    OpacityWidget *widget = (*iter).second;

    if (_colorMappingEnabled)
    {
      widget->setGeometry(_minX, _maxX, 0.0-ogap, 1.0+ogap);
    }
    else
    {
      widget->setGeometry(_minX, _maxX, 0.0-ogap-bGap, 1.0+ogap);
    }
  }       
}


//----------------------------------------------------------------------------
// Handle mouse press events
//----------------------------------------------------------------------------
void MappingFrame::mousePressEvent(QMouseEvent *event)
{
  _paramsMgr->BeginSaveStateGroup("MappingFrame mousePressEvent");
  select(event->x(), event->y(), event->modifiers());

  _lastx = xViewToWorld(event->x());
  _lasty = yViewToWorld(height() - event->y());

  _clickedPos = event->pos();
  _minValueStart = _minValue;
  _maxValueStart = _maxValue;

  _button = event->buttons();

  if (_editMode && (_button == Qt::LeftButton || _button == Qt::MidButton))
  {
    if (_lastSelected)
    {
      if (_lastSelected != _domainSlider)
      {
        if (_lastSelected == _colorbarWidget)
        {
          emit startChange("Colormap edit");
        }
		else if (_lastSelected == _isoSlider) 
		{
		  emit startChange("Iso slider move");
		}
		else if (dynamic_cast<IsoSlider*>(_lastSelected)) //check if an isolineSlider was picked...
		{
		  emit startChange("Isoline slider move");
		}
		else
		{
          emit startChange("Opacity map edit");
        }
      } else emit startChange("Domain slider move");
    }
    
  } 
  else if (!_editMode && (_button == Qt::LeftButton))
	{
		emit startChange("Mapping window zoom/pan");
	}

  updateGL();
}

//----------------------------------------------------------------------------
// Handle mouse release events
//----------------------------------------------------------------------------
void MappingFrame::mouseReleaseEvent(QMouseEvent *event)
{
  _button = event->button();

  if (_editMode &&(_button == Qt::LeftButton || _button == Qt::MidButton))
  {
    if (_lastSelected == _domainSlider)
    {
      setDomain();
    }
	else if (_lastSelected == _isoSlider)
    {
	  setIsoSlider();
	}
	else if (dynamic_cast<IsoSlider*>(_lastSelected)  && _lastSelectedIndex >= 0) //check if an isolineSlider was picked...
	{
	  setIsolineSlider(_lastSelectedIndex);
	}
    else if (_lastSelected)
    {
      emit endChange();
    }
  } 
  else if (!_editMode && _button == Qt::LeftButton) 
  {
    setMinEditBound(_minValue);
    setMaxEditBound(_maxValue);

    _domainSlider->setDomain(xDataToWorld(getMinDomainBound()), 
                             xDataToWorld(getMaxDomainBound()));

    updateGL();

	emit endChange();
	emit updateParams();
  }

  _paramsMgr->EndSaveStateGroup();
}

//----------------------------------------------------------------------------
// Handle mouse double-click events
//----------------------------------------------------------------------------
void MappingFrame::mouseDoubleClickEvent(QMouseEvent* /* event*/)
{
  editControlPoint();
}

//----------------------------------------------------------------------------
// Handle mouse movement events
//----------------------------------------------------------------------------
void MappingFrame::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons()== Qt::NoButton){
		bool isIso =  (_isoSliderEnabled ||_isolineSlidersEnabled);
		QToolTip::showText(event->globalPos(), tipText(event->pos(), isIso));
		return;
	}
	set<GLWidget*>::iterator iter;

	if (_editMode && _selectedWidgets.size())
	{
		float x = xViewToWorld(event->x());
		float y = yViewToWorld(height() - event->y());

		float dx = x - _lastx;
		float dy = y - _lasty;

		_lastx = x;
		_lasty = y;

		if (_button == Qt::LeftButton && _editMode)
		{
		  for (iter=_selectedWidgets.begin(); iter!=_selectedWidgets.end(); iter++)
		  {
			(*iter)->drag(dx, dy);
		  }
		}
		if (_button == Qt::MidButton)
		{
		  for (iter=_selectedWidgets.begin(); iter!=_selectedWidgets.end(); iter++)
		  {
			(*iter)->move(dx, dy);
		  }
		}

		parentWidget()->repaint();
	}
	else if (_button == Qt::LeftButton && !_editMode)
	{
		float zoomRatio = pow(2.f, (float)(event->y()-_clickedPos.y())/height());

		//Determine the horizontal pan as a fraction of edit window width:
		float horizFraction = (float)(event->x()-_clickedPos.x())/(float)(width());

		//The zoom starts at the original drag start; i.e. that point won't move
		float startXMapped = 
		  ((float)_clickedPos.x()/(float)(width())) * 
		  (_maxValueStart-_minValueStart) + _minValueStart;

		float minv = startXMapped - (startXMapped - _minValueStart) * zoomRatio;
		float maxv= startXMapped + (_maxValueStart - startXMapped) * zoomRatio;

		_minValue = minv - horizFraction*(maxv - minv);
		_maxValue = maxv - horizFraction*(maxv - minv);
		if(_colorbarWidget) _colorbarWidget->setDirty();
	}

	updateGL();
  
}

//----------------------------------------------------------------------------
// Handle context menu events
//----------------------------------------------------------------------------
void MappingFrame::contextMenuEvent(QContextMenuEvent* /*event*/)
{
  if (_mapper == NULL)
  {
    return;
  }

  OpacityWidget *opacWidget = dynamic_cast<OpacityWidget*>(_lastSelected);

  _contextPoint = QCursor::pos();

  _contextMenu->clear();

  //
  // Opacity widget context
  //
  if (opacWidget)
  {
    if(opacWidget->controlPointSelected())
    {
		_contextMenu->addAction(_editControlPointAction);
		_contextMenu->addAction(_deleteControlPointAction);
		_contextMenu->addSeparator();
    }
	_widgetEnabledSubMenu->actions()[ENABLED]->setEnabled(opacWidget->enabled());
	_widgetEnabledSubMenu->actions()[DISABLED]->setEnabled(!opacWidget->enabled());

    QAction* ac = _contextMenu->addMenu(_widgetEnabledSubMenu);
	ac->setText("Opacity Contribution");
	_contextMenu->addAction(_deleteOpacityWidgetAction);

    _contextMenu->addSeparator();

    if (_colorbarWidget)
    {
	  _contextMenu->addAction(_addColorControlPointAction);
    }
  }
  
  //
  // Colorbar context
  else if (_colorbarWidget && _lastSelected == _colorbarWidget)
  {
    if(_colorbarWidget->controlPointSelected())
    {
		_contextMenu->addAction(_editControlPointAction);
		_contextMenu->addAction(_deleteControlPointAction);
    }
    else
    {
		_contextMenu->addAction(_addColorControlPointAction);
    }    
  }

  //
  // General context
  //
  else
  {
    if (_opacityMappingEnabled)
    {
      QAction* act = _contextMenu->addMenu(_addOpacityWidgetSubMenu);
	  act->setText("New Opacity Widget");
    }

    _contextMenu->addSeparator();

    if (_colorMappingEnabled)
    {
      _contextMenu->addAction(_addColorControlPointAction);
    }
  }

  //
  // Determine if the context point is within the bounds of a ControlPoint
  // widget. 
  //
  QPoint pos = mapFromGlobal(_contextPoint);
  float x = xWorldToData(xViewToWorld(pos.x()));

  map<int, OpacityWidget*>::iterator iter;
  
  for (iter = _opacityWidgets.begin(); iter != _opacityWidgets.end(); iter++)
  {
    OpacityMap *omap = (*iter).second->opacityMap();

    if (omap->GetType() == OpacityMap::CONTROL_POINT && 
        x >= omap->minValue() &&
        x <= omap->maxValue())
    {
      _contextMenu->addAction(_addOpacityControlPointAction);      
      break;
    } 
  }

  _contextMenu->addSeparator();
  QAction* menAct = _contextMenu->addMenu(_histogramScalingSubMenu);
  menAct->setText("Histogram Scaling");

  if (_mapper->getNumOpacityMaps() > 1)
  {
	  QAction* compAct= _contextMenu->addMenu(_compTypeSubMenu);
	  compAct->setText("Opacity Composition");
  }

  _contextMenu->exec(_contextPoint);
}

//----------------------------------------------------------------------------
// Transform the x position in the data (model) space into the opengl world 
// space
//----------------------------------------------------------------------------
float MappingFrame::xDataToWorld(float x)
{
  float minVal = _minValue;
  float maxVal = _maxValue;

  if (maxVal == minVal) return(0.0);
  return (x - minVal) / (maxVal - minVal);
}

//----------------------------------------------------------------------------
// Transform the x position in the opengl world space into the data (model) 
// space
//----------------------------------------------------------------------------
float MappingFrame::xWorldToData(float x)
{
  float minVal = _minValue;
  float maxVal = _maxValue;

  return minVal + (x * (maxVal - minVal));
}

//----------------------------------------------------------------------------
// Transform the x position in view space to the x position in world space
//----------------------------------------------------------------------------
float MappingFrame::xViewToWorld(float x)
{
  if (_maxX <= _minX) return 0.f;
  return _minX + ((x / (float)width()) * (_maxX - _minX)); 
}

//----------------------------------------------------------------------------
// Transform the x position in world space to the x position in view space
//----------------------------------------------------------------------------
float MappingFrame::xWorldToView(float x)
{
  if (_maxX <= _minX) return 0.f;
  return width() * (x - _minX) / (_maxX - _minX);
}

//----------------------------------------------------------------------------
// Transform the y position in the data (model) space into the opengl world 
// space
//----------------------------------------------------------------------------
float MappingFrame::yDataToWorld(float y)
{
  return y;
}

//----------------------------------------------------------------------------
// Transform the y position in the opengl world space into the data (model) 
// space
//----------------------------------------------------------------------------
float MappingFrame::yWorldToData(float y)
{
  if (y < 0.0) return 0.0;
  if (y > 1.0) return 1.0;

  return y;
}

//----------------------------------------------------------------------------
// Transform the y position in view space to the y position in world space
//----------------------------------------------------------------------------
float MappingFrame::yViewToWorld(float y)
{
  assert(height() != 0);
  return _minY + ((y / (float)height()) * (_maxY - _minY)); 
}

//----------------------------------------------------------------------------
// Transform the y position in world space to the y position in view space
//----------------------------------------------------------------------------
float MappingFrame::yWorldToView(float y)
{
  if (_maxY <= _minY) return 0.f;
  return height() * (y - _minY) / (_maxY - _minY);
}

//----------------------------------------------------------------------------
// Return the minimum edit bound
//----------------------------------------------------------------------------
float MappingFrame::getMinEditBound()
{
	if (! _mapper) return(0.0);

	return _mapper->getMinMapValue();
}

//----------------------------------------------------------------------------
// Return the maximum edit bound
//----------------------------------------------------------------------------
float MappingFrame::getMaxEditBound()
{
	if (! _mapper) return(1.0);

	return _mapper->getMaxMapValue();
}

//----------------------------------------------------------------------------
// Set the minimum edit bound
//----------------------------------------------------------------------------
void MappingFrame::setMinEditBound(float val)
{
	if (! _mapper) return;

    _mapper->setMinMapValue(val);
}

//----------------------------------------------------------------------------
// Set the maximum edit bound
//----------------------------------------------------------------------------
void MappingFrame::setMaxEditBound(float val)
{
	if (! _mapper) return;

    _mapper->setMaxMapValue(val);
}

//----------------------------------------------------------------------------
// Return the minimum domain bound
//----------------------------------------------------------------------------
float MappingFrame::getMinDomainBound()
{
	if (! _mapper) return(0.0);

	return _mapper->getMinMapValue();
}


//----------------------------------------------------------------------------
// Return the maximum domain bound
//----------------------------------------------------------------------------
float MappingFrame::getMaxDomainBound()
{
	if (! _mapper) return(1.0);

	return _mapper->getMaxMapValue();
}

//----------------------------------------------------------------------------
// Return the opacity value at the given position
//----------------------------------------------------------------------------
float MappingFrame::getOpacityData(float value)
{
  if (_mapper)
  {
    return _mapper->getOpacityValueData(value);
  }

  return 0.0;
}


//----------------------------------------------------------------------------
// Return the histogram
//----------------------------------------------------------------------------
Histo* MappingFrame::getHistogram()
{
	bool mustGet = _rParams->IsEnabled();
    if (_histogram && !mustGet) return _histogram;
    if (!mustGet) return 0;

	string varname = _rParams->GetColorMapVariableName();
	if (varname == "") {
		varname = _rParams->GetVariableName();
	}
//	string varname = _rParams->GetVariableName();

    MapperFunction* mapFunc = _rParams->MakeMapperFunc(varname);
    if (!mapFunc) return 0;
    if (_histogram) delete _histogram;

    _histogram = new Histo(256,mapFunc->getMinMapValue(),
		mapFunc->getMaxMapValue());
    RefreshHistogram();
    return _histogram;
}

//----------------------------------------------------------------------------
// Add a new opacity widget
//----------------------------------------------------------------------------
void MappingFrame::addOpacityWidget(QAction* act)
{
  if (_mapper)
  {
    emit startChange("Opacity widget creation");

    OpacityMap::Type t    = (OpacityMap::Type)(act->data().toInt());
    OpacityWidget *widget = createOpacityWidget(_mapper->createOpacityMap(t));
    
    connect((QObject*)widget, SIGNAL(startChange(QString)),
            this, SIGNAL(startChange(QString)));
    connect((QObject*)widget, SIGNAL(endChange()),
            this, SIGNAL(endChange()));

    connect((QObject*)widget, SIGNAL(mapChanged()),
            this, SLOT(updateMap()));

    emit endChange();
  }
}

//----------------------------------------------------------------------------
// Add a new color control point
//----------------------------------------------------------------------------
void MappingFrame::addColorControlPoint()
{
  QPoint pos = mapFromGlobal(_contextPoint);
  GLfloat wx = pos.x();

  float x = xWorldToData(xViewToWorld(wx));

  if (_mapper)
  {
    ColorMap *colormap = _mapper->GetColorMap();

    if (colormap && (x >= colormap->minValue() && x <= colormap->maxValue()))
    {
      emit startChange("Add color control point");
      
      colormap->addControlPointAt(x);
      
      emit endChange();
      
      updateGL();
    }
  }
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void MappingFrame::addOpacityControlPoint()
{
  OpacityWidget *opacWidget = dynamic_cast<OpacityWidget*>(_lastSelected);
  QPoint pos = mapFromGlobal(_contextPoint);

  float x = xWorldToData(xViewToWorld(pos.x()));
  float y = yWorldToData(yViewToWorld(height() - pos.y()));

  emit startChange("Opacity control point addition");

  if (opacWidget)
  {
    opacWidget->opacityMap()->addControlPoint(x, y);
  }
  else
  {
    map<int, OpacityWidget*>::iterator iter;
    
    for (iter = _opacityWidgets.begin(); iter != _opacityWidgets.end(); iter++)
    {
      OpacityMap *omap = (*iter).second->opacityMap();
      
      if (omap->GetType() == OpacityMap::CONTROL_POINT && 
          x >= omap->minValue() &&
          x <= omap->maxValue())
      {
        omap->addControlPoint(x,y);
        break;
      } 
    }
  }

  emit endChange();

  updateGL();
}

//----------------------------------------------------------------------------
// Delete selected control points in either the opacity widget or colorbar
//----------------------------------------------------------------------------
void MappingFrame::deleteControlPoint()
{
  makeCurrent();

  OpacityWidget *opacWidget = dynamic_cast<OpacityWidget*>(_lastSelected);

  //
  // Opacity widget
  //
  if (opacWidget)
  {
    startChange("Opacity control point deletion");

    opacWidget->deleteSelectedControlPoint();

    emit endChange();
  }

  //
  // Colorbar widget
  //
  else if (_lastSelected == _colorbarWidget)
  {
    startChange("Color control point deletion");

    _colorbarWidget->deleteSelectedControlPoint();

    emit endChange();
  }    

  updateGL();
}

//----------------------------------------------------------------------------
// Edit the selected control point
//----------------------------------------------------------------------------
void MappingFrame::editControlPoint()
{
  OpacityWidget *opacWidget = dynamic_cast<OpacityWidget*>(_lastSelected);

  //
  // Opacity widget
  //
  if (opacWidget && opacWidget->controlPointSelected())
  {
    ControlPointEditor editor(this, 
                              opacWidget->opacityMap(), 
                              opacWidget->selectedControlPoint());
	emit startChange("Opacity Control Point Edit");
    editor.exec();
	emit endChange();
    updateMap();
    updateGL();
  }

  //
  // Colorbar
  //
  else if (_colorbarWidget && _lastSelected == _colorbarWidget && 
           _colorbarWidget->controlPointSelected())
  {
    ControlPointEditor editor(this, 
                              _colorbarWidget->colormap(), 
                              _colorbarWidget->selectedControlPoint());

    emit startChange("Color Control Point Edit");
    editor.exec();
	emit endChange();

    _colorbarWidget->setDirty();
    
    updateMap();
    updateGL();
  }
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
OpacityWidget* MappingFrame::createOpacityWidget(OpacityMap *omap)
{
  OpacityWidget *widget = new OpacityWidget(this, omap);

  _opacityWidgets[widget->id()] = widget;

  float unitPerPixel = 1.0 / (float)(height()-totalFixedHeight());
  float gap = 5.0 * unitPerPixel;

  widget->setGeometry(_minX, _maxX, 0.0-gap, 1.0+gap);

  return widget;
}

//----------------------------------------------------------------------------
// Delete the selected opacity widget
//----------------------------------------------------------------------------
void MappingFrame::deleteOpacityWidget()
{
  makeCurrent();

  OpacityWidget *opacWidget = dynamic_cast<OpacityWidget*>(_lastSelected);

  if (opacWidget)
  {
    _selectedWidgets.erase(_lastSelected);
    _lastSelected = NULL;

    emit startChange("Opacity widget deletion");

    _opacityWidgets.erase(opacWidget->id());

    _mapper->DeleteOpacityMap(opacWidget->opacityMap());

    delete _lastSelected;
    _lastSelected = NULL;

    emit endChange();
  }
}

//----------------------------------------------------------------------------
// Handle domain widget interactions.
//----------------------------------------------------------------------------
void MappingFrame::setDomain()
{
  float min = xWorldToData(_domainSlider->minValue());
  float max = xWorldToData(_domainSlider->maxValue());

  if (_mapper)
  {
	  if (_opacityMappingEnabled || _colorMappingEnabled) {
    
	  emit startChange("Mapping window boundary change");

      _mapper->setMinMaxMapValue(min,max);
    
	  emit endChange();
	 }

    updateGL();
  }
  else
  {
    _domainSlider->setDomain(xDataToWorld(_minValue),
                             xDataToWorld(_maxValue));
  }
}

//----------------------------------------------------------------------------
// Deal with iso slider movement
//----------------------------------------------------------------------------
void MappingFrame::setIsoSlider()
{
  if (!_mapper) return;
  /* Only used by isovalues
  float min = xWorldToData(_isoSlider->minValue());
  float max = xWorldToData(_isoSlider->maxValue());
  
  emit startChange("Slide Isovalue slider");
  RenderParams *params = _mapper->GetActiveParams();
  ParamsIso* iParams = (ParamsIso*)params;
  iParams->SetIsoValue((0.5*(max+min)));
  */
  emit endChange();

  updateGL();
  
}
//----------------------------------------------------------------------------
// Deal with isoline slider movement
//----------------------------------------------------------------------------
void MappingFrame::setIsolineSlider(int index)
{
#ifdef	DEAD
  if (!_mapper) return;
  IsoSlider* iSlider = _isolineSliders[index];
  float min = xWorldToData(iSlider->minValue());
  float max = xWorldToData(iSlider->maxValue());
  
  emit startChange("Slide Isoline value slider");
  IsolineParams* iParams = (IsolineParams*) GetActiveParams();
  vector<double> isovals = iParams->GetIsovalues();
  isovals[index] = (0.5*(max+min));
  iParams->SetIsovalues(isovals);
  
  emit endChange();

  updateGL();
#endif
  
}
void MappingFrame::paintEvent(QPaintEvent* event)
{
	printOpenGLErrorMsg("MappingFramePaintEvent");

	  QGLWidget::paintEvent(event);
	  printOpenGLErrorMsg("MappingFramePaintEvent");
	
}
void MappingFrame::updateGL(){
	
	QGLWidget::updateGL();
	return;
}

void MappingFrame::setIsolineSliders(const vector<double>& sliderVals){
	//delete unused sliders
	if (sliderVals.size() < _isolineSliders.size()){
		for (int i = sliderVals.size(); i< _isolineSliders.size();i++) {
			delete _isolineSliders[i];
		}
		_isolineSliders.resize(sliderVals.size());
	} else if (sliderVals.size() > _isolineSliders.size()){
		//create new ones:
		for (int i = _isolineSliders.size(); i<sliderVals.size(); i++)
			_isolineSliders.push_back(new IsoSlider(this));
	}
	//set the isovalues
	for (int i = 0; i< _isolineSliders.size(); i++){ 
		_isolineSliders[i]->setIsoValue(xDataToWorld(sliderVals[i]));
	}
}

void MappingFrame::updateHisto() {
	fitToView();
	updateMap();
}

void MappingFrame::setNavigateMode(bool mode){
	setEditMode(!mode); 
	_editButton->setChecked(!mode);
}
