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
}

ImageParams::ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *node) : RenderParams(dataManager, stateSave, node, 2) {}

ImageParams::~ImageParams() { SetDiagMsg("ImageParams::~ImageParams() this=%p", this); }

int ImageParams::Initialize()
{
    int rc = RenderParams::Initialize();
    if (rc < 0) return (rc);

    // The image renderer behaves like a 2D renderer, but it doesn't operate
    // on any data variables. Make sure the box is planar.
    //
    GetBox()->SetOrientation(VAPoR::Box::XY);
    GetBox()->SetPlanar(true);

    return (0);
}

std::string ImageParams::GetImagePath() const
{
    std::string defaultImage = Wasp::GetSharePath("images/NaturalEarth.tms");

    return GetValueString(_fileNameTag, defaultImage);
}
