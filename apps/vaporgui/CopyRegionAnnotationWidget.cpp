#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include "vapor/Renderer.h"
#include "vapor/ParamsMgr.h"
#include "vapor/RenderParams.h"
#include <vapor/AnimationParams.h>
#include "vapor/DataMgrUtils.h"
#include <vapor/ControlExecutive.h>
#include "CopyRegionAnnotationWidget.h"
#include "VLineItem.h"

#define X 0
#define Y 1
#define Z 2

using namespace VAPoR;

namespace {
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
}    // namespace

CopyRegionAnnotationWidget::CopyRegionAnnotationWidget(VAPoR::ControlExec *ce) : CopyRegionWidget() { _controlExec = ce; }

void CopyRegionAnnotationWidget::Update()
{
    _paramsMgr = _controlExec->GetParamsMgr();
    VAssert(_paramsMgr);
    updateCopyCombo();
}

void CopyRegionAnnotationWidget::copyRegion()
{
    string copyString = copyCombo->currentText().toStdString();
    if (copyString != "") {
        std::vector<std::string> elems = split(copyString, ':');
        string                   visualizer = _visNames[elems[0]];
        string                   dataSetName = elems[1];
        string                   renType = _renTypeNames[elems[2]];
        string                   renderer = elems[3];

        RenderParams *copyParams = _paramsMgr->GetRenderParams(visualizer, dataSetName, renType, renderer);
        VAssert(copyParams);

        Box *               copyBox = copyParams->GetBox();
        std::vector<double> minExtents, maxExtents;
        copyBox->GetExtents(minExtents, maxExtents);
        VAssert(minExtents.size() == maxExtents.size());

        AnnotationParams *a = _paramsMgr->GetAnnotationParams(visualizer);
        AxisAnnotation *  aa = a->GetAxisAnnotation();

        AnimationParams *aParams = dynamic_cast<AnimationParams *>(_paramsMgr->GetParams(AnimationParams::GetClassType()));
        VAssert(aParams);
        int timeStep = aParams->GetCurrentTimestep();

        _scaleWorldCoordsToNormalized(minExtents, maxExtents, timeStep);

        aa->SetAxisOrigin(minExtents);
        aa->SetMinTics(minExtents);
        aa->SetMaxTics(maxExtents);

        emit valueChanged();
    }
}

void CopyRegionAnnotationWidget::_scaleWorldCoordsToNormalized(std::vector<double> &minExts, std::vector<double> &maxExts, int timeStep)
{
    std::vector<double> minDomainExts, maxDomainExts;
    DataStatus *        dataStatus = _controlExec->GetDataStatus();
    dataStatus->GetActiveExtents(_paramsMgr, timeStep, minDomainExts, maxDomainExts);
    VAssert(minExts.size() == maxExts.size());

    int size = minExts.size();
    for (int i = 0; i < size; i++) {
        double point = minExts[i] - minDomainExts[i];
        minExts[i] = point / (maxDomainExts[i] - minDomainExts[i]);
        point = maxExts[i] - minDomainExts[i];
        maxExts[i] = point / (maxDomainExts[i] - minDomainExts[i]);
    }
}
