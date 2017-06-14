//************************************************************************
//															*
//		     Copyright (C)  2011										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		BoxSliderFrame.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2011
//
//	Description:	Implements the BoxSliderFrame class.  This provides
//		a frame that contains sliders and text box for controlling a 2D or 3D axis-aligned box.
//		Intended to be used with boxframe.ui for event routers that embed a box slider control
//
#include "BoxSliderFrame.h"
#include <QFrame>
#include <qwidget.h>
#include <vector>
#include <QString>
#include "vapor/DataStatus.h"
#include "MainForm.h"
#include "vapor/ControlExecutive.h"
using namespace VAPoR;
DataStatus * BoxSliderFrame::_dataStatus = 0;
ControlExec *BoxSliderFrame::_controlExec = 0;
BoxSliderFrame::BoxSliderFrame(QWidget *parent) : QFrame(parent), Ui_boxframe()
{
    setupUi(this);
    connect(xCenterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(boxTextChanged(const QString &)));
    connect(yCenterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(boxTextChanged(const QString &)));
    connect(zCenterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(boxTextChanged(const QString &)));
    connect(xSizeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(boxTextChanged(const QString &)));
    connect(ySizeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(boxTextChanged(const QString &)));
    connect(zSizeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(boxTextChanged(const QString &)));
    connect(xCenterEdit, SIGNAL(returnPressed()), this, SLOT(boxReturnPressed()));
    connect(yCenterEdit, SIGNAL(returnPressed()), this, SLOT(boxReturnPressed()));
    connect(zCenterEdit, SIGNAL(returnPressed()), this, SLOT(boxReturnPressed()));
    connect(xSizeEdit, SIGNAL(returnPressed()), this, SLOT(boxReturnPressed()));
    connect(ySizeEdit, SIGNAL(returnPressed()), this, SLOT(boxReturnPressed()));
    connect(zSizeEdit, SIGNAL(returnPressed()), this, SLOT(boxReturnPressed()));
    connect(xCenterSlider, SIGNAL(sliderReleased()), this, SLOT(xSliderCtrChange()));
    connect(yCenterSlider, SIGNAL(sliderReleased()), this, SLOT(ySliderCtrChange()));
    connect(zCenterSlider, SIGNAL(sliderReleased()), this, SLOT(zSliderCtrChange()));
    connect(xSizeSlider, SIGNAL(sliderReleased()), this, SLOT(xSliderSizeChange()));
    connect(ySizeSlider, SIGNAL(sliderReleased()), this, SLOT(ySliderSizeChange()));
    connect(zSizeSlider, SIGNAL(sliderReleased()), this, SLOT(zSliderSizeChange()));
    // nudge events:
    connect(xSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(nudgeXSize(int)));
    connect(xCenterSlider, SIGNAL(valueChanged(int)), this, SLOT(nudgeXCenter(int)));
    connect(ySizeSlider, SIGNAL(valueChanged(int)), this, SLOT(nudgeYSize(int)));
    connect(yCenterSlider, SIGNAL(valueChanged(int)), this, SLOT(nudgeYCenter(int)));
    connect(zSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(nudgeZSize(int)));
    connect(zCenterSlider, SIGNAL(valueChanged(int)), this, SLOT(nudgeZCenter(int)));
    for (int i = 0; i < 3; i++) {
        _lastCenterSlider[i] = 128;
        _lastSizeSlider[i] = 256;
    }

    _silenceSignals = false;
}
BoxSliderFrame::~BoxSliderFrame() {}

void BoxSliderFrame::getBoxExtents(double ext[6])
{
    for (int i = 0; i < 6; i++) ext[i] = _boxExtents[i];
}
void BoxSliderFrame::setFullDomain(const double ext[6])
{
    for (int i = 0; i < 6; i++) _domainExtents[i] = ext[i];
}

