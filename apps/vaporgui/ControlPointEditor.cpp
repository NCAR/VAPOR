//-- ControlPointEditor -------------------------------------------------------
//
// Copyright (C) 2006 Kenny Gruchalla.  All rights reserved.
//
// Simple GUI dialog used to modify transfer function control points (i.e.,
// color and opacity control points).
//
//----------------------------------------------------------------------------



#include <vapor/ColorMap.h>
#include <vapor/MapperFunction.h>
#include "ControlPointEditor.h"
#include "MappingFrame.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcolordialog.h>
#include <qlabel.h>

using namespace VAPoR;

//----------------------------------------------------------------------------
// Constructor -- Opacity control point
//----------------------------------------------------------------------------
ControlPointEditor::ControlPointEditor(MappingFrame* parent, OpacityMap *map, 
                                       int cp) :
  QDialog(parent),
  Ui_ControlPointEditorBase(),
  _controlPoint(cp),
  _mapper(parent->mapperFunction()),
  _omap(map),
  _cmap(NULL)
{
  setupUi(this);
  initWidgets();
  initConnections();
}

//----------------------------------------------------------------------------
// Constructor -- Color control point
//----------------------------------------------------------------------------
ControlPointEditor::ControlPointEditor(MappingFrame* parent, ColorMap *map, 
                                       int cp) :
  QDialog(parent),
  Ui_ControlPointEditorBase(),
  _controlPoint(cp),
  _mapper(parent->mapperFunction()),
  _omap(NULL),
  _cmap(map)
{
  setupUi(this);
  initWidgets();
  initConnections();
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
ControlPointEditor::~ControlPointEditor()
{
}

//----------------------------------------------------------------------------
// Initialize the dialogs widgets
//----------------------------------------------------------------------------
void ControlPointEditor::initWidgets()
{
  _dataValueField->setText(QString("%1").arg(dataValue()));
  _quantValueField->setText(QString("%1").arg(toIndex(dataValue())));

  if (_omap)
  {
    _opacityField->setText(QString("%1")
                   .arg(_omap->controlPointOpacity(_controlPoint)));

    _colorLabel->hide();
    _colorButton->hide();
  }
  
  if (_cmap)
  {
    ColorMap::Color color = _cmap->controlPointColor(_controlPoint);
   

    _tempColor.setHsv((int)(359*color.hue()), 
                  (int)(255*color.sat()), 
                  (int)(255*color.val()));
	
	QPalette pal;
	pal.setColor(QPalette::Base,_tempColor);
	
	_colorEdit->setPalette(pal);

                              
    _opacityLabel->hide();
    _opacityField->hide();
  }

  _nullButton->hide();

  _okButton->setDefault(true);
  _cancelButton->setDefault(false);
  //_nullButton->setDefault(true);
}

//----------------------------------------------------------------------------
// Initialize the dialog's signal/slot connections
//----------------------------------------------------------------------------
void ControlPointEditor::initConnections()
{
  connect(_dataValueField, SIGNAL(returnPressed()), 
          this, SLOT(dataValueChanged()));

  connect(_quantValueField, SIGNAL(returnPressed()), 
          this, SLOT(indexValueChanged()));

  connect(_colorButton, SIGNAL(clicked()),
          this, SLOT(pickColor()));

  connect(_okButton, SIGNAL(clicked()),
          this, SLOT(okHandler()));

  connect(_cancelButton, SIGNAL(clicked()),
          this, SLOT(cancelHandler()));

          
}

//----------------------------------------------------------------------------
// Handle changes to the data value (i.e., update the quantized value).
//----------------------------------------------------------------------------
void ControlPointEditor::dataValueChanged()
{
  bool ok;

  float value = _dataValueField->text().toFloat(&ok);

  if (!ok)
  {
    _dataValueField->setText(QString("%1").arg(dataValue()));
    _quantValueField->setText(QString("%1").arg(toIndex(dataValue())));
  }    
  else
  {
    _quantValueField->setText(QString("%1").arg(toIndex(value)));
  }    
}

//----------------------------------------------------------------------------
// Handle changes to the quanitized index value (i.e., update the data value).
//----------------------------------------------------------------------------
void ControlPointEditor::indexValueChanged()
{
  bool ok;

  int value = _quantValueField->text().toInt(&ok);

  if (!ok)
  {
    _dataValueField->setText(QString("%1").arg(dataValue()));
    _quantValueField->setText(QString("%1").arg(toIndex(dataValue())));
  }
  else
  {
    _dataValueField->setText(QString("%1").arg(toData(value)));
  }    
}

//----------------------------------------------------------------------------
// Launch the color picker and handle the selection of a new color
//----------------------------------------------------------------------------
void ControlPointEditor::pickColor()
{
	QPalette pal(_colorEdit->palette());
	QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
	if (!newColor.isValid()) return; 
	pal.setColor(QPalette::Base, newColor);
	_colorEdit->setPalette(pal);
	_tempColor = newColor;

}

//----------------------------------------------------------------------------
// Ok button handler
//----------------------------------------------------------------------------
void ControlPointEditor::okHandler()
{
  if (_omap)
  {
    float value   = _dataValueField->text().toFloat();
    float opacity = _opacityField->text().toFloat();
    
    _omap->controlPointValue(_controlPoint, value);
    _omap->controlPointOpacity(_controlPoint, opacity);
  }
  else
  {
    float value = _dataValueField->text().toFloat();

    int h,s,v;
	
	_tempColor.getHsv(&h,&s,&v);

    ColorMap::Color color(h/359.0, s/255.0, v/255.0);

    _cmap->controlPointValue(_controlPoint, value);
    _cmap->controlPointColor(_controlPoint, color);
  }
  
  close();
}


//----------------------------------------------------------------------------
// Cancel button handler
//----------------------------------------------------------------------------
void ControlPointEditor::cancelHandler()
{
  close();
}

//----------------------------------------------------------------------------
// Returns the control point's data value.
//----------------------------------------------------------------------------
float ControlPointEditor::dataValue()
{
  return _omap ? 
    _omap->controlPointValue(_controlPoint)
  : _cmap->controlPointValue(_controlPoint);
}

//----------------------------------------------------------------------------
// Returns the quantized index value given a data value
//----------------------------------------------------------------------------
int ControlPointEditor::toIndex(float data)
{
  if (_mapper)
  {
    return  _mapper->mapFloatToIndex(data);
  }

  return 0;
}

//----------------------------------------------------------------------------
// Returns the data value given a quantized index value
//----------------------------------------------------------------------------
float ControlPointEditor::toData(int index)
{
  if (_mapper)
  {
    return 
      _mapper->mapIndexToFloat(index);    
  }

  return 0;
}

