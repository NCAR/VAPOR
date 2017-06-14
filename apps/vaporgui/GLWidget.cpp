//-- GLWidget.cpp -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// Abstract base class for OpenGL-based widgets that provides a common
// interface for selecting, moving, and drawing.
//
//----------------------------------------------------------------------------

#include "GLWidget.h"

#include <qwidget.h>

using namespace std;

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
GLWidget::GLWidget(QWidget *parent) : QObject(parent), _id(createId()), _selected(NONE), _enabled(true) {}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
GLWidget::~GLWidget() {}

//----------------------------------------------------------------------------
// Set the widget's geometry
//----------------------------------------------------------------------------
void GLWidget::setGeometry(float x0, float x1, float y0, float y1)
{
    _minX = x0 <= x1 ? x0 : x1;
    _maxX = x1 > x0 ? x1 : x0;

    _minY = y0 <= y1 ? y0 : y1;
    _maxY = y1 > y0 ? y1 : y0;
}

//- static -------------------------------------------------------------------
//
// Generate a unique identifier.
//----------------------------------------------------------------------------
unsigned int GLWidget::createId()
{
    static unsigned int id = 1;

    return id++;
}
