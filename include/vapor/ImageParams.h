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

  virtual int Initialize() override;

  static std::string GetClassType() 
  {
    return ("ImageParams");
  }

 //! \copydoc RenderParams::GetRenderDim()
 //
 virtual  size_t GetRenderDim() const override {
	return(2);
 }

  //
  // Get and set image file path
  // 
  void SetImagePath( std::string file )
  {
    SetValueString( _fileNameTag, "Set image file path", file );
  }

    std::string GetImagePath( ) const;


  //
  // Get and set ifGeoRef
  // 
  bool GetIsGeoRef() const {
	return (GetValueLong(_isGeoRefTag, (long)true));
  }

  void SetIsGeoRef( bool val ) {
    SetValueLong( _isGeoRefTag, "Geo-reference the image", (long)val );
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
  // Get and set the threshold for TMS image downsampling
  // 
  int GetTMSLOD() const
  {
    int value = ( (int)GetValueLong( _TMSLODTag, 1 ) );
    return value;
  }
  void SetTMSLOD( int val )
  {
    SetValueLong( _TMSLODTag, "TMS level of detail", (long)val );
  }

private:
  static const std::string          _fileNameTag;
  static const std::string          _isGeoRefTag;
  static const std::string          _ignoreTransparencyTag;
  static const std::string          _opacityTag;
  static const std::string          _TMSLODTag;
  static const std::string          _orientationTag;  // If it's X-Y (orientation = 2) 
                                                      // If it's X-Z (orientation = 1)
                                                      // If it's Y-Z (orientation = 0)
};
}

#endif
