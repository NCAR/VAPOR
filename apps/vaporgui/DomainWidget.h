//-- DomainWidget.h ---------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// OpenGL-based widget used to scale and move the a transfer function domain.
// Also supports derived class: IsoSlider, with a limited subset of the functionality
//
//----------------------------------------------------------------------------

#ifndef DomainWidget_H
    #define DomainWidget_H

    #include "GLWidget.h"
    #include <vapor/MyBase.h>

    #include <qobject.h>
    #include <iostream>
using namespace std;

class GLUquadric;

class DomainWidget : public GLWidget {
    Q_OBJECT
protected:
    enum { LEFT, RIGHT, BAR, VERTLINE };

public:
    DomainWidget(QWidget *parent, Qt::Orientation = Qt::Horizontal, float min = 0.0, float max = 1.0);
    virtual ~DomainWidget();

    virtual int paintGL();

    void         move(float dx, float dy = 0.0, float dz = 0.0);    // world-coordinates
    virtual void drag(float dx, float dy = 0.0, float dz = 0.0);    // world-coordinates

    float minValue() const { return _minValue; }    // world-coordinates
    float maxValue() const { return _maxValue; }    // world-coordinates

    virtual void setDomain(float minv, float maxv)
    {
        // cout << "setDomain " << minv << " " << maxv << endl;
        _minValue = minv;
        _maxValue = maxv;
    }

    virtual void setGeometry(float x0, float x1, float y0, float y1);

signals:

    //
    // Signals that the widget is being moved/edited.  Currently not connected.
    //
    void changingDomain(float min, float max);

protected:
    float width();
    float mid();

    float _minValue;
    float _maxValue;

    //
    // Frame data
    //
    float       _handleRadius;
    GLUquadric *_quadHandle;

    Qt::Orientation _orientation;
};

class IsoSlider : public DomainWidget {
public:
    IsoSlider(QWidget *parent, float min = 0.0, float max = 1.0);

    virtual void drag(float dx, float dy = 0.0, float dz = 0.0);
    virtual int  paintGL();

    void setIsoValue(float val) { setDomain(val - 0.01, val + 0.01); }

protected:
    float _lineWidth;
};

#endif    // DomainWidget_H

class ContourRangeSlider : public DomainWidget {
public:
    ContourRangeSlider(QWidget *parent, float min = 0.0, float max = 1.0);
    virtual int paintGL();
};
