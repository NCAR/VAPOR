//--MappingFrame.h ---------------------------------------------------------
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

#ifndef MappingFrame_H
#define MappingFrame_H

#include <GL/glew.h>
#ifdef Darwin
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <qgl.h>

#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include "vapor/Visualizer.h"

#include <qpoint.h>
#include <list>
#include <map>
#include <set>


class QAction;
class QLabel;
class QMenu;
class QPushButton;
class QSlider;
class QTabWidget;

class OpacityWidget;
class GLColorbarWidget;
class Histo;
class DomainWidget;
class IsoSlider;
class GLWidget;
class RenderEventRouter;

namespace VAPoR {
	class MapperFunction;
	class RenderParams;
	class OpacityMap;
	class ControlExec;
};

//! \class MappingFrame
//! \ingroup Public_GUI
//! \brief A QGLWidget that displays a Transfer Function Editor, or an Iso Selection Window
//! \author Alan Norton
//! \version 3.0
//! \date    May 2015

//!	The MappingFrame is a QGLWidget that can be used to map colors and/or opacity from a 1D interval.
//! The MappingFrame can be inserted in the VAPOR tabs using QDesigner.  Several Signal/Slot connections
//! must be implemented to support editing functionality.
//! 

class MappingFrame : public QGLWidget
{
  Q_OBJECT

  Q_PROPERTY(bool colorMapping   READ colorMapping   WRITE setColorMapping)
  Q_PROPERTY(bool opacityMapping READ opacityMapping WRITE setOpacityMapping)

  enum
  {
    BOOLEAN=0,
    LINEAR=1,
    LOG=2
  };

  enum
  {
    ENABLED=0,
    DISABLED=1
  };

  enum
  {
    OPACITY_WIDGETS,
    DOMAIN_WIDGET,
    COLORBAR_WIDGET,
	ISO_WIDGET
  };

public:

  MappingFrame(QWidget* parent);
  virtual ~MappingFrame();

  void RefreshHistogram();
 
  //! Enable or disable the color mapping in the Transfer Function.
  //! Should be specified in the RenderEventRouter constructor
  //! \param[in] flag set true if color mapping will be enabled.
  void setColorMapping(bool flag);
   
  //! Enable or disable the opacity mapping in the Transfer Function.
  //! Should be specified in the RenderEventRouter constructor
  //! \param[in] flag set true if opacity mapping will be enabled.
  void setOpacityMapping(bool flag);

  //! Enable or disable the use of a single iso slider in an IsoControl editor
  //! Should be specified in the RenderEventRouter constructor
  //! \param[in] flag set true if one iso Slider will be enabled.
  void setIsoSlider(bool flag) {_isoSliderEnabled = flag;}

  //! Enable or disable the use of multiple iso sliders in an IsoControl editor
  //! Should be specified in the RenderEventRouter constructor
  //! \param[in] flag set true if multiple iso Sliders will be enabled.
  void setIsolineSliders(bool flag) {_isolineSlidersEnabled = flag;}

  //! Specify the set of values that will be associated with multiple Iso Sliders
  //! Should be specified whenever the Isovalues are changed.
  //! \param[in] slidervals is a vector of iso values
  void setIsolineSliders(const vector<double>& slidervals);

  //! Update the display of the Transfer Function or IsoControl based on the current RenderParams that
  //! contains the MapperFunction being used.  This should be invoked in RenderEventRouter::updateTab()
  //void updateTab();
  void Update(VAPoR::DataMgr *dataMgr=NULL,
	VAPoR::ParamsMgr *paramsMgr=NULL,
	VAPoR::RenderParams *rParams=NULL);

  //! Specify the variable associated with the MappingFrame.  Invoked in RenderEventRouter::setEditorDirty()
  void setVariableName(std::string name);


  //! Identify the current mapperFunction associated with the MappingFrame.
  //! Needed by various GLWidgets embedded in the MappingFrame
  //! \return MapperFunction* associated with this MappingFrame.
  VAPoR::MapperFunction* mapperFunction() { return _mapper; }

  //! Identify the minimum data value associated with the MappingFrame.
  //! Needed by various GLWidgets embedded in the MappingFrame
  //! \return minimum data bound associated with this MappingFrame.
  float minDataValue() { return _minValue; }
  
  //! Identify the maximum data value associated with the MappingFrame.
  //! Needed by various GLWidgets embedded in the MappingFrame
  //! \return maximum data bound associated with this MappingFrame.
  float maxDataValue() { return _maxValue; }

  virtual float getMinEditBound();
  virtual float getMaxEditBound();
  //Identify the position in the containing tabwidget
  void setTabWidget(QTabWidget* wid, int posn){
	  _myTabWidget = wid; _myTabPosn = posn;
  }
  //static void SetControlExec(VAPoR::ControlExec* ce){_controlExec = ce;}

signals:

   //! Signal that is invoked when user starts to modify the transfer function.
  //! This should be connected to a slot in the RenderEventRouter that creates a Command that
  //! will be used for putting the edit changes into the Command queue.
  //! \param[in] description of the change that is occurring
  void startChange(QString description);

  //! Signal that is invoked when user ends modifying the transfer function.
  //! This should be connected to a slot in the RenderEventRouter that completes and saves the Command that
  //! is used for saving the edit changes in the Command queue.
  void endChange();
  //
  //! Signal indicates that the mapping function has changed, update the visualizer
  //! on this signal if you want transfer function edits visualized in real-time
  //
  void mappingChanged();

