#include <vapor/ImageParams.h>

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
    SetDiagMsg("ImageParams::ImageParams() this=%p", this);
}

ImageParams::ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *node) : RenderParams(dataManager, stateSave, node, 2)
{
    SetDiagMsg("ImageParams::ImageParams() this=%p", this);
    if (node->GetTag() != ImageParams::GetClassType()) { node->SetTag(ImageParams::GetClassType()); }
}

ImageParams::~ImageParams() { SetDiagMsg("ImageParams::~ImageParams() this=%p", this); }
