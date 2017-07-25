#include <vapor/ImageParams.cpp>

using namespace VAPoR;

//
// Register class with object factory
//
static RenParamsRegistrar<ImageParams> registrar(ImageParams::GetClassType());

ImageParams::ImageParams(DataMgr *dataManager,
                         ParamsBase::StateSave *stateSave,
                         std::string file) : RenderParams(dataManager,
                                                          stateSave,
                                                          ImageParams::GetClassType(),
                                                          2) {
    SetDiagMsg("ImageParams::ImageParams() this=%p", this);
    _fileName = file;
    _init();
}

ImageParams::ImageParams(DataMgr *dataManager,
                         ParamsBase::StateSave *stateSave,
                         std::string file,
                         XmlNode *node) : RenderParams(dataManager,
                                                       stateSave,
                                                       ImageParams::GetClassType(),
                                                       2) {
    SetDiagMsg("ImageParams::ImageParams() this=%p", this);
    if (node->GetTag() != ImageParams::GetClasstype()) {
        node->SetTag(ImageParams::GetClasstype());
        _fileName = file;
        _init();
    }
}

ImageParams::~ImageParams() {
    SetDiagMsg("ImageParams::~ImageParams() this=%p", this);
}

ImageParams::_init() {
    _isGeoTIFF = false;
    _ignoreTransparence = false;
    _opacity = -1.0;
}
