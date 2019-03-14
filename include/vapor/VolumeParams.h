#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API VolumeParams : public RenderParams {
public:

 VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
 VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType);
 VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
 virtual ~VolumeParams();

 virtual bool IsOpaque() const;

 virtual bool usingVariable(const std::string& varname);
 virtual string GetDefaultAlgorithmName() const;

  static string GetClassType() 
  {
	  return("VolumeParams");
  }
    
    std::string GetAlgorithm() const;
    void SetAlgorithm(std::string algorithm);
    double GetIsoValue() const;
    void SetIsoValue(double isoValue);
    
    void SetIsoValues(std::vector<double> values);
    std::vector<double> GetIsoValues() const;
    void SetEnabledIsoValues(std::vector<bool> mask);
    std::vector<bool> GetEnabledIsoValues() const;
    
    static const std::vector<std::string> GetAlgorithmNames();
    static void Register(const std::string &name);

private:

 void _init();
    
    static std::vector<std::string> _algorithmNames;
    
    static const std::string _algorithmTag;
    static const std::string _isoValueTag;
    static const std::string _isoValuesTag;
    static const std::string _enabledIsoValuesTag;


};
    
};
