
#ifndef IMAGEPARAMS_H
    #define IMAGEPARAMS_H

    #include <vapor/RenderParams.h>
    #include <vapor/DataMgr.h>

namespace VAPoR {
//! \class ImageParams
//! \brief Class that supports drawing Barbs based on 2D or 3D vector field
class PARAMS_API ImageParams : public RenderParams {
public:
    ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, std::string &fileName);
    ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, std::string *fileName, XmlNode *xmlNode);

    virtual ~ImageParams();

    static std::string getClassType() { return ("ImageParams"); }

    bool isGeoTIFF() const { return _isGeoTIFF; }
    void treatAsGeoTIFF() { _isGeoTIFF = true; }
    void doNotTreatAsGeoTIFF() { _isGeoTIFF = false; }

    bool isTransparencyIgnored() const { return _ignoreTransparence; }
    void ignoreTransparency() { _ignoreTransparence = true; }
    void doNotIgnoreTransparency() { _ignoreTransparence = false; }

    float getOpacity() const { return _opacity; }
    float setOpacity(float val) { _opacity = val; }

private:
    bool        _isGeoTIFF;
    bool        _ignoreTransparence;
    float       _opacity;
    std::string _fileName;

    void _init();
};
}    // namespace VAPoR
