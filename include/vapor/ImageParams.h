#ifndef IMAGEPARAMS_H
#define IMAGEPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/GetAppPath.h>

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
    std::vector<std::string> paths;
    paths.push_back("images/NaturalEarth.tms");
    std::string defaultImage = Wasp::GetAppPath("VAPOR", "share", paths);

    return GetValueString( _fileNameTag, defaultImage );
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
  int GetOrientation() const
  {
    return GetValueLong( _orientationTag, 2 );
  }
  void  SetOrientation( int val ) 
  {
    SetValueLong( _orientationTag, "set orientation value", val );
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
  static const std::string          _orientationTag;  // If it's X-Y (orientation = 2) 
                                                      // If it's X-Z (orientation = 1)
                                                      // If it's Y-Z (orientation = 0)
};
}

#endif
