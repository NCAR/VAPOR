#include <vapor/ImageParams.h>

using namespace VAPoR;

//
// Register class with object factory
//
static RenParamsRegistrar<ImageParams> registrar(ImageParams::GetClassType());

ImageParams::ImageParams(DataMgr *dataManager,
                         ParamsBase::StateSave *stateSave)
    : RenderParams(dataManager,
                   stateSave,
                   ImageParams::GetClassType(),
                   2) {
    SetDiagMsg("ImageParams::ImageParams() this=%p", this);
}

ImageParams::ImageParams(DataMgr *dataManager,
                         ParamsBase::StateSave *stateSave,
                         XmlNode *node)
    : RenderParams(dataManager,
                   stateSave,
                   ImageParams::GetClassType(),
                   2) {
    SetDiagMsg("ImageParams::ImageParams() this=%p", this);
    if (node->GetTag() != ImageParams::GetClassType()) {
        node->SetTag(ImageParams::GetClassType());
    }
}

ImageParams::~ImageParams() {
    SetDiagMsg("ImageParams::~ImageParams() this=%p", this);
}
