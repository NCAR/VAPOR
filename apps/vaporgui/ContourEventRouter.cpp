#include "ContourEventRouter.h"
#include "vapor/ContourParams.h"
#include "PWidgets.h"
#include "PSliderEditHLI.h"

using namespace VAPoR;
typedef ContourParams CP;

static RenderEventRouterRegistrar<ContourEventRouter> registrar(ContourEventRouter::GetClassType());

ContourEventRouter::ContourEventRouter(QWidget *parent, ControlExec *ce)
: RenderEventRouterGUI(ce, ContourParams::GetClassType())
{
    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector,
            new PHeightVariableSelector,
        }),
        new PFidelitySection
    }));
    
    AddSubtab("Appearance", new PGroup({
        new PTFEditor(RenderParams::_variableNameTag, {
            PTFEditor::Default,
            PTFEditor::RegularIsoArray
        }),
        new PSection("Contour Lines", {
            _spacingSlider = new PDoubleSliderEditHLI<CP>("Spacing", &CP::GetContourSpacing, &CP::SetContourSpacing),
            (new PIntegerSliderEditHLI<CP>("Count", &CP::GetContourCount, &CP::SetContourCount))->SetRange(1, 50),
            _minValueSlider = new PDoubleSliderEditHLI<CP>("Minimum Value", &CP::GetContourMin, &CP::SetContourMin),
        }),
    }));
    
    AddSubtab("Geometry", new PGeometrySubtab);
    AddSubtab("Annotation", new PAnnotationColorbarWidget);
}

void ContourEventRouter::_updateTab()
{
    RenderParams *rp = GetActiveParams();
    string varname = rp->GetVariableName();
    size_t ts = rp->GetCurrentTimestep();
    int level = rp->GetRefinementLevel();
    int lod = rp->GetCompressionLevel();

    vector<double> minExt, maxExt;
    rp->GetBox()->GetExtents(minExt, maxExt);

    vector<double> dataExts(2,0);
    GetActiveDataMgr()->GetDataRange(ts, varname, level, lod, minExt, maxExt, dataExts);
    
    _spacingSlider->SetRange(0, dataExts[1] - dataExts[0]);
    _minValueSlider->SetRange(dataExts[0], dataExts[1]);
    
    RenderEventRouterGUI::_updateTab();
}

string ContourEventRouter::_getDescription() const {
	return (
	"Displays "
    "a series of user defined contours along a two dimensional plane within the "
    "user's domain.\n\nContours may hae constant coloration, or may be colored "
    "according to a secondary variable.\n\nContours may be displaced by a height "
    "variable.\n\n "
	);
}