void BoxSliderFrame::setBoxExtents(const std::vector<double> &minexts, const std::vector<double> &maxexts)
{
    for (int i = 0; i < 3; i++) {
        _boxExtents[i] = minexts[i];
        _boxExtents[i + 3] = maxexts[i];
    }
    double mid[3], sz[3];
    for (int i = 0; i < 3; i++) {
        mid[i] = 0.5 * (_boxExtents[i] + _boxExtents[i + 3]);
        sz[i] = _boxExtents[i + 3] - _boxExtents[i];
    }
    _silenceSignals = true;
    updateGuiValues(mid, sz);
    _silenceSignals = false;
}
void BoxSliderFrame::confirmText()
{
    if (!_textChangedFlag) return;
    _textChangedFlag = false;
    _silenceSignals = true;
    // Force the text to be valid
    double centers[3], sizes[3];
    centers[0] = xCenterEdit->text().toDouble();
    centers[1] = yCenterEdit->text().toDouble();
    centers[2] = zCenterEdit->text().toDouble();
    sizes[0] = xSizeEdit->text().toDouble();
    sizes[1] = ySizeEdit->text().toDouble();
    sizes[2] = zSizeEdit->text().toDouble();
    for (int i = 0; i < 3; i++) {
        if (sizes[i] > _domainExtents[i + 3] - _domainExtents[i]) sizes[i] = _domainExtents[i + 3] - _domainExtents[i];
        if (sizes[i] < 0.0) sizes[i] = 0.0;
        if (i < 2 && centers[i] < _domainExtents[i]) centers[i] = _domainExtents[i];
        if (centers[i] > _domainExtents[i + 3]) centers[i] = _domainExtents[i + 3];
        if ((centers[i] + 0.5 * sizes[i]) > _domainExtents[i + 3]) centers[i] = _domainExtents[i + 3] - 0.5 * sizes[i];
        if (i < 2 && (centers[i] - 0.5 * sizes[i]) < _domainExtents[i]) centers[i] = _domainExtents[i] + 0.5 * sizes[i];
        _boxExtents[i] = centers[i] - 0.5 * sizes[i];
        _boxExtents[i + 3] = centers[i] + 0.5 * sizes[i];
    }
    updateGuiValues(centers, sizes);
    emit extentsChanged();
    _silenceSignals = false;
}
// slots:
void BoxSliderFrame::boxTextChanged(const QString &)
{
    if (_silenceSignals) return;
    _textChangedFlag = true;
}

