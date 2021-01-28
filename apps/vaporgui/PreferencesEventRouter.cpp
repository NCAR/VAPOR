#include <PreferencesEventRouter.h>
#include <vapor/PreferencesParams.h>
#include "PWidgets.h"
#include "PTMSLODInput.h"

using namespace VAPoR;
typedef PreferencesParams IP;

static RenderEventRouterRegistrar<PreferencesEventRouter> registrar(PreferencesEventRouter::GetClassType());

PreferencesEventRouter::PreferencesEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, PreferencesParams::GetClassType())
{
    // clang-format off

    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new PHeightVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddSubtab("Appearance", new PSection("Preferences", {
        new PCheckbox(IP::_isGeoRefTag, "Geo Reference"),
        new PCheckbox(IP::_ignoreTransparencyTag, "Ingore Transparency"),
        (new PDoubleSliderEdit(IP::_constantOpacityTag, "Opacity"))->EnableDynamicUpdate(),
        (new PFileOpenSelector(IP::_fileNameTag, "Preferences File"))->SetFileTypeFilter("TIFF files, tiled images (*.tiff *.tif *.gtif *.tms)"),
        new PTMSLODInput()
    }));
    
    AddSubtab("Geometry", new PGeometrySubtab);

    // clang-format on
}

string PreferencesEventRouter::_getDescription() const
{
    return ("Displays a "
            "georeferenced image that is automatically reprojected and fit to the user's"
            "data, as long as the data contains georeference metadata.  The image "
            "renderer may be offset by a height variable to show bathymetry or mountainous"
            " terrain.\n\n ");
}
