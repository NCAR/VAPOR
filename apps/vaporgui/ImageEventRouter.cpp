#include <ImageEventRouter.h>
#include <vapor/ImageParams.h>
#include "PWidgets.h"
#include "PTMSLODInput.h"

using namespace VAPoR;
typedef ImageParams IP;

static RenderEventRouterRegistrar<ImageEventRouter> registrar(ImageEventRouter::GetClassType());

ImageEventRouter::ImageEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, ImageParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PHeightVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddAppearanceSubtab(new PSection("Image", {
        new PCheckbox(IP::_isGeoRefTag, "Geo Reference"),
        new PCheckbox(IP::_ignoreTransparencyTag, "Ingore Transparency"),
        (new PDoubleSliderEdit(IP::_constantOpacityTag, "Opacity"))->EnableDynamicUpdate(),
        (new PFileOpenSelector(IP::_fileNameTag, "Image File"))->SetFileTypeFilter("TIFF files, tiled images (*.tiff *.tif *.gtif *.tms)"),
        new PTMSLODInput()
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);

    // clang-format on
}

string ImageEventRouter::_getDescription() const
{
    return ("Displays a "
            "georeferenced image that is automatically reprojected and fit to the user's"
            "data, as long as the data contains georeference metadata.  The image "
            "renderer may be offset by a height variable to show bathymetry or mountainous"
            " terrain.\n\n ");
}
