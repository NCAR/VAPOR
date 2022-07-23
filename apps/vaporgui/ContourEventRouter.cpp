#include "ContourEventRouter.h"
#include "vapor/ContourParams.h"
#include "PWidgets.h"
#include "PSliderEditHLI.h"
#include "PConstantColorWidget.h"
#include "PSliceController.h"
#include "PMetadataClasses.h"

using namespace VAPoR;
typedef ContourParams CP;

static RenderEventRouterRegistrar<ContourEventRouter> registrar(ContourEventRouter::GetClassType());

ContourEventRouter::ContourEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, ContourParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PDimensionSelector,
            new PScalarVariableSelector,
            new PHeightVariableSelector,
        }),
        new PFidelitySection,
        new POpenVariableMetadataWidget
    }));
    
    AddAppearanceSubtab(new PGroup({
        new PTFEditor(RenderParams::_variableNameTag, {
            PTFEditor::Default,
            PTFEditor::RegularIsoArray
        }),
        new PSection("Contour Lines", {
            _spacingSlider = new PDoubleSliderEditHLI<CP>("Spacing", &CP::GetContourSpacing, &CP::SetContourSpacing),
            (new PIntegerSliderEditHLI<CP>("Count", &CP::GetContourCount, &CP::SetContourCount))->SetRange(1, 50),
            _minValueSlider = new PDoubleSliderEditHLI<CP>("Minimum Value", &CP::GetContourMin, &CP::SetContourMin),
            new PConstantColorWidget
        }),
        (new PShowIf(""))->DimensionEquals(3)->Then(new PGroup({
            new PSection("Slice", {
                (new PDoubleSliderEdit(RenderParams::SampleRateTag, "N Samples"))->SetRange(32, 2000)
            })
        })),
    }));
    
    AddGeometrySubtab(new PGroup({
        (new PShowIf(""))->DimensionEquals(3)->Then(new PGroup({ new PSliceController })),
        new PGeometrySubtab,
    }));
    
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

void ContourEventRouter::_updateTab()
{
    RenderParams *rp = GetActiveParams();
    string        varname = rp->GetVariableName();
    size_t        ts = rp->GetCurrentTimestep();
    int           level = rp->GetRefinementLevel();
    int           lod = rp->GetCompressionLevel();

    CoordType minExt = {0.0, 0.0, 0.0};
    CoordType maxExt = {0.0, 0.0, 0.0};
    rp->GetBox()->GetExtents(minExt, maxExt);

    vector<double> dataExts(2, 0);
    GetActiveDataMgr()->GetDataRange(ts, varname, level, lod, minExt, maxExt, dataExts);

    _spacingSlider->SetRange(0, dataExts[1] - dataExts[0]);
    _minValueSlider->SetRange(dataExts[0], dataExts[1]);

    RenderEventRouterGUI::_updateTab();
}

string ContourEventRouter::_getDescription() const
{
    return ("Displays "
            "a series of user defined contours along a two dimensional plane within the "
            "user's domain.\n\nContours may hae constant coloration, or may be colored "
            "according to a secondary variable.\n\nContours may be displaced by a height "
            "variable.\n\n ");
}
