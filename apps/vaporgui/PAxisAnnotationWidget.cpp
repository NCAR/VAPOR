#include <QLabel>
#include <vapor/ControlExecutive.h>
#include <vapor/RenderParams.h>
#include <vapor/DataMgrUtils.h>
#include "PAxisAnnotationWidget.h"
#include "VSection.h"
#include "V3DInput.h"
#include "V3DIntInput.h"
#include "VLineItem.h"
#include <vapor/AnimationParams.h>
#include "ErrorReporter.h"

PAxisAnnotationWidget::PAxisAnnotationWidget(VAPoR::ControlExec *controlExec) : PWidget("", _group = new VGroup())
{
    _controlExec = controlExec;

    _group->Add(new VLineItem("# tics ", _numTics = new V3DIntInput));
    _group->Add(new VLineItem("Size   ", _size = new V3DInput));
    _group->Add(new VLineItem("Min    ", _min = new V3DInput));
    _group->Add(new VLineItem("Max    ", _max = new V3DInput));
    _group->Add(new VLineItem("Origin ", _origin = new V3DInput));

    QObject::connect(_numTics, &V3DIntInput::ValueChangedVec, this, &PAxisAnnotationWidget::_numTicsChanged);
    QObject::connect(_size, &V3DInput::ValueChangedVec, this, &PAxisAnnotationWidget::_sizeChanged);
    QObject::connect(_min, &V3DInput::ValueChangedVec, this, &PAxisAnnotationWidget::_minChanged);
    QObject::connect(_max, &V3DInput::ValueChangedVec, this, &PAxisAnnotationWidget::_maxChanged);
    QObject::connect(_origin, &V3DInput::ValueChangedVec, this, &PAxisAnnotationWidget::_originChanged);
}

void PAxisAnnotationWidget::updateGUI() const
{
    VAPoR::AxisAnnotation *aa = getParams<VAPoR::AxisAnnotation>();

    std::vector<double> dTics = aa->GetNumTics();
    std::vector<int>    iTics(dTics.begin(), dTics.end());
    _numTics->SetValue(iTics);
    _size->SetValue(aa->GetTicSize());

    std::vector<double> minTics = aa->GetMinTics();
    std::vector<double> maxTics = aa->GetMaxTics();
    std::vector<double> origin = aa->GetAxisOrigin();
    _scaleNormalizedCoordsToWorld(minTics);
    _scaleNormalizedCoordsToWorld(maxTics);
    _scaleNormalizedCoordsToWorld(origin);

    if (aa->GetLatLonAxesEnabled()) {
        _convertPCSToLonLat(minTics[0], minTics[1]);
        _convertPCSToLonLat(maxTics[0], maxTics[1]);
        _convertPCSToLonLat(origin[0], origin[1]);
    }

    _min->SetValue(minTics);
    _max->SetValue(maxTics);
    _origin->SetValue(origin);
}

void PAxisAnnotationWidget::_numTicsChanged(const std::vector<int> xyz)
{
    std::vector<double> dTics(xyz.begin(), xyz.end());
    getParams<VAPoR::AxisAnnotation>()->SetNumTics(dTics);
}

void PAxisAnnotationWidget::_sizeChanged(const std::vector<double> xyz) { getParams<VAPoR::AxisAnnotation>()->SetTicSize(xyz); }

void PAxisAnnotationWidget::_minChanged(std::vector<double> xyz)
{
    VAPoR::AxisAnnotation *aa = getParams<VAPoR::AxisAnnotation>();
    if (aa->GetLatLonAxesEnabled()) _convertLonLatToPCS(xyz[0], xyz[1]);

    _scaleWorldCoordsToNormalized(xyz);
    getParams<VAPoR::AxisAnnotation>()->SetMinTics(xyz);
}

void PAxisAnnotationWidget::_maxChanged(std::vector<double> xyz)
{
    VAPoR::AxisAnnotation *aa = getParams<VAPoR::AxisAnnotation>();
    if (aa->GetLatLonAxesEnabled()) _convertLonLatToPCS(xyz[0], xyz[1]);

    _scaleWorldCoordsToNormalized(xyz);
    getParams<VAPoR::AxisAnnotation>()->SetMaxTics(xyz);
}

void PAxisAnnotationWidget::_originChanged(std::vector<double> xyz)
{
    VAPoR::AxisAnnotation *aa = getParams<VAPoR::AxisAnnotation>();
    if (aa->GetLatLonAxesEnabled()) _convertLonLatToPCS(xyz[0], xyz[1]);

    _scaleWorldCoordsToNormalized(xyz);
    getParams<VAPoR::AxisAnnotation>()->SetAxisOrigin(xyz);
}

void PAxisAnnotationWidget::_scaleNormalizedCoordsToWorld(std::vector<double> &coords) const
{
    std::vector<double> extents = _getDomainExtents();
    int                 size = extents.size() / 2;
    for (int i = 0; i < size; i++) {
        double offset = coords[i] * (extents[i + 3] - extents[i]);
        double minimum = extents[i];
        coords[i] = offset + minimum;
    }
}

void PAxisAnnotationWidget::_scaleWorldCoordsToNormalized(std::vector<double> &coords) const
{
    std::vector<double> extents = _getDomainExtents();
    int                 size = extents.size() / 2;
    for (int i = 0; i < size; i++) {
        double point = coords[i] - extents[i];
        double magnitude = extents[i + 3] - extents[i];
        coords[i] = point / magnitude;
    }
}

std::vector<double> PAxisAnnotationWidget::_getDomainExtents() const
{
    VAPoR::ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    AnimationParams * aParams = dynamic_cast<AnimationParams *>(paramsMgr->GetParams(AnimationParams::GetClassType()));
    VAssert(aParams);
    int                 ts = aParams->GetCurrentTimestep();
    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<double> minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);

    std::vector<double> extents = {minExts[0], minExts[1], minExts[2], maxExts[0], maxExts[1], maxExts[2]};
    return extents;
}

void PAxisAnnotationWidget::_convertPCSToLonLat(double &xCoord, double &yCoord) const
{
    VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
    string             projString = dataStatus->GetMapProjection();
    double             coords[2] = {xCoord, yCoord};
    double             coordsForError[2] = {coords[0], coords[1]};

    int rc = VAPoR::DataMgrUtils::ConvertPCSToLonLat(projString, coords, 1);
    if (rc < 0) {
        MyBase::SetErrMsg("Could not convert point %f, %f to Lon/Lat", coordsForError[0], coordsForError[1]);
        MSG_ERR("Error converting PCS to Lat-Lon coordinates");
    }

    xCoord = coords[0];
    yCoord = coords[1];
}

void PAxisAnnotationWidget::_convertLonLatToPCS(double &xCoord, double &yCoord) const
{
    VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
    string             projString = dataStatus->GetMapProjection();
    double             coords[2] = {xCoord, yCoord};
    double             coordsForError[2] = {coords[0], coords[1]};

    int rc = VAPoR::DataMgrUtils::ConvertLonLatToPCS(projString, coords, 1);
    if (rc < 0) {
        MyBase::SetErrMsg("Could not convert point %f, %f to PCS", coordsForError[0], coordsForError[1]);
        MSG_ERR("Error converting from Lat-Lon to PCS coordinates");
    }

    xCoord = coords[0];
    yCoord = coords[1];
}
