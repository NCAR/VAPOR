//-- ControlPointEditor -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// Simple GUI dialog used to modify transfer function control points (i.e.,
// color and opacity control points).
//
//----------------------------------------------------------------------------

#ifndef ControlPointEditor_h
#define ControlPointEditor_h

#include "ui_ControlPointEditorBase.h"
#include <qdialog.h>

namespace VAPoR {
class MapperFunction;
class OpacityMap;
class ColorMap;
} // namespace VAPoR

class MappingFrame;

class ControlPointEditor : public QDialog, public Ui_ControlPointEditorBase {
    Q_OBJECT

  public:
    ControlPointEditor(MappingFrame *parent, VAPoR::OpacityMap *map, int cp);
    ControlPointEditor(MappingFrame *parent, VAPoR::ColorMap *map, int cp);

    ~ControlPointEditor();

    void update() { initWidgets(); }

  protected:
    QColor _tempColor;
    void initWidgets();
    void initConnections();

    float dataValue();
    int toIndex(float);
    float toData(int);

  protected slots:

    void dataValueChanged();
    void indexValueChanged();
    void pickColor();
    void okHandler();
    void cancelHandler();

  private:
    int _controlPoint;

    VAPoR::MapperFunction *_mapper;
    VAPoR::OpacityMap *_omap;
    VAPoR::ColorMap *_cmap;
};

#endif
