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

#define X 0
#define Y 1
#define Z 2

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

    _paramsMgr = nullptr;
    _dataMgr = nullptr;
    _rParams = nullptr;

    _minXCombo = new Combo(_minXEdit, _minXSlider);
    _maxXCombo = new Combo(_maxXEdit, _maxXSlider);
    _xRangeCombo = new RangeCombo(_minXCombo, _maxXCombo);

    _minYCombo = new Combo(_minYEdit, _minYSlider);
    _maxYCombo = new Combo(_maxYEdit, _maxYSlider);
    _yRangeCombo = new RangeCombo(_minYCombo, _maxYCombo);

    _minZCombo = new Combo(_minZEdit, _minZSlider);
    _maxZCombo = new Combo(_maxZEdit, _maxZSlider);
    _zRangeCombo = new RangeCombo(_minZCombo, _maxZCombo);

    _xSinglePoint->SetLabel("Location");
    _ySinglePoint->SetLabel("Location");
    _zSinglePoint->SetLabel("Location");

    connect(_xSinglePoint, SIGNAL(valueChanged(double)), this, SLOT(setPoint(double)));
    connect(_ySinglePoint, SIGNAL(valueChanged(double)), this, SLOT(setPoint(double)));
    connect(_zSinglePoint, SIGNAL(valueChanged(double)), this, SLOT(setPoint(double)));

    _orientationTab->hide();

    connectWidgets();
}

void GeometryWidget::showOrientationOptions()
{
    _orientationTab->show();
    _xPointFrame->show();
    _yPointFrame->show();
    _zPointFrame->show();
}

void GeometryWidget::hideOrientationOptions()
{
    _orientationTab->hide();
    _xPointFrame->hide();
    _yPointFrame->hide();
    _zPointFrame->hide();
}

void GeometryWidget::adjustLayoutToPlanar(int  plane,
                                          bool reinit    // = true by default
)
{
    if (plane == XY)
        adjustLayoutToPlanarXY(reinit);
    else if (plane == XZ)
        adjustLayoutToPlanarXZ(reinit);
    else if (plane == YZ)
        adjustLayoutToPlanarYZ(reinit);
}

void GeometryWidget::adjustLayoutToPlanarXY(bool reinit)
{
    _xMinMaxFrame->show();
    _yMinMaxFrame->show();
    _zMinMaxFrame->hide();

    _xPointFrame->hide();
    _yPointFrame->hide();
    _zPointFrame->show();

    if (!_rParams) return;

    if (reinit) reinitBoxToPlanarAxis(Z, _zSinglePoint);
}

void GeometryWidget::adjustLayoutToPlanarXZ(bool reinit)
{
    _xMinMaxFrame->show();
    _yMinMaxFrame->hide();
    _zMinMaxFrame->show();

    _xPointFrame->hide();
    _yPointFrame->show();
    _zPointFrame->hide();

    if (!_rParams) return;

    if (reinit) reinitBoxToPlanarAxis(Y, _ySinglePoint);
}

void GeometryWidget::adjustLayoutToPlanarYZ(bool reinit)
{
    _xMinMaxFrame->hide();
    _yMinMaxFrame->show();
    _zMinMaxFrame->show();

    _xPointFrame->show();
    _yPointFrame->hide();
    _zPointFrame->hide();

    if (!_rParams) return;

    if (reinit) reinitBoxToPlanarAxis(X, _xSinglePoint);
}

void GeometryWidget::reinitBoxToPlanarAxis(int planarAxis, QSliderEdit *slider)
{
    std::vector<double> minExt(3, 0);
    std::vector<double> maxExt(3, 1);
    getFullExtents(minExt, maxExt);
    double average = (minExt[planarAxis] + maxExt[planarAxis]) / 2.f;
    slider->SetValue(average);

    minExt[planarAxis] = average;
    maxExt[planarAxis] = average;
    _box->SetExtents(minExt, maxExt);
}

void GeometryWidget::adjustLayoutTo2D()
{
    _zFrame->hide();
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

    if (_geometryFlags & PLANAR) {
        showOrientationOptions();
        adjustLayoutToPlanar(XY, false);
    } else
        hideOrientationOptions();

    _minMaxTab->adjustSize();
}

GeometryWidget::~GeometryWidget()
{
    if (_minXCombo) {
        delete _minXCombo;
        _minXCombo = nullptr;
    }
    if (_maxXCombo) {
        delete _maxXCombo;
        _maxXCombo = nullptr;
    }
    if (_xRangeCombo) {
        delete _xRangeCombo;
        _xRangeCombo = nullptr;
    }

    if (_minYCombo) {
        delete _minYCombo;
        _minYCombo = nullptr;
    }
    if (_maxYCombo) {
        delete _maxYCombo;
        _maxYCombo = nullptr;
    }
    if (_yRangeCombo) {
        delete _yRangeCombo;
        _yRangeCombo = nullptr;
    }

    if (_minZCombo) {
        delete _minZCombo;
        _minZCombo = nullptr;
    }
    if (_maxZCombo) {
        delete _maxZCombo;
        _maxZCombo = nullptr;
    }
    if (_zRangeCombo) {
        delete _zRangeCombo;
        _zRangeCombo = nullptr;
    }
}

