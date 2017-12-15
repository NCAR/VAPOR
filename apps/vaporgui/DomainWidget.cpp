//-- DomainWidget.cpp -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// OpenGL-based widget used to scale and move the a transfer function domain.
//
//----------------------------------------------------------------------------

#include <vapor/glutil.h>    // Must be included first!!!
#include "DomainWidget.h"
#include "MappingFrame.h"

#include <iostream>

using namespace std;
using namespace VAPoR;

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
DomainWidget::DomainWidget(QWidget *parent, Qt::Orientation orientation, float min, float max)
: GLWidget(parent), _minValue(min), _maxValue(max), _handleRadius(0.06), _quadHandle(gluNewQuadric()), _orientation(orientation)
{
    _minY = 1.14;
    _maxY = 1.3;
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
DomainWidget::~DomainWidget() {}

//----------------------------------------------------------------------------
// Widget's width in the world coordinate frame
//----------------------------------------------------------------------------
float DomainWidget::width() { return _maxValue - _minValue; }

//----------------------------------------------------------------------------
// Move the entire domain
//----------------------------------------------------------------------------
void DomainWidget::move(float dx, float dy, float)
{
    if (_selected != NONE) {
        _minValue += (_orientation == Qt::Horizontal) ? dx : dy;
        _maxValue += (_orientation == Qt::Horizontal) ? dx : dy;

        emit changingDomain(_minValue, _maxValue);
    }
}

//----------------------------------------------------------------------------
// Drag an element of the domain.
//----------------------------------------------------------------------------
void DomainWidget::drag(float dx, float dy, float)
{
    if (_selected == LEFT && _minValue + dx < _maxValue) {
        _minValue += (_orientation == Qt::Horizontal) ? dx : dy;
    }

    else if (_selected == RIGHT && _maxValue + dx > _minValue) {
        _maxValue += (_orientation == Qt::Horizontal) ? dx : dy;
    }

    else if (_selected == BAR) {
        _minValue += (_orientation == Qt::Horizontal) ? dx : dy;
        _maxValue += (_orientation == Qt::Horizontal) ? dx : dy;
    }

    emit changingDomain(_minValue, _maxValue);
}

//----------------------------------------------------------------------------
// Render the widget
//----------------------------------------------------------------------------
int DomainWidget::paintGL()
{
    int rc = printOpenGLErrorMsg("DomainWidget");
    if (rc < 0) return -1;
    float length = 0.03;

    glPushName(_id);
    glPushMatrix();
    {
        if (_enabled) {
#ifdef Darwin
            //
            // Mac version is not rendering the gluCylinders for some
            // reason. Replace with simplier geometry for the Mac.
            //
            if (_orientation == Qt::Horizontal) {
                float y = (_maxY + _minY) / 2.0;
                cout << "miny/maxy " << _minY << " " << _maxY << endl;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(_minValue - length, y, 0.0);
                    glVertex3f(_minValue, y + _handleRadius, 0.0);
                    glVertex3f(_minValue, y - _handleRadius, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(_maxValue + length, y, 0.0);
                    glVertex3f(_maxValue, y + _handleRadius, 0.0);
                    glVertex3f(_maxValue, y - _handleRadius, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glBegin(GL_QUADS);
                    glVertex3f(_minValue, y + 0.4 * _handleRadius, 0.0);
                    glVertex3f(_maxValue, y + 0.4 * _handleRadius, 0.0);
                    glVertex3f(_maxValue, y - 0.4 * _handleRadius, 0.0);
                    glVertex3f(_minValue, y - 0.4 * _handleRadius, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();
            } else    // Vertical
            {
                float x = 1 - (_maxY + _minY) / 2.0;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(x, _minValue - length, 0.0);
                    glVertex3f(x + _handleRadius, _minValue, 0.0);
                    glVertex3f(x - _handleRadius, _minValue, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(x, _maxValue + length, 0.0);
                    glVertex3f(x + _handleRadius, _maxValue, 0.0);
                    glVertex3f(x - _handleRadius, _maxValue, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glBegin(GL_QUADS);
                    glVertex3f(x + 0.4 * _handleRadius, _minValue, 0.0);
                    glVertex3f(x + 0.4 * _handleRadius, _maxValue, 0.0);
                    glVertex3f(x - 0.4 * _handleRadius, _maxValue, 0.0);
                    glVertex3f(x - 0.4 * _handleRadius, _minValue, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();
            }
#else
            if (_orientation == Qt::Horizontal) {
                float y = (_maxY + _minY) / 2.0;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glTranslatef(_minValue - length, y, 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_quadHandle, 0.0, _handleRadius, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glTranslatef(_maxValue, y, 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_quadHandle, _handleRadius, 0.0, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glTranslatef(_minValue, y, 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_quadHandle, 0.4 * _handleRadius, 0.4 * _handleRadius, width(), 10, 2);

                    glPopName();
                }
                glPopMatrix();
            } else    // Vertical
            {
                float x = 1 - (_maxY + _minY) / 2.0;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glTranslatef(x, _minValue - length, 0.0);
                    glRotatef(90, -1, 0, 0);
                    gluCylinder(_quadHandle, 0.0, _handleRadius, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glTranslatef(x, _maxValue, 0.0);
                    glRotatef(90, -1, 0, 0);
                    gluCylinder(_quadHandle, _handleRadius, 0.0, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glTranslatef(x, _minValue, 0.0);
                    glRotatef(90, -1, 0, 0);
                    gluCylinder(_quadHandle, 0.4 * _handleRadius, 0.4 * _handleRadius, width(), 10, 2);

                    glPopName();
                }
                glPopMatrix();
            }
#endif
        }
    }
    glPopMatrix();
    glPopName();

    rc = printOpenGLErrorMsg("DomainWidget");
    if (rc < 0) return -1;
    return 0;
}

//----------------------------------------------------------------------------
// Set the widget's geometry
//----------------------------------------------------------------------------
void DomainWidget::setGeometry(float x0, float x1, float y0, float y1)
{
    GLWidget::setGeometry(x0, x1, y0, y1);

    _handleRadius = (_maxY - _minY) * 0.375;
}

//----------------------------------------------------------------------------
// center of the widget in world coordinates
//----------------------------------------------------------------------------
float DomainWidget::mid() { return (0.5 * (_minValue + _maxValue)); }

//############################################################################
// IsoSlider
//############################################################################

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
IsoSlider::IsoSlider(QWidget *parent, float min, float max) : DomainWidget(parent, Qt::Horizontal, min, max)
{
    _minValue = 0.47;
    _maxValue = 0.53;
    _minY = -.10;
    _maxY = 0.06;
    _lineWidth = 0.02;
}

//----------------------------------------------------------------------------
// Drag an element of the Iso-domain.
//----------------------------------------------------------------------------
void IsoSlider::drag(float dx, float, float)
{
    _minValue += dx;
    _maxValue += dx;

    emit changingDomain(_minValue, _maxValue);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int IsoSlider::paintGL()
{
    DomainWidget::paintGL();

    glPushMatrix();
    glPushName(VERTLINE);
    {
        glColor3f(1., 1., 1.);

        glBegin(GL_QUADS);
        glNormal3f(0., 0., 1.);
        glVertex3f(mid() - 0.1 * _lineWidth, 0.0, 0.0);
        glVertex3f(mid() + 0.1 * _lineWidth, 0.0, 0.0);
        glVertex3f(mid() + 0.1 * _lineWidth, 0.8, 0.0);
        glVertex3f(mid() - 0.1 * _lineWidth, 0.8, 0.0);

        glEnd();
    }
    glPopMatrix();
    glPopName();
    int rc = printOpenGLError();
    if (rc < 0) return -1;
    return 0;
}

ContourRangeSlider::ContourRangeSlider(QWidget *parent, float min, float max) : DomainWidget(parent, Qt::Horizontal, min, max)
{
    _minY = 1.0;
    _maxY = 1.02;
}

//----------------------------------------------------------------------------
// Render the widget
//----------------------------------------------------------------------------
int ContourRangeSlider::paintGL()
{
    int rc = printOpenGLErrorMsg("DomainWidget");
    if (rc < 0) return -1;
    float length = 0.03;

    glPushName(_id);
    glPushMatrix();
    {
        if (_enabled) {
#ifdef Darwin
            //
            // Mac version is not rendering the gluCylinders for some
            // reason. Replace with simplier geometry for the Mac.
            //
            if (_orientation == Qt::Horizontal) {
                float y = (_maxY + _minY) / 2.0;
                cout << "crs miny/maxy " << _minY << " " << _maxY << endl;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(_minValue - length, y, 0.0);
                    glVertex3f(_minValue, y + _handleRadius, 0.0);
                    glVertex3f(_minValue, y - _handleRadius, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(_maxValue + length, y, 0.0);
                    glVertex3f(_maxValue, y + _handleRadius, 0.0);
                    glVertex3f(_maxValue, y - _handleRadius, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glBegin(GL_QUADS);
                    glVertex3f(_minValue, y + 0.4 * _handleRadius, 0.0);
                    glVertex3f(_maxValue, y + 0.4 * _handleRadius, 0.0);
                    glVertex3f(_maxValue, y - 0.4 * _handleRadius, 0.0);
                    glVertex3f(_minValue, y - 0.4 * _handleRadius, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();
            } else    // Vertical
            {
                float x = 1 - (_maxY + _minY) / 2.0;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(x, _minValue - length, 0.0);
                    glVertex3f(x + _handleRadius, _minValue, 0.0);
                    glVertex3f(x - _handleRadius, _minValue, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(x, _maxValue + length, 0.0);
                    glVertex3f(x + _handleRadius, _maxValue, 0.0);
                    glVertex3f(x - _handleRadius, _maxValue, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glBegin(GL_QUADS);
                    glVertex3f(x + 0.4 * _handleRadius, _minValue, 0.0);
                    glVertex3f(x + 0.4 * _handleRadius, _maxValue, 0.0);
                    glVertex3f(x - 0.4 * _handleRadius, _maxValue, 0.0);
                    glVertex3f(x - 0.4 * _handleRadius, _minValue, 0.0);
                    glEnd();
                    glPopName();
                }
                glPopMatrix();
            }
#else
            if (_orientation == Qt::Horizontal) {
                float y = (_maxY + _minY) / 2.0;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glTranslatef(_minValue - length, y, 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_quadHandle, 0.0, _handleRadius, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glTranslatef(_maxValue, y, 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_quadHandle, _handleRadius, 0.0, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glTranslatef(_minValue, y, 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_quadHandle, 0.4 * _handleRadius, 0.4 * _handleRadius, width(), 10, 2);

                    glPopName();
                }
                glPopMatrix();
            } else    // Vertical
            {
                float x = 1 - (_maxY + _minY) / 2.0;

                glColor3f(1, 0.3, 0.3);

                glPushMatrix();
                {
                    glPushName(LEFT);
                    glTranslatef(x, _minValue - length, 0.0);
                    glRotatef(90, -1, 0, 0);
                    gluCylinder(_quadHandle, 0.0, _handleRadius, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glPushMatrix();
                {
                    glPushName(RIGHT);
                    glTranslatef(x, _maxValue, 0.0);
                    glRotatef(90, -1, 0, 0);
                    gluCylinder(_quadHandle, _handleRadius, 0.0, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();

                glColor3f(0.7, 0.0, 0.0);

                glPushMatrix();
                {
                    glPushName(BAR);
                    glTranslatef(x, _minValue, 0.0);
                    glRotatef(90, -1, 0, 0);
                    gluCylinder(_quadHandle, 0.4 * _handleRadius, 0.4 * _handleRadius, width(), 10, 2);

                    glPopName();
                }
                glPopMatrix();
            }
#endif
        }
    }
    glPopMatrix();
    glPopName();

    rc = printOpenGLErrorMsg("DomainWidget");
    if (rc < 0) return -1;
    return 0;
}
