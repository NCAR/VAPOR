
#ifndef IMAGEPARAMS_H
#define IMAGEPARAMS_H

#include <vapor/ParamNode.h>
#include <vapor/params.h>
#include <vapor/renderparams.h>
#include <vapor/command.h>

namespace VAPoR 
{
//! \class ImageParams
//! \brief Class that supports drawing Barbs based on 2D or 3D vector field
class PARAMS_API ImageParams : public RenderParams 
{
public:

  ImageParams( DataMgr* dataManager, ParamsBase::StateSave* stateSave );
  ImageParams( DataMgr* dataManager, ParamsBase::StateSave* stateSave, XmlNode* xmlNode );

 //! Required static method for extensibility:
 //! \retval ParamsBase* pointer to a default Params instance
 /* static ParamsBase* CreateDefaultInstance() 
 {
	return new ImageParams(0, -1);
 } */

 virtual ~ImageParams();

  static std::string getClassType() 
  {
    return ("ImageParams");
  }

  virtual void Validate( int type );

  virtual void IsOpaque() const;

  virtual bool usingVariable( const std::string& varName );

private:
  bool          _hasGeoreference;
  bool          _ignoreTransparence;
  float         _Opacity;
  std::string   _filePath;

  void          _init();
};
}
