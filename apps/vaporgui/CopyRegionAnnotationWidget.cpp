#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include "vapor/Renderer.h"
#include "vapor/ParamsMgr.h"
#include "vapor/RenderParams.h"
#include "vapor/DataMgrUtils.h"
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

CopyRegionAnnotationWidget::CopyRegionAnnotationWidget( VAPoR::ControlExecutive* ce ) : CopyRegionWidget(), ControlExecWidget(ce) {}

void CopyRegionAnnotationWidget::Update(ParamsMgr* paramsMgr) {
    VAssert(paramsMgr);
    _paramsMgr = paramsMgr;
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

/*        Box *               myBox = _rParams->GetBox();
        std::vector<double> myMin = _rParams->GetMinTics();
        std::vector<double> myMax = _rParams->GetMaxTics();
        myBox->GetExtents(myMin, myMax);
        VAssert(myMin.size() == myMax.size());
        for (int i = 0; i < myMin.size(); i++) {
            myMin[i] = minExtents[i];
            myMax[i] = maxExtents[i];
        }*/

        //_configurePlanarBox(myBox, &myMin, &myMax);

        //myBox->SetExtents(myMin, myMax);
        AnnotationParams* a = _paramsMgr->GetAnnotationParams( visualizer );
        AxisAnnotation* aa  = a->GetAxisAnnotation();
        std::cout << minExtents[0] << " " << minExtents[1] << " " << minExtents[2] << std::endl;
        std::cout << maxExtents[0] << " " << maxExtents[1] << " " << maxExtents[2] << std::endl;
        aa->SetMinTics( minExtents );
        aa->SetMaxTics( maxExtents );

        emit valueChanged();
    }
}
