#ifndef IMAGEPARAMS_H
#define IMAGEPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/TMSUtils.h>

namespace VAPoR {

class PARAMS_API ImageParams : public RenderParams {
public:
    ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave);
    ImageParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    virtual ~ImageParams();

    virtual int Initialize() override;

    static std::string GetClassType() { return ("ImageParams"); }

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override { return (2); }

    //! Set file path for the image to be read and displayed.
    //! \param[in] string - Path to image file
    void SetImagePath(std::string file)
    {
        BeginGroup("Set image path");
        SetValueString(_fileNameTag, "Set image file path", file);
        if (Wasp::TMSUtils::IsTMSFile(file)) {
            int numTMSLODs = Wasp::TMSUtils::GetNumTMSLODs(file);
            _setNumTMSLODs(numTMSLODs);
        }
        EndGroup();
    }

    //! Set image file path
    //! \retval string - Path to image file
    std::string GetImagePath() const;

    //! Inquire whether the currently selected image is georeferenced
    //! Valid values: 0 = do not use georeference information, 1 = use georeference information
    //! \retval bool - State indicating whether current image is georeferenced
    bool GetIsGeoRef() const { return (GetValueLong(_isGeoRefTag, (long)true)); }

    //! If the raster image contained in the path returned by GetImagePath() has georeference information (e.g. the file is a GeoTIFF) this boolean determines whether or not the georeference information is honored
    //! Valid values: 0 = do not use georeference information, 1 = use georeference information
    //! \param[in] bool - State indicating whether current image is georeferenced
    void SetIsGeoRef(bool val) { SetValueLong(_isGeoRefTag, "Geo-reference the image", (long)val); }

    //! Get whether transparency is being ignored regarding the currently selected image
    //! Valid values: 0 = transparency is being honored, 1 = transparency is being ignored
    //! \retval bool - State whether transparency is being ignored
    bool GetIgnoreTransparency() const { return (0 != GetValueLong(_ignoreTransparencyTag, (long)false)); }

    //! Set whether transparency is being ignored regarding the currently selected image
    //! Valid values: 0 = transparency is being honored, 1 = transparency is being ignored
    //! \param[in] bool - State whether transparency is being ignored
    void SetIgnoreTransparency(bool val) { SetValueLong(_ignoreTransparencyTag, "if transparence is ignored", (long)val); }

    //
    // Get and set opacity value
    //
    /*
  double GetOpacity() const
  {
    return GetValueDouble( _opacityTag, 1.0 );
  }
  void  SetOpacity( double val )
  {
    SetValueDouble( _opacityTag, "set opacity value", val );
  }
  */

    //
    // Get and set orientation
    //
    int  GetOrientation() const { return GetValueLong(_orientationTag, 2); }
    void SetOrientation(int val) { SetValueLong(_orientationTag, "set orientation value", val); }

    //! Get the TMS level of detail
    //! \retval int Currently selected TMS level of detail.  Value of -1 means we are computing a default value.
    int GetTMSLOD() const
    {
        int value = ((int)GetValueLong(_TMSLODTag, -1));
        return value;
    }

    //! Set the current TMS level of detail
    //! \param[in] val int val, the TMS level of detail to be applied.  Value of 0 means we are computing a default value.
    void SetTMSLOD(int val) { SetValueLong(_TMSLODTag, "TMS level of detail", (long)val); }

    //! Get the number of available levels of detail in the currently selected file.
    //! \retval int The currently available levels of detail in the current file.
    int GetNumTMSLODs() const
    {
        int value = ((int)GetValueLong(_numTMSLODTag, 4));
        return value;
    }

    //! \copydoc RenderParams::GetActualColorMapVariableName()
    virtual string GetActualColorMapVariableName() const override { return ""; }

public:
    static const std::string _fileNameTag;
    static const std::string _isGeoRefTag;
    static const std::string _ignoreTransparencyTag;
    static const std::string _opacityTag;
    static const std::string _TMSLODTag;
    static const std::string _numTMSLODTag;
    static const std::string _orientationTag;    // If it's X-Y (orientation = 2)
                                                 // If it's X-Z (orientation = 1)
                                                 // If it's Y-Z (orientation = 0)

private:
    bool _initialized = false;

    //! Set the number of TMS levels of detail for the currently selected image
    //! \param[in] val int val, the number of TMS levels of detail available in the current file.
    void _setNumTMSLODs(int val) { SetValueLong(_numTMSLODTag, "Number of TMS levels of detail", (long)val); }
};
}    // namespace VAPoR

#endif
