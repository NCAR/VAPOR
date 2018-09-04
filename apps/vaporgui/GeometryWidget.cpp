//************************************************************************
//							  *
//	   Copyright (C)  2017					*
//	 University Corporation for Atmospheric Research		  *
//	   All Rights Reserved					*
//							  *
//************************************************************************/
//
//  File:	   GeometryWidget.cpp
//
//  Author:	 Scott Pearse
//	  National Center for Atmospheric Research
//	  PO 3000, Boulder, Colorado
//
//  Date:	   March 2017
//
//  Description:	Implements the GeometryWidget class.  This provides
//  a widget that is inserted in the "Appearance" tab of various Renderer GUIs
//
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include "vapor/ParamsMgr.h"
#include "vapor/RenderParams.h"
#include "vapor/DataMgrUtils.h"
#include "GeometryWidget.h"

#define XY 0
#define XZ 1
#define YZ 2

using namespace VAPoR;

template<typename Out> void split(const std::string &s, char delim, Out result)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) { *(result++) = item; }
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

GeometryWidget::GeometryWidget(QWidget *parent) : QWidget(parent), Ui_GeometryWidgetGUI()
{
    setupUi(this);

    _paramsMgr = NULL;
    _dataMgr = NULL;
    _rParams = NULL;

    _minXCombo = new Combo(_minXEdit, _minXSlider);
    _maxXCombo = new Combo(_maxXEdit, _maxXSlider);
    _xRangeCombo = new RangeCombo(_minXCombo, _maxXCombo);

    _minYCombo = new Combo(_minYEdit, _minYSlider);
    _maxYCombo = new Combo(_maxYEdit, _maxYSlider);
    _yRangeCombo = new RangeCombo(_minYCombo, _maxYCombo);

    _minZCombo = new Combo(_minZEdit, _minZSlider);
    _maxZCombo = new Combo(_maxZEdit, _maxZSlider);
    _zRangeCombo = new RangeCombo(_minZCombo, _maxZCombo);

    connectWidgets();
}

void GeometryWidget::adjustLayoutToPlanar(int plane)
{
    if (plane == XY)
        adjustLayoutToPlanarXY();
    else if (plane == XZ)
        adjustLayoutToPlanarXZ();
    else if (plane == YZ)
        adjustLayoutToPlanarYZ();
}

void GeometryWidget::adjustLayoutToPlanarXY()
{
    _xMinMaxFrame->show();
    _xPointFrame->hide();

    _yMinMaxFrame->show();
    _yPointFrame->hide();

    _zMinMaxFrame->hide();
    _zPointFrame->show();
}

void GeometryWidget::adjustLayoutToPlanarXZ()
{
    _xMinMaxFrame->show();
    _xPointFrame->hide();

    _yMinMaxFrame->show();
    _yPointFrame->hide();

    _zMinMaxFrame->hide();
    _zPointFrame->show();
}

void GeometryWidget::adjustLayoutToPlanarYZ()
{
    _xMinMaxFrame->hide();
    _xPointFrame->show();

    _yMinMaxFrame->show();
    _yPointFrame->hide();

    _zMinMaxFrame->show();
    _zPointFrame->hide();
}

void GeometryWidget::adjustLayoutTo2D()
{
    _zFrame->hide();
    _zFrame->resize(0, 0);
    _minMaxContainerWidget->adjustSize();
    _minMaxTab->adjustSize();

    _stackedSliderWidget->adjustSize();
    adjustSize();
}

void GeometryWidget::Reinit(DimFlags dimFlags, VariableFlags varFlags, GeometryFlags geometryFlags)
{
    _dimFlags = dimFlags;
    _geometryFlags = geometryFlags;
    _varFlags = varFlags;

    if (_dimFlags & TWOD) {
        adjustLayoutTo2D();
    } else if (_dimFlags & THREED) {
        _zFrame->show();
    }

    _stackedSliderWidget->adjustSize();
    _minMaxTab->adjustSize();
}

GeometryWidget::~GeometryWidget()
{
    if (_minXCombo) {
        delete _minXCombo;
        _minXCombo = NULL;
    }
    if (_maxXCombo) {
        delete _maxXCombo;
        _maxXCombo = NULL;
    }
    if (_xRangeCombo) {
        delete _xRangeCombo;
        _xRangeCombo = NULL;
    }

    if (_minYCombo) {
        delete _minYCombo;
        _minYCombo = NULL;
    }
    if (_maxYCombo) {
        delete _maxYCombo;
        _maxYCombo = NULL;
    }
    if (_yRangeCombo) {
        delete _yRangeCombo;
        _yRangeCombo = NULL;
    }

    if (_minZCombo) {
        delete _minZCombo;
        _minZCombo = NULL;
    }
    if (_maxZCombo) {
        delete _maxZCombo;
        _maxZCombo = NULL;
    }
    if (_zRangeCombo) {
        delete _zRangeCombo;
        _zRangeCombo = NULL;
    }
}

