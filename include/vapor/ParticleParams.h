#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API ParticleParams : public RenderParams {
public:
 ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
 ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType);
 ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
 virtual ~ParticleParams();

  static string GetClassType() 
  {
	  return("ParticleParams");
  }

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual  size_t GetRenderDim() const override {
        return(3);
    }
    
private:

    void _init();
    
public:
    static const std::string ShowDirectionTag;
    static const std::string DirectionScaleTag;
    static const std::string StrideTag;
};
    
};