void BoxSliderFrame::boxReturnPressed() { confirmText(); }
void BoxSliderFrame::xSliderCtrChange()
{
    if (_silenceSignals) return;
    _silenceSignals = true;
    // Force new center to conform to current size
    int    pos = xCenterSlider->value();
    double center = _domainExtents[0] + pos * (_domainExtents[3] - _domainExtents[0]) / 256.;
    if ((center + 0.5 * (_boxExtents[3] - _boxExtents[0])) > _domainExtents[3]) center = _domainExtents[3] - 0.5 * (_boxExtents[3] - _boxExtents[0]);
    if ((center - 0.5 * (_boxExtents[3] - _boxExtents[0])) < _domainExtents[0]) center = _domainExtents[0] + 0.5 * (_boxExtents[3] - _boxExtents[0]);
    double size = _boxExtents[3] - _boxExtents[0];
    _boxExtents[0] = center - 0.5 * size;
    _boxExtents[3] = center + 0.5 * size;
    xCenterEdit->setText(QString::number(center));
    xCenterSlider->setValue((int)(0.5 + 256 * (center - _domainExtents[0]) / (_domainExtents[3] - _domainExtents[0])));
    emit extentsChanged();
    _silenceSignals = false;
}
void BoxSliderFrame::xSliderSizeChange()
{
    if (_silenceSignals) return;
    _silenceSignals = true;
    int    pos = xSizeSlider->value();
    double newSize = pos * (_domainExtents[3] - _domainExtents[0]) / 256.;
    // force center to conform to new size.
    double center = 0.5 * (_boxExtents[3] + _boxExtents[0]);
    if ((center + 0.5 * (newSize)) > _domainExtents[3]) center = _domainExtents[3] - 0.5 * (newSize);
    if ((center - 0.5 * (newSize)) < _domainExtents[0]) center = _domainExtents[0] + 0.5 * (newSize);
    _boxExtents[0] = center - 0.5 * newSize;
    _boxExtents[3] = center + 0.5 * newSize;
    xCenterSlider->setValue((int)(0.5 + 256 * (center - _domainExtents[0]) / (_domainExtents[3] - _domainExtents[0])));
    xCenterEdit->setText(QString::number(center));
    xSizeEdit->setText(QString::number(newSize));
    emit extentsChanged();
    _silenceSignals = false;
}
void BoxSliderFrame::ySliderCtrChange()
{
    if (_silenceSignals) return;
    _silenceSignals = true;
    int pos = yCenterSlider->value();
    // Force new center to conform to current size
    double center = _domainExtents[1] + pos * (_domainExtents[4] - _domainExtents[1]) / 256.;
    if ((center + 0.5 * (_boxExtents[4] - _boxExtents[1])) > _domainExtents[4]) center = _domainExtents[4] - 0.5 * (_boxExtents[4] - _boxExtents[1]);
    if ((center - 0.5 * (_boxExtents[4] - _boxExtents[1])) < _domainExtents[1]) center = _domainExtents[1] + 0.5 * (_boxExtents[4] - _boxExtents[1]);
    double size = _boxExtents[4] - _boxExtents[1];
    _boxExtents[1] = center - 0.5 * size;
    _boxExtents[4] = center + 0.5 * size;
    yCenterEdit->setText(QString::number(center));
    yCenterSlider->setValue((int)(0.5 + 256 * (center - _domainExtents[1]) / (_domainExtents[4] - _domainExtents[1])));
    emit extentsChanged();
    _silenceSignals = false;
}
void BoxSliderFrame::ySliderSizeChange()
{
    if (_silenceSignals) return;
    _silenceSignals = true;
    int    pos = ySizeSlider->value();
    double newSize = pos * (_domainExtents[4] - _domainExtents[1]) / 256.;
    // force center to conform to new size.
    double center = 0.5 * (_boxExtents[4] + _boxExtents[1]);
    if ((center + 0.5 * (newSize)) > _domainExtents[4]) center = _domainExtents[4] - 0.5 * (newSize);
    if ((center - 0.5 * (newSize)) < _domainExtents[1]) center = _domainExtents[1] + 0.5 * (newSize);
    _boxExtents[1] = center - 0.5 * newSize;
    _boxExtents[4] = center + 0.5 * newSize;
    yCenterSlider->setValue((int)(0.5 + 256 * (center - _domainExtents[1]) / (_domainExtents[4] - _domainExtents[1])));
    yCenterEdit->setText(QString::number(center));
    ySizeEdit->setText(QString::number(newSize));
    emit extentsChanged();
    _silenceSignals = false;
}
void BoxSliderFrame::zSliderCtrChange()
{
    if (_silenceSignals) return;
    _silenceSignals = true;
    int pos = zCenterSlider->value();
    // Force new center to conform to current size
    double center = _domainExtents[2] + pos * (_domainExtents[5] - _domainExtents[2]) / 256.;
    if ((center + 0.5 * (_boxExtents[5] - _boxExtents[2])) > _domainExtents[5]) center = _domainExtents[5] - 0.5 * (_boxExtents[5] - _boxExtents[2]);
    if ((center - 0.5 * (_boxExtents[5] - _boxExtents[2])) < _domainExtents[2]) center = _domainExtents[2] + 0.5 * (_boxExtents[5] - _boxExtents[2]);
    double size = _boxExtents[5] - _boxExtents[2];
    _boxExtents[2] = center - 0.5 * size;
    _boxExtents[5] = center + 0.5 * size;
    zCenterEdit->setText(QString::number(center));
    zCenterSlider->setValue((int)(0.5 + 256 * (center - _domainExtents[2]) / (_domainExtents[5] - _domainExtents[2])));
    emit extentsChanged();
    _silenceSignals = false;
}
void BoxSliderFrame::zSliderSizeChange()
{
    if (_silenceSignals) return;
    _silenceSignals = true;
    int    pos = zSizeSlider->value();
    double newSize = pos * (_domainExtents[5] - _domainExtents[2]) / 256.;
    // force center to conform to new size.
    double center = 0.5 * (_boxExtents[5] + _boxExtents[2]);
    if ((center + 0.5 * (newSize)) > _domainExtents[5]) center = _domainExtents[5] - 0.5 * (newSize);
    if ((center - 0.5 * (newSize)) < _domainExtents[2]) center = _domainExtents[2] + 0.5 * (newSize);
    _boxExtents[2] = center - 0.5 * newSize;
    _boxExtents[5] = center + 0.5 * newSize;
    zCenterSlider->setValue((int)(0.5 + 256 * (center - _domainExtents[2]) / (_domainExtents[5] - _domainExtents[2])));
    zCenterEdit->setText(QString::number(center));
    zSizeEdit->setText(QString::number(newSize));
    emit extentsChanged();
    _silenceSignals = false;
}
void BoxSliderFrame::updateGuiValues(const double centers[3], const double sizes[3])
{
    xCenterEdit->setText(QString::number(centers[0]));
    yCenterEdit->setText(QString::number(centers[1]));
    zCenterEdit->setText(QString::number(centers[2]));
    xSizeEdit->setText(QString::number(sizes[0]));
    ySizeEdit->setText(QString::number(sizes[1]));
    zSizeEdit->setText(QString::number(sizes[2]));
    for (int i = 0; i < 3; i++) _lastCenterSlider[i] = (int)(0.5 + 256 * (centers[i] - _domainExtents[i]) / (_domainExtents[3 + i] - _domainExtents[i]));
    xCenterSlider->setValue(_lastCenterSlider[0]);
    yCenterSlider->setValue(_lastCenterSlider[1]);
    zCenterSlider->setValue(_lastCenterSlider[2]);
    xSizeSlider->setValue((int)(0.5 + 256 * sizes[0] / (_domainExtents[3] - _domainExtents[0])));
    ySizeSlider->setValue((int)(0.5 + 256 * sizes[1] / (_domainExtents[4] - _domainExtents[1])));
    zSizeSlider->setValue((int)(0.5 + 256 * sizes[2] / (_domainExtents[5] - _domainExtents[2])));
}
// Nudge events:
void BoxSliderFrame::nudgeXCenter(int val)
{
    if (_silenceSignals) return;

    if (!_controlExec->GetDataMgr()) return;
    // ignore if change is not 1
    if (abs(val - _lastCenterSlider[0]) != 1) {
        _lastCenterSlider[0] = val;
        return;
    }
    nudgeCenter(_varname, val, 0);
    emit extentsChanged();
}

