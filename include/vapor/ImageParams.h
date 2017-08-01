#ifndef IMAGEPARAMS_H
#define IMAGEPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR 
{

class PARAMS_API ImageParams : public RenderParams 
{
public:

  ImageParams( DataMgr*                 dataManager, 
               ParamsBase::StateSave*   stateSave );
  ImageParams( DataMgr*                 dataManager, 
               ParamsBase::StateSave*   stateSave, 
               XmlNode*                 xmlNode );

  virtual ~ImageParams();

  static std::string GetClassType() 
  {
    return ("ImageParams");
  }

  //
  // Get and set image file path
  // 
  void SetImagePath( std::string file )
  {
    SetValueString( _fileNameTag, "Set image file path", file );
  }
  std::string GetImagePath( ) const
  {
    return GetValueString( _fileNameTag, "File name not found!" );
  }


  //
  // Get and set ifGeoTIFF
  // 
  bool GetIsGeoTIFF() const
  {
    return (0 != GetValueLong( _isGeoTIFFTag, (long)false ) );
  }
  void SetIsGeoTIFF( bool val )
  {
    SetValueLong( _isGeoTIFFTag, "if the image has geo reference", (long)val );
  }


  //
  // Get and set ignoreTransparency
  // 
  bool GetIgnoreTransparency() const
  {
    return (0 != GetValueLong( _ignoreTransparencyTag, (long)false ) );
  }
  void SetIgnoreTransparency( bool val )
  {
    SetValueLong( _ignoreTransparencyTag, "if transparence is ignored", (long)val );
  }


  //
  // Get and set opacity value
  // 
  double GetOpacity() const
  {
    return GetValueDouble( _opacityTag, 1.0 );
  }
  void  setOpacity( double val ) 
  {
    SetValueDouble( _opacityTag, "set opacity value", val );
  }


  //
  // (Pure virtual methods from RenderParams)
  //
  virtual bool IsOpaque() const override
  { 
    return false; 
  }
  virtual bool usingVariable(const std::string& varname) override
  {
    return false;   // since this class is for an image, not rendering a variable.
  }


private:
  static const std::string          _fileNameTag;
  static const std::string          _isGeoTIFFTag;
  static const std::string          _ignoreTransparencyTag;
  static const std::string          _opacityTag;
};
}

#endif
