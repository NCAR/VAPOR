.. _vapor.utils.Transform:


vapor.utils.Transform
---------------------


Help on class Transform in vapor.utils:

vapor.utils.Transform = class Transform(vapor.params.ParamsWrapper)
 |  vapor.utils.Transform(p: cppyy.gbl.VAPoR.ParamsBase)
 |  
 |  Wraps VAPoR::Transform
 |  class that indicates location and direction of view
 |  
 |  Method resolution order:
 |      Transform
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetOrigin(...)
 |      vector<double> VAPoR::Transform::GetOrigin()
 |  
 |  GetRotations(...)
 |      vector<double> VAPoR::Transform::GetRotations()
 |  
 |  GetScales(...)
 |      vector<double> VAPoR::Transform::GetScales()
 |  
 |  GetTranslations(...)
 |      vector<double> VAPoR::Transform::GetTranslations()
 |  
 |  SetOrigin(...)
 |      void VAPoR::Transform::SetOrigin(const vector< double > origin)
 |  
 |  SetRotations(...)
 |      void VAPoR::Transform::SetRotations(const vector< double > rotation)
 |  
 |  SetScales(...)
 |      void VAPoR::Transform::SetScales(const vector< double > scale)
 |  
 |  SetTranslations(...)
 |      void VAPoR::Transform::SetTranslations(const vector< double > translation)
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