// Nudge events:
void BoxSliderFrame::nudgeYCenter(int val)
{
    if (_silenceSignals) return;

    if (!_controlExec->GetDataMgr()) return;
    // ignore if change is not 1
    if (abs(val - _lastCenterSlider[1]) != 1) {
        _lastCenterSlider[1] = val;
        return;
    }
    nudgeCenter(_varname, val, 1);
    emit extentsChanged();
}    // Nudge events:
void BoxSliderFrame::nudgeZCenter(int val)
{
    if (_silenceSignals) return;

    if (!_controlExec->GetDataMgr()) return;
    // ignore if change is not 1
    if (abs(val - _lastCenterSlider[2]) != 1) {
        _lastCenterSlider[2] = val;
        return;
    }
    nudgeCenter(_varname, val, 2);
    emit extentsChanged();
}
void BoxSliderFrame::nudgeCenter(string varname, int val, int dir)
{
    // See if the change was an increase or decrease:

    GUIStateParams * p = MainForm::getInstance()->GetStateParams();
    string           activeViz = p->GetActiveVizName();
    AnimationParams *animP = _controlExec->GetParamsMgr()->GetAnimationParams();

    size_t timeStep = animP->GetCurrentTimestep();

    vector<double> minExts, maxExts;
    _dataStatus->GetExtents(timeStep, minExts, maxExts);
    float  voxelSize = _dataStatus->getVoxelSize(timeStep, varname, _numRefinements, dir);
    double pmin = _boxExtents[dir];
    double pmax = _boxExtents[dir + 3];
    float  maxExtent = maxExts[dir];
    float  minExtent = minExts[dir];
    double newCenter = (pmin + pmax) * 0.5f;
    if (val > _lastCenterSlider[dir]) {    // move by 1 voxel, but don't move past end
        _lastCenterSlider[dir]++;
        if (pmax + voxelSize <= maxExtent) {
            _boxExtents[dir] = pmin + voxelSize;
            _boxExtents[dir + 3] = pmax + voxelSize;
            newCenter += voxelSize;
        }
    } else {
        _lastCenterSlider[dir]--;
        if (pmin - voxelSize >= minExtent) {    // slide 1 voxel down:
            _boxExtents[dir] = pmin - voxelSize;
            _boxExtents[dir + 3] = pmax - voxelSize;
            newCenter -= voxelSize;
        }
    }
    // Determine where the slider really should be:

    int newSliderPos = (int)(256. * (newCenter - minExtent) / (maxExtent - minExtent) + 0.5f);
    if (_lastCenterSlider[dir] != newSliderPos) { _lastCenterSlider[dir] = newSliderPos; }
    return;
}
// nudge size
void BoxSliderFrame::nudgeXSize(int val)
{
    if (_silenceSignals) return;

    if (!_controlExec->GetDataMgr()) return;
    // ignore if change is not 1
    if (abs(val - _lastSizeSlider[0]) != 1) {
        _lastSizeSlider[0] = val;
        return;
    }
    nudgeSize(val, 0);
    emit extentsChanged();
}

