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

    _spXCombo = new Combo(xEdit, xSlider);
    _spYCombo = new Combo(yEdit, ySlider);
    _spZCombo = new Combo(zEdit, zSlider);

    _minXCombo = new Combo(minXEdit, minXSlider);
    _maxXCombo = new Combo(maxXEdit, maxXSlider);
    _xRangeCombo = new RangeCombo(_minXCombo, _maxXCombo);

    _minYCombo = new Combo(minYEdit, minYSlider);
    _maxYCombo = new Combo(maxYEdit, maxYSlider);
    _yRangeCombo = new RangeCombo(_minYCombo, _maxYCombo);

    _minZCombo = new Combo(minZEdit, minZSlider);
    _maxZCombo = new Combo(maxZEdit, maxZSlider);
    _zRangeCombo = new RangeCombo(_minZCombo, _maxZCombo);

    connectWidgets();
}

void GeometryWidget::hideSinglePointTabHeader() { singlePointTab->setStyleSheet("QTabBar::tab { height: 0px }"); }

void GeometryWidget::adjustLayoutToSinglePoint()
{
    return;
    QSizePolicy::Policy minimum = QSizePolicy::Minimum;
    QSizePolicy::Policy ignored = QSizePolicy::Ignored;

    singlePointPage->setSizePolicy(minimum, minimum);
    singlePointTab->setSizePolicy(minimum, minimum);

    minMaxPage->setSizePolicy(ignored, ignored);
    minMaxTab->setSizePolicy(ignored, ignored);

    centerSizePage->setSizePolicy(ignored, ignored);
    centerSizeTab->setSizePolicy(ignored, ignored);

    stackedSliderWidget->adjustSize();
    adjustSize();
}

void GeometryWidget::adjustLayoutToMinMax()
{
    return;
    QSizePolicy::Policy minimum = QSizePolicy::Minimum;
    QSizePolicy::Policy ignored = QSizePolicy::Ignored;

    singlePointPage->setSizePolicy(ignored, ignored);
    singlePointTab->setSizePolicy(ignored, ignored);

    minMaxPage->setSizePolicy(minimum, minimum);
    minMaxTab->setSizePolicy(minimum, minimum);

    centerSizePage->setSizePolicy(ignored, ignored);
    centerSizeTab->setSizePolicy(ignored, ignored);

    stackedSliderWidget->adjustSize();
    adjustSize();
}

void GeometryWidget::adjustLayoutTo2D()
{
    zMinMaxFrame->hide();
    zMinMaxFrame->resize(0, 0);
    minMaxContainerWidget->adjustSize();
    minMaxTab->adjustSize();

    stackedSliderWidget->adjustSize();
    adjustSize();
}

void GeometryWidget::Reinit(DisplayFlags displayFlags, VariableFlags varFlags)
{
    _displayFlags = displayFlags;
    _varFlags = varFlags;

    if (_displayFlags & MINMAX) {
        adjustLayoutToMinMax();
        stackedSliderWidget->setCurrentIndex(0);
    } else if (_displayFlags & SINGLEPOINT) {
        adjustLayoutToSinglePoint();
        stackedSliderWidget->setCurrentIndex(1);
    }

    stackedSliderWidget->adjustSize();
    minMaxTab->adjustSize();
}

GeometryWidget::~GeometryWidget()
{
    if (_spXCombo) {
        delete _spXCombo;
        _spXCombo = NULL;
    }
    if (_spYCombo) {
        delete _spYCombo;
        _spYCombo = NULL;
    }
    if (_spZCombo) {
        delete _spZCombo;
        _spZCombo = NULL;
    }
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
    xMinMaxLabel->setText(xTitle);

    if (minExt.size() < 2) return;
    QString yTitle = QString("Y Min: ") + QString::number(minExt[1], 'g', 3) + QString("	Max: ") + QString::number(maxExt[1], 'g', 3);
    yMinMaxLabel->setText(yTitle);

    if (minExt.size() < 3) {
        adjustLayoutTo2D();
        zMinMaxLabel->setText(QString("Z Coordinates aren't available for 2D variables!"));
    } else {
        zMinMaxFrame->show();
        QString zTitle = QString("Z Min: ") + QString::number(minExt[2], 'g', 3) + QString("	Max: ") + QString::number(maxExt[2], 'g', 3);
        zMinMaxLabel->setText(zTitle);
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

    // Update single-point combos
    //
    _spXCombo->Update(minFullExt[0], maxFullExt[0], minExt[0]);
    _spYCombo->Update(minFullExt[1], maxFullExt[1], minExt[1]);
    if (!box->IsPlanar()) { _spZCombo->Update(minFullExt[2], maxFullExt[2], minExt[2]); }
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
    connect(_spXCombo, SIGNAL(valueChanged(double)), this, SLOT(setPoint(double)));
    connect(_spYCombo, SIGNAL(valueChanged(double)), this, SLOT(setPoint(double)));
    connect(_spZCombo, SIGNAL(valueChanged(double)), this, SLOT(setPoint(double)));
    connect(_xRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_yRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_zRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
}

void GeometryWidget::setPoint(double point)
{
    size_t dimension;
    if (QObject::sender() == _spXCombo)
        dimension = 0;
    else if (QObject::sender() == _spYCombo)
        dimension = 1;
    else
        dimension = 2;

    setRange(point, point, dimension);
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