void GeometryWidget::updateRangeLabels(std::vector<double> minExt, std::vector<double> maxExt)
{
    VAssert(minExt.size() == maxExt.size());

    if (minExt.size() < 1) return;
    QString xTitle = QString("X Min: ") + QString::number(minExt[0], 'g', 3) + QString("	Max: ") + QString::number(maxExt[0], 'g', 3);
    _xMinMaxLabel->setText(xTitle);

    if (minExt.size() < 2) return;
    QString yTitle = QString("Y Min: ") + QString::number(minExt[1], 'g', 3) + QString("	Max: ") + QString::number(maxExt[1], 'g', 3);
    _yMinMaxLabel->setText(yTitle);

    if (minExt.size() < 3) {
        _zFrame->hide();
    } else {
        if (_zFrame->isHidden() && _dimFlags & THREED) _zFrame->show();
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

    // If no variable name set find one.
    //
    if (varName.empty()) {
        vector<string> varNames;
        for (int dim = 3; dim > 1; dim--) {
            varNames = _dataMgr->GetDataVarNames(dim);
            if (!varNames.empty()) {
                varName = varNames[0];
                break;
            }
        }
        if (varName.empty()) return (false);
    }
    int rc = _dataMgr->GetVariableExtents(ts, varName, level, minFullExts, maxFullExts);
    if (rc < 0) {
        MyBase::SetErrMsg("Error: DataMgr could "
                          "not return valid values from GetVariableExtents()");
    }
    return true;
}

void GeometryWidget::updateBoxCombos(std::vector<double> &minFullExt, std::vector<double> &maxFullExt)
{
    VAssert(minFullExt.size() == maxFullExt.size());
    if (minFullExt.size() < 2) return;

    // Get current user selected extents
    //
    std::vector<double> minExt, maxExt;
    _box->GetExtents(minExt, maxExt);

    // Force the user extents to be within the domain extents
    //
    int extSize = minExt.size();    // extSize = box->IsPlanar() ? 2 : 3;
    for (int i = 0; i < extSize; i++) {
        if (minExt[i] < minFullExt[i]) minExt[i] = minFullExt[i];
        if (maxExt[i] > maxFullExt[i]) maxExt[i] = maxFullExt[i];
    }

    // Update RangeCombos
    //
    _xRangeCombo->Update(minFullExt[X], maxFullExt[X], minExt[X], maxExt[X]);
    _xSinglePoint->SetExtents(minFullExt[X], maxFullExt[X]);
    _xSinglePoint->SetValue((minExt[X] + maxExt[X]) / 2.f);

    _yRangeCombo->Update(minFullExt[Y], maxFullExt[Y], minExt[Y], maxExt[Y]);
    _ySinglePoint->SetExtents(minFullExt[Y], maxFullExt[Y]);
    _ySinglePoint->SetValue((minExt[Y] + maxExt[Y]) / 2.f);

    if (extSize > 2) {
        _zRangeCombo->Update(minFullExt[Z], maxFullExt[Z], minExt[Z], maxExt[Z]);
        _zSinglePoint->SetExtents(minFullExt[Z], maxFullExt[Z]);
        _zSinglePoint->SetValue((minExt[Z] + maxExt[Z]) / 2.f);
    }
}

void GeometryWidget::Update(ParamsMgr *paramsMgr, DataMgr *dataMgr, RenderParams *rParams, Box *box)
{
    VAssert(paramsMgr);
    VAssert(dataMgr);
    VAssert(rParams);

    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    _rParams = rParams;

    if (box != nullptr)
        _box = box;
    else
        _box = _rParams->GetBox();
    VAssert(_box);

    // Get current domain extents
    //
    std::vector<double> minFullExt, maxFullExt;
    getFullExtents(minFullExt, maxFullExt);

    updateRangeLabels(minFullExt, maxFullExt);
    updateBoxCombos(minFullExt, maxFullExt);

    if (_geometryFlags & PLANAR) {
        _planeComboBox->blockSignals(true);

        int rParamsOrientation = _box->GetOrientation();
        _planeComboBox->setCurrentIndex(rParamsOrientation);

        bool reinit = false;
        adjustLayoutToPlanar(rParamsOrientation, reinit);

        _planeComboBox->blockSignals(false);
    }
}

void GeometryWidget::getFullExtents(std::vector<double> &minFullExt, std::vector<double> &maxFullExt)
{
    if (_varFlags & AUXILIARY)    // for Statistics
    {
        if (!getAuxiliaryExtents(minFullExt, maxFullExt)) return;
    } else if (_varFlags & VECTOR) {    // for vector renderers, ie Barbs
        if (!getVectorExtents(minFullExt, maxFullExt)) return;
    } else {    // for single variable renderers (most cases)
        if (!getVariableExtents(minFullExt, maxFullExt)) return;
    }
}

void GeometryWidget::connectWidgets()
{
    connect(_xRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_yRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_zRangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_planeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustLayoutToPlanar(int)));
}

void GeometryWidget::setPoint(double point) { setRange(point, point); }

void GeometryWidget::setRange(double min, double max, int dimension)
{
    QObject *sender = QObject::sender();
    if (dimension == -1) {
        if (sender == _xRangeCombo || sender == _xSinglePoint)
            dimension = X;
        else if (sender == _yRangeCombo || sender == _ySinglePoint)
            dimension = Y;
        else
            dimension = Z;
    }

    std::vector<double> minExt, maxExt;

    _box->GetExtents(minExt, maxExt);
    minExt[dimension] = min;
    maxExt[dimension] = max;
    _box->SetExtents(minExt, maxExt);

    emit valueChanged();
}