  void updateParams(); 

public slots:
	void updateHisto();
	void fitToView();
	void updateMap();

private:
  void updateMapperFunction(VAPoR::MapperFunction *mapper);

  bool colorMapping() const { return _colorMappingEnabled; }
  bool opacityMapping() const { return _opacityMappingEnabled; }
  bool isoSliderEnabled() const { return _isoSliderEnabled; }
  bool isolineSlidersEnabled() const { return _isolineSlidersEnabled; }
  void setIsoValue(float val){_isoVal = val;}
  QString tipText(const QPoint &pos, bool isIso=false);
  int   histoValue(const QPoint &pos);
  float xVariable(const QPoint &pos);
  float yVariable(const QPoint &pos);
  bool  canBind();
  
protected slots:
  void setEditMode(bool);
  void setNavigateMode(bool mode);
  void setHistogramScale(QAction*);
  void setCompositionType(QAction*);
  void setWidgetEnabled(QAction*);
  
  void newHsv(int h, int s, int v);
  void bindColorToOpacity();
  void bindOpacityToColor();
  

signals:
  //
  // Signals that a color control point has been selected
  //
  void sendRgb(QRgb color);

  //
  // Signals that one color control point and one opacity control point have
  // been selected.
  //
  void canBindControlPoints(bool);

 
private:

  void initWidgets();
  void initConnections();

  OpacityWidget* createOpacityWidget(VAPoR::OpacityMap *map);
  void deleteOpacityWidgets();

  void initializeGL();
  void paintGL();
  void resizeGL( int w, int h );
  //Virtual, Reimplemented here:

  int drawHistogram();
  int drawOpacityCurve();
  int drawOpacityWidgets();
  int drawDomainSlider();
  int drawIsoSlider();
  int drawIsolineSliders();
  int drawColorbar();

  void updateTexture();

  void updateAxisLabels();
  void addAxisLabel(int x, int y, const QString &text);

  void select(int x, int y, Qt::KeyboardModifiers);
  void select(int hits, GLuint *selectionBuffer, Qt::KeyboardModifiers);

  void deselectWidgets();

  int  totalFixedHeight();
  void resize();

  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void mouseDoubleClickEvent(QMouseEvent *event);
  virtual void contextMenuEvent(QContextMenuEvent *e);

  float xDataToWorld(float x);
  float xWorldToData(float x);
  float xViewToWorld(float x);
  float xWorldToView(float x);

  float yDataToWorld(float y);
  float yWorldToData(float y);
  float yViewToWorld(float y);
  float yWorldToView(float y);

 
  virtual void  setMinEditBound(float v);
  virtual void  setMaxEditBound(float v);

  virtual float getMinDomainBound();
  virtual float getMaxDomainBound();

  virtual float getOpacityData(float val);
  virtual Histo* getHistogram();

protected slots:

  void addOpacityWidget(QAction*);
  void deleteOpacityWidget();

  void addColorControlPoint();

  void addOpacityControlPoint();
  void editControlPoint();
  void deleteControlPoint();

  void setDomain();
  void setIsoSlider();
  void setIsolineSlider(int sliderIndex);

private:
  static VAPoR::ControlExec* _controlExec;
  const int _NUM_BINS;

  int _myTabPosn;
  QTabWidget* _myTabWidget;
  VAPoR::MapperFunction *_mapper;
  Histo          *_histogram;

  bool            _opacityMappingEnabled;
  bool            _colorMappingEnabled;
  bool			  _isoSliderEnabled;
  bool			  _isolineSlidersEnabled;
  vector<IsoSlider*> _isolineSliders;
  int			_lastSelectedIndex;
  QPushButton*	navigateButton;
  QPushButton*  _editButton;


  std::string     _variableName;

  std::map<int, OpacityWidget*> _opacityWidgets;
  DomainWidget                 *_domainSlider;
  IsoSlider					   *_isoSlider;
  GLColorbarWidget               *_colorbarWidget;
  GLWidget                     *_lastSelected;
  std::set<GLWidget*>           _selectedWidgets;

  unsigned int   _texid;
  unsigned char *_texture;
  bool           _updateTexture;
  int            _histogramScale;

    
  QPoint      _contextPoint;
  QMenu *_contextMenu;
  QMenu *_addOpacityWidgetSubMenu;
  QMenu *_histogramScalingSubMenu;
  QMenu *_compTypeSubMenu;
  QMenu *_widgetEnabledSubMenu;
  QAction    *_editOpacityWidgetAction;
  QAction    *_deleteOpacityWidgetAction;
  QAction    *_addColorControlPointAction;
  QAction    *_addOpacityControlPointAction;
  QAction    *_editControlPointAction;
  QAction    *_deleteControlPointAction;

  float _lastx;
  float _lasty;

  bool   _editMode;
  QPoint _clickedPos;

  float  _minValueStart;
  float  _maxValueStart;
  float  _isoVal;

  Qt::MouseButtons _button;

  float _minX;
  float _maxX;
  float _minY;
  float _maxY;

  float _minValue;
  float _maxValue;

  const int _colorbarHeight;
  const int _domainBarHeight;
  const int _domainLabelHeight;
  const int _domainHeight;
  int _axisRegionHeight;
  const int _opacityGap;
  const int _bottomGap;
  VAPoR::DataMgr *_dataMgr;
  VAPoR::RenderParams *_rParams;
  VAPoR::ParamsMgr *_paramsMgr;

  QStringList _axisTexts;
  QList<QPoint*> _axisTextPos;


};

#endif // MappingFrame_H
