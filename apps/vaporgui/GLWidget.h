//-- GLWidget.h ---------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// Abstract base class for OpenGL-based widgets that provides a common
// interface for selecting, moving, and drawing.
//
//----------------------------------------------------------------------------

#ifndef GLWidget_H
#define GLWidget_H

#include <qobject.h>

#include <qnamespace.h>
#include <list>

class QWidget;

class GLWidget : public QObject {
    Q_OBJECT

protected:
    enum { NONE = -1 };

public:
    GLWidget(QWidget *parent = 0);
    virtual ~GLWidget();

    virtual int paintGL() = 0;

    virtual void move(float dx, float dy = 0.0, float dz = 0.0) = 0;
    virtual void drag(float dx, float dy = 0.0, float dz = 0.0) = 0;

    virtual bool selected() { return _selected != NONE; }
    virtual void deselect() { _selected = NONE; }
    virtual void select(int handle, Qt::KeyboardModifiers) { _selected = handle; }

    virtual bool enabled() const { return _enabled; }
    virtual void enable(bool flag) { _enabled = flag; }

    virtual void setGeometry(float x0, float x1, float y0, float y1);

    int id() const { return _id; }

signals:

    void startChange(QString);
    void endChange();

protected:
    static unsigned int createId();

    unsigned int _id;
    int          _selected;
    bool         _enabled;

    float _minX;
    float _maxX;
    float _minY;
    float _maxY;
};

#endif    // GLWidget_H