// Nudge events:
void BoxSliderFrame::nudgeYSize(int val)
{
    if (_silenceSignals) return;
    if (!_controlExec->GetDataMgr()) return;
    // ignore if change is not 1
    if (abs(val - _lastSizeSlider[1]) != 1) {
        _lastSizeSlider[1] = val;
        return;
    }
    nudgeSize(val, 1);
    emit extentsChanged();
}    // Nudge events:
void BoxSliderFrame::nudgeZSize(int val)
{
    if (_silenceSignals) return;
    if (!_controlExec->GetDataMgr()) return;
    // ignore if change is not 1
    if (abs(val - _lastSizeSlider[2]) != 1) {
        _lastSizeSlider[2] = val;
        return;
    }
    nudgeSize(val, 2);
    emit extentsChanged();
}
void BoxSliderFrame::nudgeSize(int val, int dir)
{
    // See if the change was an increase or decrease:

    GUIStateParams * p = MainForm::getInstance()->GetStateParams();
    string           activeViz = p->GetActiveVizName();
    AnimationParams *animP = _controlExec->GetParamsMgr()->GetAnimationParams();

    size_t timeStep = animP->GetCurrentTimestep();

    float  voxelSize = _dataStatus->getVoxelSize(timeStep, _varname, _numRefinements, dir);
    double pmin = _boxExtents[dir];
    double pmax = _boxExtents[dir + 3];
    float  maxExtent = _dataStatus->getLocalExtents()[dir + 3];
    float  minExtent = _dataStatus->getLocalExtents()[dir];
    double newSize = (pmax - pmin);
    if (val > _lastSizeSlider[dir]) {    // increase by 1 voxel, but don't move past end
        _lastSizeSlider[dir]++;
        if (pmax + voxelSize < maxExtent) {
            _boxExtents[dir + 3] = pmax + voxelSize;
            newSize += voxelSize;
        }
        if (pmin - voxelSize > minExtent) {
            _boxExtents[dir] = pmin - voxelSize;
            newSize += voxelSize;
        }
    } else {
        _lastSizeSlider[dir]--;
        if (pmin + voxelSize <= pmax) {
            _boxExtents[dir] = pmin + voxelSize;
            newSize -= voxelSize;
        }
        if (pmin + voxelSize <= pmax - voxelSize) {
            _boxExtents[dir + 3] = pmax - voxelSize;
            newSize -= voxelSize;
        }
    }
    // Determine where the slider really should be:

    int newSliderPos = (int)(256. * newSize / (maxExtent - minExtent) + 0.5f);
    if (_lastSizeSlider[dir] != newSliderPos) { _lastSizeSlider[dir] = newSliderPos; }
    return;
}