void GeometryWidget::updateRangeLabels(std::vector<double> minExt, std::vector<double> maxExt)
{
    assert(minExt.size() == maxExt.size());

    if (minExt.size() < 1) return;
    QString xTitle = QString("X Min: ") + QString::number(minExt[0], 'g', 3) + QString("	Max: ") + QString::number(maxExt[0], 'g', 3);
    _xMinMaxLabel->setText(xTitle);

    if (minExt.size() < 2) return;
    QString yTitle = QString("Y Min: ") + QString::number(minExt[1], 'g', 3) + QString("	Max: ") + QString::number(maxExt[1], 'g', 3);
    _yMinMaxLabel->setText(yTitle);

    if (minExt.size() < 3) {
        Reinit((DimFlags)TWOD, _varFlags, _geometryFlags);
        _zMinMaxLabel->setText(QString("Z Coordinates aren't available for 2D variables!"));
    } else {
        Reinit((DimFlags)THREED, _varFlags, _geometryFlags);

        QString zTitle = QString("Z Min: ") + QString::number(minExt[2], 'g', 3) + QString("	Max: ") + QString::number(maxExt[2], 'g', 3);
        _zMinMaxLabel->setText(zTitle);
    }
}

bool GeometryWidget::getAuxiliaryExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts)
{
    std::vector<std::string> auxVarNames = _rParams->GetAuxVariableNames();
    if (auxVarNames.empty())
        return false;
    else {
        vector<int> axes;
        int         ts = _rParams->GetCurrentTimestep();
        int         level = _rParams->GetRefinementLevel();
        int         rc = DataMgrUtils::GetExtents(_dataMgr, ts, auxVarNames, minFullExts, maxFullExts, axes, level);

        if (rc < 0) {
            MyBase::SetErrMsg("Error: DataMgr could "
                              "not return valid values from GetVariableExtents()");
        }
    }
    return true;
}

bool GeometryWidget::getVectorExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts)
{
    std::vector<string> varNames = _rParams->GetFieldVariableNames();
    if (varNames.empty()) return false;

    if ((varNames[0] == "") && (varNames[1] == "") && (varNames[2] == "")) return false;

    std::vector<int> axes;
    int              ts = _rParams->GetCurrentTimestep();
    int              level = _rParams->GetRefinementLevel();
    int              rc = DataMgrUtils::GetExtents(_dataMgr, ts, varNames, minFullExts, maxFullExts, axes, level);

    if (rc < 0) {
        MyBase::SetErrMsg("Error: DataMgr could "
                          "not return valid values from GetVariableExtents()");
    }
    return true;
}

bool GeometryWidget::getVariableExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts)
{
    size_t ts = _rParams->GetCurrentTimestep();
    int    level = _rParams->GetRefinementLevel();
    string varName = _rParams->GetVariableName();

    if (varName.empty()) return false;
    int rc = _dataMgr->GetVariableExtents(ts, varName, level, minFullExts, maxFullExts);
    if (rc < 0) {
        MyBase::SetErrMsg("Error: DataMgr could "
                          "not return valid values from GetVariableExtents()");
    }
    return true;
}

void GeometryWidget::updateBoxCombos(std::vector<double> &minFullExt, std::vector<double> &maxFullExt)
{
    assert(minFullExt.size() == maxFullExt.size());
    if (minFullExt.size() < 2) return;

    // Get current user selected extents
    //
    Box *               box = _rParams->GetBox();
    std::vector<double> minExt, maxExt;
    box->GetExtents(minExt, maxExt);

    // Verify that the user extents comply with the domain extents
    //
    size_t extSize = box->IsPlanar() ? 2 : 3;
    for (int i = 0; i < extSize; i++) {
        if (minExt[i] < minFullExt[i]) minExt[i] = minFullExt[i];
        if (maxExt[i] > maxFullExt[i]) maxExt[i] = maxFullExt[i];
    }

    // Update RangeCombos
    //
    _xRangeCombo->Update(minFullExt[0], maxFullExt[0], minExt[0], maxExt[0]);
    _yRangeCombo->Update(minFullExt[1], maxFullExt[1], minExt[1], maxExt[1]);
    if (!box->IsPlanar()) { _zRangeCombo->Update(minFullExt[2], maxFullExt[2], minExt[2], maxExt[2]); }
}

void GeometryWidget::Update(ParamsMgr *paramsMgr, DataMgr *dataMgr, RenderParams *rParams)
{
    assert(paramsMgr);
    assert(dataMgr);
    assert(rParams);

    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    _rParams = rParams;

    // Get current domain extents
    //
    std::vector<double> minFullExt, maxFullExt;

    if (_varFlags & AUXILIARY)    // for Statistics
    {
        if (!getAuxiliaryExtents(minFullExt, maxFullExt)) return;
    } else if (_varFlags & VECTOR) {    // for vector renderers, ie Barbs
        if (!getVectorExtents(minFullExt, maxFullExt)) return;
    } else {    // for single variable renderers (most cases)
        if (!getVariableExtents(minFullExt, maxFullExt)) return;
    }

    updateRangeLabels(minFullExt, maxFullExt);
    updateBoxCombos(minFullExt, maxFullExt);
    adjustSize();
}

void GeometryWidget::connectWidgets()
{
    connect(_xRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_yRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_zRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_planeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustLayoutToPlanar(int)));
}

void GeometryWidget::setRange(double min, double max, int dimension)
{
    if (dimension == -1) {
        if (QObject::sender() == _xRangeCombo)
            dimension = 0;
        else if (QObject::sender() == _yRangeCombo)
            dimension = 1;
        else
            dimension = 2;
    }

    std::vector<double> minExt, maxExt;
    Box *               box = _rParams->GetBox();
    box->GetExtents(minExt, maxExt);

    // Apply the extents that changed into minExt and maxExt
    //
    minExt[dimension] = min;
    maxExt[dimension] = max;
    box->SetExtents(minExt, maxExt);

    emit valueChanged();
}
