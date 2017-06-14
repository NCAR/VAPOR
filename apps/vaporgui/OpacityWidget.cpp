//-- OpacityWidget.cpp -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
//
//----------------------------------------------------------------------------

#include <vapor/glutil.h>    // Must be included first!!!
#include "OpacityWidget.h"
#include "vapor/OpacityMap.h"
#include "MappingFrame.h"

#include <iostream>

using namespace std;
using namespace VAPoR;

#ifndef MAX
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
OpacityWidget::OpacityWidget(MappingFrame *parent, OpacityMap *omap) : GLWidget(), _parent(parent), _opacityMap(omap), _handleRadius(0.045), _handle(NULL), _minValue(0.0), _maxValue(1.0)
{
    _minY = -0.05;
    _maxY = 1.05;
    _handle = gluNewQuadric();
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
OpacityWidget::~OpacityWidget() {}

//----------------------------------------------------------------------------
// Widget's width in the world coordinate frame
//----------------------------------------------------------------------------
float OpacityWidget::width() { return right() - left(); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void OpacityWidget::move(float dx, float dy, float)
{
    float scaling = (_parent->maxDataValue() - _parent->minDataValue());

    if ((_selected == BOTTOM_LEFT || _selected == TOP_LEFT || _selected == BOTTOM_RIGHT || _selected == TOP_RIGHT || _selected == BORDER) && left() + dx >= right() && right() + dx <= left()) {
        _opacityMap->setMinValue(_opacityMap->minValue() + dx * scaling);
        _opacityMap->setMaxValue(_opacityMap->maxValue() + dx * scaling);
    }

    else if (_selected >= CONTROL_POINT && _opacityMap->GetType() == OpacityMap::CONTROL_POINT) {
        set<int>::iterator iter;

        for (iter = _selectedCPs.begin(); iter != _selectedCPs.end(); iter++) { _opacityMap->moveControlPoint(*iter - CONTROL_POINT, dx * scaling, dy); }
    }

    emit mapChanged();
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void OpacityWidget::drag(float dx, float dy, float)
{
    float scaling = (_parent->maxDataValue() - _parent->minDataValue());

    if ((_selected == BOTTOM_LEFT || _selected == TOP_LEFT) && left() + dx <= right() && left() + dx >= 0.0) {
        _opacityMap->setMinValue(_opacityMap->minValue() + dx * scaling);
    }

    else if ((_selected == BOTTOM_RIGHT || _selected == TOP_RIGHT) && right() + dx >= left() && right() + dx <= 1.0) {
        _opacityMap->setMaxValue(_opacityMap->maxValue() + dx * scaling);
    }

    else if (_selected >= CONTROL_POINT) {
        set<int>::iterator iter;

        for (iter = _selectedCPs.begin(); iter != _selectedCPs.end(); iter++) { _opacityMap->moveControlPoint(*iter - CONTROL_POINT, dx * scaling, dy); }

    }

    else if (_selected == TOP_SLIDER && topSlider() + dx >= left() && topSlider() + dx <= right()) {
        topSlider(topSlider() + dx);
    }

    else if (_selected == LEFT_SLIDER && leftSlider() + dy >= 0.0 && leftSlider() + dy <= 1.0) {
        leftSlider(leftSlider() + dy);
    }

    else if (_selected == BORDER && left() + dx <= right() && right() + dx >= left()) {
        _opacityMap->setMinValue(_opacityMap->minValue() + dx * scaling);
        _opacityMap->setMaxValue(_opacityMap->maxValue() + dx * scaling);
    }

    emit mapChanged();
}

//----------------------------------------------------------------------------
// Select the widget given the opengl selection name and qt button state.
//----------------------------------------------------------------------------
void OpacityWidget::select(int handle, Qt::KeyboardModifiers state)
{
    if (!(state & (Qt::ShiftModifier | Qt::ControlModifier))) { _selectedCPs.clear(); }

    if (handle >= CONTROL_POINT) {
        //
        // Control button
        //
        if (state & Qt::ControlModifier) {
            if (_selectedCPs.find(handle) != _selectedCPs.end()) {
                //
                // Clicked on an already selected control point, deselect it.
                //
                _selectedCPs.erase(handle);

                if (_selectedCPs.size()) {
                    _selected = *_selectedCPs.begin();
                } else {
                    _selected = NONE;
                }
            } else {
                _selected = handle;
                _selectedCPs.insert(_selected);
            }

            return;
        }

        //
        // Shift button
        //
        if (state & Qt::ShiftModifier) {
            _selected = handle;

            if (_selectedCPs.find(handle) != _selectedCPs.end()) {
                return;    // Already selected
            }

            set<int>::iterator iter = _selectedCPs.insert(handle).first;

            set<int>::iterator loweri = iter;
            set<int>::iterator upperi = iter;

            int lower = (loweri != _selectedCPs.begin()) ? *(--loweri) : handle;
            int upper = (++upperi != _selectedCPs.end()) ? *upperi : handle;

            for (int i = lower; i <= upper; i++) { _selectedCPs.insert(i); }

            return;
        }

        _selectedCPs.insert(handle);
    }

    _selected = handle;
}

//----------------------------------------------------------------------------
// Deselecte the widget
//----------------------------------------------------------------------------
void OpacityWidget::deselect()
{
    _selected = NONE;
    _selectedCPs.clear();
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
bool OpacityWidget::enabled() const { return _opacityMap->IsEnabled(); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void OpacityWidget::enable(bool flag)
{
    _enabled = flag;
    _opacityMap->SetEnabled(flag);
}

//----------------------------------------------------------------------------
// Deselect selected control points
//----------------------------------------------------------------------------
void OpacityWidget::deleteSelectedControlPoint()
{
    if (_selected >= CONTROL_POINT) {
        emit startChange("Delete opacity control point");

        _opacityMap->deleteControlPoint(_selected - CONTROL_POINT);

        _selected = CURVE;

        emit endChange();
    }
}

//----------------------------------------------------------------------------
// Return the positions of the selected control points
//----------------------------------------------------------------------------
list<float> OpacityWidget::selectedPoints()
{
    set<int>::iterator iter;
    list<float>        points;

    for (iter = _selectedCPs.begin(); iter != _selectedCPs.end(); iter++) { points.push_back(_opacityMap->controlPointValue(*iter - CONTROL_POINT)); }

    return points;
}

//----------------------------------------------------------------------------
// Render the widget
//----------------------------------------------------------------------------
int OpacityWidget::paintGL()
{
    int rc = printOpenGLErrorMsg("OpacityWidgetPaintGL");
    if (rc < 0) return -1;

    glPushName(_id);

    glPushMatrix();
    {
        if (_selected != NONE) {
            glColor4f(1.0, 1.0, 0.5, 1.0);

            //
            // Draw interaction handles
            //
            float length = 0.02;

            glPushMatrix();
            {
                glPushName(BOTTOM_LEFT);
                glTranslatef(left() - length, bottom(), 0.0);
                glRotatef(90, 0, 1, 0);
                gluCylinder(_handle, 0.0, _handleRadius, length, 10, 2);
                glPopName();
            }
            glPopMatrix();

            glPushMatrix();
            {
                glPushName(BOTTOM_RIGHT);
                glTranslatef(right(), bottom(), 0.0);
                glRotatef(90, 0, 1, 0);
                gluCylinder(_handle, _handleRadius, 0.0, length, 10, 2);
                glPopName();
            }
            glPopMatrix();

            glPushMatrix();
            {
                glPushName(TOP_RIGHT);
                glTranslatef(right(), top(), 0.0);
                glRotatef(90, 0, 1, 0);
                gluCylinder(_handle, _handleRadius, 0.0, length, 10, 2);
                glPopName();
            }
            glPopMatrix();

            glPushMatrix();
            {
                glPushName(TOP_LEFT);
                glTranslatef(left() - length, top(), 0.0);
                glRotatef(90, 0, 1, 0);
                gluCylinder(_handle, 0.0, _handleRadius, length, 10, 2);
                glPopName();
            }
            glPopMatrix();

            glColor4f(0.5, 1.0, 1.0, 1.0);

            if (hasTopSlider()) {
                glPushMatrix();
                {
                    glPushName(TOP_SLIDER);
                    glTranslatef(topSlider() - length / 2.0, top(), 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_handle, _handleRadius, _handleRadius, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();
            }

            if (hasBottomSlider()) {
                glPushMatrix();
                {
                    glPushName(BOTTOM_SLIDER);
                    glTranslatef(bottomSlider() - length / 2.0, bottom(), 0.0);
                    glRotatef(90, 0, 1, 0);
                    gluCylinder(_handle, _handleRadius, _handleRadius, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();
            }

            glColor4f(1.0, 0.0, 0.5, 1.0);

            if (hasRightSlider()) {
                glPushMatrix();
                {
                    glPushName(RIGHT_SLIDER);
                    glTranslatef(right(), rightSlider(), 0.0);
                    glRotatef(90, 1, 0, 0);
                    gluCylinder(_handle, _handleRadius, _handleRadius, length, 10, 2);
                    glPopName();
                }
                glPopMatrix();
            }

            if (hasLeftSlider()) {
                glPushMatrix();
                {
                    glPushName(LEFT_SLIDER);
                    glTranslatef(left(), leftSlider(), 0.0);
                    glRotatef(90, 1, 0, 0);
                    gluCylinder(_handle, _handleRadius / 3.0, _handleRadius / 3.0, 3 * length, 10, 2);
                    glPopName();
                }
                glPopMatrix();
            }

            glColor4f(1.0, 1.0, 0.5, 1.0);

            glPushName(BORDER);
            drawLines();
            glPopName();
        }

        glDisable(GL_LIGHTING);

        glPushName(CURVE);
        drawCurve();
        glPopName();

        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
    glPopName();

    rc = printOpenGLErrorMsg("OpacityWidget");
    if (rc < 0) return -1;
    return 0;
}

//----------------------------------------------------------------------------
// Draw the widget's bounding box lines
//----------------------------------------------------------------------------
void OpacityWidget::drawLines()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glLineWidth(2.0f);

    glBegin(GL_LINES);
    {
        glVertex3f(left(), bottom(), 0.0);
        glVertex3f(left(), top(), 0.0);

        glVertex3f(right(), bottom(), 0.0);
        glVertex3f(right(), top(), 0.0);

        glVertex3f(left(), top(), 0.0);
        glVertex3f(right(), top(), 0.0);

        glVertex3f(left(), bottom(), 0.0);
        glVertex3f(right(), bottom(), 0.0);
    }
    glEnd();

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

//----------------------------------------------------------------------------
// Draw the widget's opacity curve
//----------------------------------------------------------------------------
void OpacityWidget::drawCurve()
{
    float z = (_selected == NONE) ? 0.0 : 0.1;

    const int segments = 256;

    float step = width() / (segments - 1);

    if (enabled()) {
        if (_selected != NONE) {
            glColor3f(1.0, 1.0, 1.0);
        } else {
            glColor3f(0.65, 0.65, 0.65);
        }
    } else {
        glColor3f(0.4, 0.0, 0.0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glBegin(GL_LINE_STRIP);
    {
        for (int i = 0; i < segments; i++) {
            float value = left() + step * i;
            float opacity = _opacityMap->opacityData(worldToData(value));

            glVertex3f(value, opacity, z);
        }
    }
    glEnd();

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    //
    // Draw control points
    //
    if (_opacityMap->GetType() == OpacityMap::CONTROL_POINT) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        float scale = 2 * (float)viewport[2] / (float)viewport[3];

        for (int i = 0; i < _opacityMap->numControlPoints(); i++) {
            glPushName(CONTROL_POINT + i);

            float x = dataToWorld(_opacityMap->controlPointValue(i));
            float y = _opacityMap->controlPointOpacity(i);

            if (_selectedCPs.find(CONTROL_POINT + i) != _selectedCPs.end()) {
                glColor3f(1.0, 1.0, 0.0);
            } else {
                glColor3f(0.75, 0.75, 0.75);
            }

            glBegin(GL_TRIANGLE_FAN);
            {
                glVertex3f(x - 0.0075, y - scale * 0.0075, z);
                glVertex3f(x - 0.0075, y + scale * 0.0075, z);
                glVertex3f(x + 0.0075, y + scale * 0.0075, z);
                glVertex3f(x + 0.0075, y - scale * 0.0075, z);
            }
            glEnd();

            glPopName();
        }
    }
}

//----------------------------------------------------------------------------
// Set the widget's geometry
//----------------------------------------------------------------------------
void OpacityWidget::setGeometry(float x0, float x1, float y0, float y1)
{
    GLWidget::setGeometry(x0, x1, y0, y1);

    _handleRadius = (_maxY - 1.0) * 0.9;
}

//----------------------------------------------------------------------------
// Rightmost extent of widget in world coordinates
//----------------------------------------------------------------------------
float OpacityWidget::right() { return dataToWorld(_opacityMap->maxValue()); }

//----------------------------------------------------------------------------
// Leftmost extent of widget in world coordinates
//----------------------------------------------------------------------------
float OpacityWidget::left() { return dataToWorld(_opacityMap->minValue()); }

//----------------------------------------------------------------------------
// Returns true if this type of widget has a top slider control
//----------------------------------------------------------------------------
bool OpacityWidget::hasTopSlider() { return (_opacityMap->GetType() == OpacityMap::GAUSSIAN || _opacityMap->GetType() == OpacityMap::INVERTED_GAUSSIAN || _opacityMap->GetType() == OpacityMap::SINE); }

//----------------------------------------------------------------------------
// Current value of the top slider in world coordiates
//----------------------------------------------------------------------------
float OpacityWidget::topSlider()
{
    switch (_opacityMap->GetType()) {
    case OpacityMap::GAUSSIAN:
    case OpacityMap::INVERTED_GAUSSIAN: return left() + (_opacityMap->GetMean() * (right() - left()));
    case OpacityMap::SINE: return left() + (_opacityMap->GetFreq() * (right() - left()));
    default: return 0.5;
    }
}

//----------------------------------------------------------------------------
// Set the value of the top slider in world coordinates
//----------------------------------------------------------------------------
void OpacityWidget::topSlider(float value)
{
    switch (_opacityMap->GetType()) {
    case OpacityMap::GAUSSIAN:
    case OpacityMap::INVERTED_GAUSSIAN: _opacityMap->SetMean((value - left()) / (right() - left()));
    case OpacityMap::SINE: _opacityMap->SetFreq((value - left()) / (right() - left()));
    default: break;
    }
}

//----------------------------------------------------------------------------
// Returns true if this type of widget has a top slider control
//----------------------------------------------------------------------------
bool OpacityWidget::hasBottomSlider() { return false; }

//----------------------------------------------------------------------------
// Current value of the slider in world coordiates
//----------------------------------------------------------------------------
float OpacityWidget::bottomSlider() { return 0.5; }

//----------------------------------------------------------------------------
// Set the value of the slider in world coordinates
//----------------------------------------------------------------------------
void OpacityWidget::bottomSlider(float) {}

//----------------------------------------------------------------------------
// Returns true if this type of widget has a left slider control
//----------------------------------------------------------------------------
bool OpacityWidget::hasLeftSlider()
{
    return (_opacityMap->GetType() == OpacityMap::GAUSSIAN || _opacityMap->GetType() == OpacityMap::INVERTED_GAUSSIAN || _opacityMap->GetType() == OpacityMap::SINE);
}

//----------------------------------------------------------------------------
// Current value of the slider in world coordiates
//----------------------------------------------------------------------------
float OpacityWidget::leftSlider()
{
    switch (_opacityMap->GetType()) {
    case OpacityMap::GAUSSIAN:
    case OpacityMap::INVERTED_GAUSSIAN: return sqrt(_opacityMap->GetSSQ());
    case OpacityMap::SINE: return _opacityMap->GetPhase();
    default: break;
    }

    return 0.5;
}

//----------------------------------------------------------------------------
// Set the value of the slider in world coordinates
//----------------------------------------------------------------------------
void OpacityWidget::leftSlider(float value)
{
    switch (_opacityMap->GetType()) {
    case OpacityMap::GAUSSIAN:
    case OpacityMap::INVERTED_GAUSSIAN: _opacityMap->SetSSQ(value * value);
    case OpacityMap::SINE: _opacityMap->SetPhase(value);
    default: break;
    }
}

//----------------------------------------------------------------------------
// Returns true if this type of widget has a right slider control
//----------------------------------------------------------------------------
bool OpacityWidget::hasRightSlider() { return false; }

//----------------------------------------------------------------------------
// Current value of the slider in world coordiates
//----------------------------------------------------------------------------
float OpacityWidget::rightSlider() { return 0.5; }

//----------------------------------------------------------------------------
// Set the value of the slider in world coordinates
//----------------------------------------------------------------------------
void OpacityWidget::rightSlider(float) {}

//----------------------------------------------------------------------------
// Transform the x position in the data (model) space into the opengl world
// space (0.0 ... 1.0)
//----------------------------------------------------------------------------
float OpacityWidget::dataToWorld(float x) { return (x - _parent->minDataValue()) / (_parent->maxDataValue() - _parent->minDataValue()); }

//----------------------------------------------------------------------------
// Transform the x position in the opengl world space into the data (model)
// space
//----------------------------------------------------------------------------
float OpacityWidget::worldToData(float x) { return _parent->minDataValue() + (x * (_parent->maxDataValue() - _parent->minDataValue())); }
