#include <vapor/ImageParams.h>
#include <vapor/ResourcePath.h>

using namespace VAPoR;

const std::string ImageParams::_fileNameTag = "FileName";
const std::string ImageParams::_isGeoRefTag = "IsGeoRefTag";
const std::string ImageParams::_ignoreTransparencyTag = "IgnoreTransparency";
const std::string ImageParams::_opacityTag = "Opacity";
const std::string ImageParams::_orientationTag = "Orientation";

//
// Register class with object factory
//
static RenParamsRegistrar<ImageParams> registrar(ImageParams::GetClassType());

ImageParams::ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave) : RenderParams(dataManager, stateSave, ImageParams::GetClassType(), 2)
{
    SetVariableName("");
    SetFieldVariableNames(vector<string>());
    SetDiagMsg("ImageParams::ImageParams() this=%p", this);

    //
    // The image renderer behaves like a 2D renderer, but it doesn't operate
    // on any data variables. If InitBox() fails to initialize using 2D
    // data variables (because none are present in the data collection) try
    // to initialize the Box using 3D variables
    //
    bool ok = InitBox(2);
    if (!ok) {
        InitBox(3);
        GetBox()->SetOrientation(VAPoR::Box::XY);
        GetBox()->SetPlanar(true);
    }
}

ImageParams::ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *node) : RenderParams(dataManager, stateSave, node, 2) {}

ImageParams::~ImageParams() { SetDiagMsg("ImageParams::~ImageParams() this=%p", this); }

std::string ImageParams::GetImagePath() const
{
    std::string defaultImage = Wasp::GetSharePath("images/NaturalEarth.tms");

    return GetValueString(_fileNameTag, defaultImage);
}
