.. _vapor.session.SceneAnnotation:


vapor.session.SceneAnnotation
-----------------------------


Help on class SceneAnnotation in vapor.session:

vapor.session.SceneAnnotation = class SceneAnnotation(vapor.params.ParamsWrapper)
 |  vapor.session.SceneAnnotation(p: cppyy.gbl.VAPoR.ParamsBase)
 |  
 |  Wraps VAPoR::AnnotationParams
 |  A class for describing visual features displayed in the visualizer.
 |  The AnnotationParams class controls various features displayed in the visualizers There is a global AnnotationParams , that is shared by all windows whose vizfeature is set to "global". There is also a local AnnotationParams for each window, that users can select whenever there are multiple windows. When local settings are used, they only affect one currently active visualizer. The AnnotationParams class also has several methods that are useful in setting up data requests from the DataMgr .
 |  
 |  Method resolution order:
 |      SceneAnnotation
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetAxisArrowEnabled(...)
 |      bool VAPoR::AnnotationParams::GetAxisArrowEnabled()
 |  
 |  GetAxisArrowSize(...)
 |      double VAPoR::AnnotationParams::GetAxisArrowSize()
 |  
 |  GetAxisArrowXPos(...)
 |      double VAPoR::AnnotationParams::GetAxisArrowXPos()
 |  
 |  GetAxisArrowYPos(...)
 |      double VAPoR::AnnotationParams::GetAxisArrowYPos()
 |  
 |  GetAxisFontSize(...)
 |      int VAPoR::AnnotationParams::GetAxisFontSize()
 |  
 |  GetBackgroundColor(...)
 |      void VAPoR::AnnotationParams::GetBackgroundColor(double color[3])
 |      
 |      void VAPoR::AnnotationParams::GetBackgroundColor(std::vector< double > &color)
 |  
 |  GetCurrentAxisDataMgrName(...)
 |      string VAPoR::AnnotationParams::GetCurrentAxisDataMgrName()
 |  
 |  GetDomainColor(...)
 |      void VAPoR::AnnotationParams::GetDomainColor(double color[3])
 |      
 |      void VAPoR::AnnotationParams::GetDomainColor(std::vector< double > &color)
 |  
 |  GetRegionColor(...)
 |      void VAPoR::AnnotationParams::GetRegionColor(double color[3])
 |      
 |      void VAPoR::AnnotationParams::GetRegionColor(std::vector< double > &color)
 |  
 |  GetTimeLLX(...)
 |      double VAPoR::AnnotationParams::GetTimeLLX()
 |  
 |  GetTimeLLY(...)
 |      double VAPoR::AnnotationParams::GetTimeLLY()
 |  
 |  GetTimeSize(...)
 |      int VAPoR::AnnotationParams::GetTimeSize()
 |  
 |  GetTimeType(...)
 |      int VAPoR::AnnotationParams::GetTimeType()
 |  
 |  GetUseDomainFrame(...)
 |      bool VAPoR::AnnotationParams::GetUseDomainFrame()
 |  
 |  GetUseRegionFrame(...)
 |      bool VAPoR::AnnotationParams::GetUseRegionFrame()
 |  
 |  SetAxisArrowEnabled(...)
 |      void VAPoR::AnnotationParams::SetAxisArrowEnabled(bool enabled)
 |  
 |  SetAxisArrowSize(...)
 |      void VAPoR::AnnotationParams::SetAxisArrowSize(double pos)
 |  
 |  SetAxisArrowXPos(...)
 |      void VAPoR::AnnotationParams::SetAxisArrowXPos(double pos)
 |  
 |  SetAxisArrowYPos(...)
 |      void VAPoR::AnnotationParams::SetAxisArrowYPos(double pos)
 |  
 |  SetAxisFontSize(...)
 |      void VAPoR::AnnotationParams::SetAxisFontSize(int size)
 |  
 |  SetBackgroundColor(...)
 |      void VAPoR::AnnotationParams::SetBackgroundColor(std::vector< double > color)
 |  
 |  SetCurrentAxisDataMgrName(...)
 |      void VAPoR::AnnotationParams::SetCurrentAxisDataMgrName(string dataMgr="default")
 |  
 |  SetDomainColor(...)
 |      void VAPoR::AnnotationParams::SetDomainColor(vector< double > color)
 |  
 |  SetTimeColor(...)
 |      void VAPoR::AnnotationParams::SetTimeColor(std::vector< double > color)
 |  
 |  SetTimeLLX(...)
 |      void VAPoR::AnnotationParams::SetTimeLLX(double llx)
 |  
 |  SetTimeLLY(...)
 |      void VAPoR::AnnotationParams::SetTimeLLY(double lly)
 |  
 |  SetTimeSize(...)
 |      void VAPoR::AnnotationParams::SetTimeSize(int size)
 |  
 |  SetTimeType(...)
 |      void VAPoR::AnnotationParams::SetTimeType(int type)
 |  
 |  SetUseDomainFrame(...)
 |      void VAPoR::AnnotationParams::SetUseDomainFrame(bool onOff)
 |  
 |  SetUseRegionFrame(...)
 |      void VAPoR::AnnotationParams::SetUseRegionFrame(bool onOff)
 |  
 |  f(...)
 |      std::vector<double> VAPoR::AnnotationParams::GetTimeColor()
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes defined here:
 |  
 |  TimeAnnotationType = <class 'vapor.annotations.SceneAnnotation.TimeAnn...
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from vapor.params.ParamsWrapper:
 |  
 |  __init__(self, p: cppyy.gbl.VAPoR.ParamsBase)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Class methods inherited from vapor.smartwrapper.SmartWrapper:
 |  
 |  __subclasses_rec__() from vapor.smartwrapper.SmartWrapperMeta
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from vapor.smartwrapper.SmartWrapper:
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)

