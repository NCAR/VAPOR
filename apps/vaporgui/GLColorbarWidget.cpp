//-- GLColorbarWidget.cpp -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// OpenGL-based colorbar with interactive color control points.
// 
//----------------------------------------------------------------------------

#include <vapor/glutil.h>	// Must be included first!!!
#include <vapor/ColorMap.h>
#include "GLColorbarWidget.h"
#include "MappingFrame.h"

#include <iostream>

using namespace std;
using namespace VAPoR;


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
GLColorbarWidget::GLColorbarWidget(MappingFrame *parent, ColorMap *colormap) :
  GLWidget(parent),
  _NUM_BINS(256),
  _parent(parent),
  _colormap(colormap),
  _texid(0),
  _texture(new unsigned char[_NUM_BINS*4]),
  _updateTexture(true),
  _minValue(0.0),
  _maxValue(1.0)
{
  _minX = -0.03;
  _maxX =  1.03;
  _minY = -0.3;
  _maxY = -0.1;
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
GLColorbarWidget::~GLColorbarWidget()
{
  _parent->makeCurrent();

  delete [] _texture;
  glDeleteTextures(1, &_texid);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void GLColorbarWidget::setColormap(ColorMap *colormap)
{
  _colormap = colormap;
  vector<double> cps = colormap->GetControlPoints();
  assert(cps.size()>0);
  _updateTexture = true;
}

//----------------------------------------------------------------------------
// Return colorbar width in world coordinates
//----------------------------------------------------------------------------
float GLColorbarWidget::width()
{
  return right() - left();
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void GLColorbarWidget::move(float dx, float, float)
{
  float scaling = (_parent->maxDataValue() - _parent->minDataValue());

  if (_colormap && _selected != NONE)
  {
    set<int>::iterator iter;

    for (iter=_selectedCPs.begin(); iter!=_selectedCPs.end(); iter++)
    {
      _colormap->move(*iter, dx*scaling);
    }

    _updateTexture = true;
  }

  emit mapChanged();
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void GLColorbarWidget::drag(float dx, float, float)
{
  float scaling = (_parent->maxDataValue() - _parent->minDataValue());

  if (_colormap && _selected != NONE)
  {
    set<int>::iterator iter;

    for (iter=_selectedCPs.begin(); iter!=_selectedCPs.end(); iter++)
    {
      _colormap->move(*iter, dx*scaling);
    }

    _updateTexture = true;
  }

  emit mapChanged();
}

//----------------------------------------------------------------------------
// Render the colorbar
//----------------------------------------------------------------------------
int GLColorbarWidget::paintGL()
{
  int rc = printOpenGLErrorMsg("GLColorbarWidget");
  if (rc < 0) return -1;
  float offset = (_maxY - _minY) * 0.2;

  if (_colormap)
  {
    glColor3f(1.0, 1.0, 1.0);

    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, _texid);

    if (_updateTexture)
    {
      updateTexture();
    }

	glBegin(GL_QUADS);
    {
      glTexCoord1f(0.0f);
      glVertex3f(_minX, _maxY-offset, 0.09);
      
      glTexCoord1f(0.0f);
      glVertex3f(_minX, _minY+offset, 0.09);
      
      glTexCoord1f(1.0f);
      glVertex3f(_maxX, _minY+offset, 0.09);
      
      glTexCoord1f(1.0f); 
      glVertex3f(_maxX, _maxY-offset, 0.09); 
    }
	glEnd();

    glDisable(GL_TEXTURE_1D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_SMOOTH);

    for (int i=0; i < _colormap->numControlPoints(); i++)
    {
      drawControlPoint(i);
    }

    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_BLEND);
  }

  rc = printOpenGLErrorMsg("GLColorbarWidget");
  if (rc < 0) return -1;
  return 0;

}

//----------------------------------------------------------------------------
// Set up the OpenGL rendering state
//----------------------------------------------------------------------------
void GLColorbarWidget::initializeGL()
{
  printOpenGLErrorMsg("GLColorbarWidgetInitialize");
 
  glShadeModel( GL_SMOOTH );
  glPolygonMode(GL_FRONT, GL_FILL);

  glGenTextures(1, &_texid);
  glBindTexture(GL_TEXTURE_1D, _texid);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

  updateTexture();
  printOpenGLErrorMsg("GLColorbarWidget");
}


//----------------------------------------------------------------------------
// Render an individual control point
//----------------------------------------------------------------------------
void GLColorbarWidget::drawControlPoint(int index)
{
  if (_colormap)
  {
    if (_selectedCPs.find(index) != _selectedCPs.end())
    {
      glColor3f(1.0, 1.0, 0.0);
    }
    else
    {
      glColor3f(0.5, 0.5, 0.5);
    }
    
    float x = dataToWorld(_colormap->controlPointValue(index));

    float halfW = 0.010;

    glPushName(index);

    glBegin(GL_TRIANGLES);
    {
      glVertex3f(x-halfW, _minY, 0.1);
      glVertex3f(x+halfW, _minY, 0.1);
      glVertex3f(x, (_maxY + _minY)/2.0, 0.1);
      
      glVertex3f(x-halfW, _maxY, 0.1);
      glVertex3f(x, (_maxY + _minY)/2.0, 0.1);    
      glVertex3f(x+halfW, _maxY, 0.1);
    }
    glEnd();

    glPopName();
  }
}

//----------------------------------------------------------------------------
// Construct/Re-construct the colorbar texture
//----------------------------------------------------------------------------
void GLColorbarWidget::updateTexture()
{
  if (_colormap)
  {
    float step = 
      (_parent->maxDataValue() - _parent->minDataValue())/(_NUM_BINS-1);
    
    for (int i=0; i<_NUM_BINS; i++)
    {
      float value = _parent->minDataValue() + i*step;
      float rgb[3];
      
      _colormap->color(value).toRGB(rgb);

      _texture[i*4+0] = (int)(255*rgb[0]);
      _texture[i*4+1] = (int)(255*rgb[1]);
      _texture[i*4+2] = (int)(255*rgb[2]);
      _texture[i*4+3] = 255;
    }
  }

  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, 
               GL_UNSIGNED_BYTE, _texture);

  _updateTexture = false;
}

//----------------------------------------------------------------------------
// Select the widget's part given the opengl selection name and qt button 
// state. 
//----------------------------------------------------------------------------
void GLColorbarWidget::select(int handle, Qt::KeyboardModifiers state) 
{
  if (!(state & (Qt::ShiftModifier|Qt::ControlModifier)))
  {
    _selectedCPs.clear();
  }

  //
  // Control button
  //
  if (state & Qt::ControlModifier)
  {
    if (_selectedCPs.find(handle) != _selectedCPs.end())
    {
      //
      // Clicked on an already selected control point, deselect it. 
      //
      _selectedCPs.erase(handle);

      if (_selectedCPs.size())
      {
        _selected = *_selectedCPs.begin();
      }
      else
      {
        _selected = NONE;
      }
    }
    else
    {
      _selected = handle;
      _selectedCPs.insert(_selected);
    }
    
    return;
  }

  //
  // Shift button
  //
  if (state & Qt::ShiftModifier)
  {
    _selected = handle;
    
    if (_selectedCPs.find(handle) != _selectedCPs.end())
    {
      return; // Already selected
    }
    
    set<int>::iterator iter = _selectedCPs.insert(handle).first;
    
    set<int>::iterator loweri = iter;
    set<int>::iterator upperi = iter;

    int lower = (loweri != _selectedCPs.begin()) ? *(--loweri) : handle;
    int upper = (++upperi != _selectedCPs.end()) ? *upperi : handle;
    
    for (int i=lower; i<=upper; i++)
    {
      _selectedCPs.insert(i);
    }
    
    return;
  }
  
  _selected = handle;
  _selectedCPs.insert(handle);

  if (_colormap)
  {
    ColorMap::Color color = _colormap->controlPointColor(handle);
    QColor qcolor;
    qcolor.setHsv((int)(360*color.hue()),
                  (int)(255*color.sat()),
                  (int)(255*color.val()));
    emit sendRgb(qcolor.rgb());
  }
}


//----------------------------------------------------------------------------
// Deselect this widget and any selected control points. 
//----------------------------------------------------------------------------
void GLColorbarWidget::deselect()
{
  _selected = NONE; 
  _selectedCPs.clear();
}

//----------------------------------------------------------------------------
// Return the positions of the selected control points. 
//----------------------------------------------------------------------------
list<float> GLColorbarWidget::selectedPoints()
{
  set<int>::iterator iter;
  list<float> points;

  if (_colormap)
  {
    for (iter=_selectedCPs.begin(); iter!=_selectedCPs.end(); iter++)
    {
      points.push_back(_colormap->controlPointValue(*iter));
    }
  }

  return points;
}


//----------------------------------------------------------------------------
// Delete the selected control points. 
//----------------------------------------------------------------------------
void GLColorbarWidget::deleteSelectedControlPoint()
{
  if (_colormap && selected() && _colormap->numControlPoints() > 2)
  {
    emit startChange("Delete color control point");

    _colormap->deleteControlPoint(_selected);

    _selected = NONE;
	_selectedCPs.clear();
    _updateTexture = true;

    emit endChange();
  }
  
}

//----------------------------------------------------------------------------
// Assign a new color (h,s,v) to all selected control points. 
//----------------------------------------------------------------------------
void GLColorbarWidget::newHsv(int h, int s, int v)
{
  if (_colormap)
  {
    if (_selectedCPs.size())
    {
      float hf = (float)h/360.0;
      float sf = (float)s/255.0;
      float vf = (float)v/255.0;
      
      set<int>::iterator iter;
    
      for (iter=_selectedCPs.begin(); iter!=_selectedCPs.end(); iter++)
      {
        _colormap->controlPointColor(*iter, ColorMap::Color(hf, sf ,vf));
      }
      
      _updateTexture = true;
    }
  }
}

//----------------------------------------------------------------------------
// Rightmost edge of colorbar in world coordinates
//----------------------------------------------------------------------------
float GLColorbarWidget::right()
{
  return dataToWorld(_parent->maxDataValue());
}

//----------------------------------------------------------------------------
// Leftmost edge of colorbar in world coordinates
//----------------------------------------------------------------------------
float GLColorbarWidget::left()
{
  return dataToWorld(_parent->minDataValue());
}

//----------------------------------------------------------------------------
// Transform the x position in the data (model) space into the opengl world 
// space 
//----------------------------------------------------------------------------
float GLColorbarWidget::dataToWorld(float x)
{
  return (x - _parent->minDataValue()) / 
    (_parent->maxDataValue() - _parent->minDataValue());
}

//----------------------------------------------------------------------------
// Transform the x position in the opengl world space into the data (model) 
// space
//----------------------------------------------------------------------------
float GLColorbarWidget::worldToData(float x)
{
  return _parent->minDataValue() + 
    (x * (_parent->maxDataValue() - _parent->minDataValue()));
}
