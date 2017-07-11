//************************************************************************
//															*
//		     Copyright (C)  2016										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		ColorbarWidget.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016
//
//	Description:	Implements the ColorbarWidget class.  This provides
//		a frame that contains buttons and text for controlling a color bar
//		Intended to be used with colorbarframe.ui for event routers that
//		embed a colorbarframe
//
#include "vapor/RenderParams.h"
#include "ColorbarWidget.h"
#include "vapor/ColorbarPbase.h"
#include <QFrame>
#include <qwidget.h>
#include <vector>
#include <QString>
#include <qcolordialog.h>
#include "vapor/DataStatus.h"

using namespace VAPoR;

ColorbarWidget::ColorbarWidget(QWidget *parent) : QFrame(parent), Ui_ColorbarWidgetGUI()
{
    setupUi(this);
    _eventRouter = NULL;
    _textChangedFlag = false;

    connect(colorbarXposEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));
    connect(colorbarYposEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));
    connect(colorbarXsizeEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));
    connect(colorbarYsizeEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));
    connect(colorbarFontsizeEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));
    connect(colorbarDigitsEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));
    connect(colorbarNumTicsEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));
    connect(colorbarTitleEdit, SIGNAL(returnPressed()), this, SLOT(colorbarReturnPressed()));

    connect(colorbarEnableCheckbox, SIGNAL(released()), this, SLOT(enabledChange()));
    connect(colorbarBackgroundSelectButton, SIGNAL(clicked()), this, SLOT(setBackgroundColor()));
    connect(colorbarApplyAllButton, SIGNAL(clicked()), this, SLOT(applyToAll()));
}

ColorbarWidget::~ColorbarWidget() {}

void ColorbarWidget::colorbarReturnPressed() {}

void ColorbarWidget::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams)
{
    assert(paramsMgr);
    assert(dataMgr);
    assert(rParams);

    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    _rParams = rParams;
}

void ColorbarWidget::colorbarTextChanged(const QString &) { _textChangedFlag = true; }

void ColorbarWidget::enabledChange()
{
    Qt::CheckState state = colorbarEnableCheckbox->checkState();
    ColorbarPbase *cbPbase = GetActiveParams();
    if (!cbPbase) return;
    if (state == Qt::Unchecked) {
        cbPbase->SetEnabled(0);
    } else {
        cbPbase->SetEnabled(1);
    }
}

void ColorbarWidget::setBackgroundColor()
{
    QPalette pal(ColorbarBackgroundColorEdit->palette());
    QColor   newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
    if (!newColor.isValid()) return;
    pal.setColor(QPalette::Base, newColor);
    ColorbarBackgroundColorEdit->setPalette(pal);
    qreal rgb[3];
    newColor.getRgbF(rgb, rgb + 1, rgb + 2);
    vector<double> rgbd;
    for (int i = 0; i < 3; i++) rgbd.push_back((double)rgb[i]);
    GetActiveParams()->SetBackgroundColor(rgbd);
}
void ColorbarWidget::applyToAll()
{
#ifdef DEAD
    _paramsMgr->CopyColorBarSettings((RenderParams *)_params);
#endif
}
