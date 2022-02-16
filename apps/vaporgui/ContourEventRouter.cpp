#include "ContourEventRouter.h"
#include "vapor/ContourParams.h"
#include "PWidgets.h"
#include "PSliderEditHLI.h"
#include "PConstantColorWidget.h"
#include "PSliceOriginSelector.h"

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
        new PFidelitySection
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
        (new PShowIf(""))->DimensionEquals(3)->Then(new PGroup({
            (new PSection("Slice Orientation", {
                new PEnumDropdown(RenderParams::SlicePlaneOrientationModeTag, {"Rotation", "Normal"}, {(int)RenderParams::SlicePlaneOrientationMode::Rotation, (int)RenderParams::SlicePlaneOrientationMode::Normal}, "Orientation Mode"),
                
                (new PShowIf(RenderParams::SlicePlaneOrientationModeTag))->Equals((int)RenderParams::SlicePlaneOrientationMode::Rotation)->Then({
                    (new PDoubleSliderEdit(RenderParams::XSlicePlaneRotationTag, "X"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
                    (new PDoubleSliderEdit(RenderParams::YSlicePlaneRotationTag, "Y"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
                    (new PDoubleSliderEdit(RenderParams::ZSlicePlaneRotationTag, "Z"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
                })->Else({
                    (new PDoubleSliderEdit(RenderParams::SlicePlaneNormalXTag, "X"))->SetRange(-1,1)->EnableDynamicUpdate(),
                    (new PDoubleSliderEdit(RenderParams::SlicePlaneNormalYTag, "Y"))->SetRange(-1,1)->EnableDynamicUpdate(),
                    (new PDoubleSliderEdit(RenderParams::SlicePlaneNormalZTag, "Z"))->SetRange(-1,1)->EnableDynamicUpdate(),
                }),
            }))->SetTooltip("The plane normal of the slice. The offset will move the slice along this normal as well."),
            (new PSliceOffsetSelector)->SetTooltip("Offset the plane from its origin along its normal (set by the orientation)."),
            (new PSliceOriginSelector)->SetTooltip("The slice plane will pass through this point. The plane can be offset from this point along the plane normal determined by the orientation."),
        })),
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
