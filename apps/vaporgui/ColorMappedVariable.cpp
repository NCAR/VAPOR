//************************************************************************
//							  *
//	   Copyright (C)  2017					*
//	 University Corporation for Atmospheric Research		  *
//	   All Rights Reserved					*
//							  *
//************************************************************************/
//
//  File:	   ColorMappedVariable.cpp
//
//  Author:	 Scott Pearse
//	  National Center for Atmospheric Research
//	  PO 3000, Boulder, Colorado
//
//  Date:	   March 2017
//
//  Description:	Implements the ColorMappedVariable class.  This provides
//	  a widget that is inserted in the "Appearance" tab of various Renderer GUIs
//
#include <sstream>
#include <qwidget.h>
#include <qcolordialog.h>
#include "TwoDSubtabs.h"
#include "vapor/RenderParams.h"
#include "ColorMappedVariable.h"

using namespace VAPoR;

ColorMappedVariable::ColorMappedVariable(QWidget *parent) : QWidget(parent), Ui_ColorMappedVariableGUI()
{
    setupUi(this);

    _myRGB[0] = _myRGB[1] = _myRGB[2] = .1;

    connectWidgets();
}

void ColorMappedVariable::Reinit(Flags flags) { _flags = flags; }

ColorMappedVariable::~ColorMappedVariable() {}

void ColorMappedVariable::Update(ParamsMgr *paramsMgr, DataMgr *dataMgr, RenderParams *rParams)
{
    cout << "Updating ColorMappedVariable" << endl;

    assert(rParams);

    _rParams = rParams;
}

void ColorMappedVariable::connectWidgets()
{
    connect(colormapVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCMVar(int)));
    connect(colorSelectButton, SIGNAL(pressed()), this, SLOT(setSingleColor()));
}

void ColorMappedVariable::setCMVar()
{
    string var = colormapVarCombo->currentText().toStdString();
    if (var == "None") {
        var = "";
        _rParams->SetUseSingleColor(true);
        _rParams->SetConstantColor(_myRGB);
    } else {
        _rParams->SetColorMapVariableName(var);
        _rParams->SetUseSingleColor(false);
    }
}

void ColorMappedVariable::setSingleColor()
{
    QPalette palette(colorDisplay->palette());
    QColor   color = QColorDialog::getColor(palette.color(QPalette::Base), this);
    if (!color.isValid()) return;

    palette.setColor(QPalette::Base, color);
    colorDisplay->setPalette(palette);

    qreal rgb[3];
    color.getRgbF(&rgb[0], &rgb[1], &rgb[2]);
    _myRGB[0] = rgb[0];
    _myRGB[1] = rgb[1];
    _myRGB[2] = rgb[2];

    _rParams->SetConstantColor(_myRGB);
    _rParams->SetUseSingleColor(true);
}
