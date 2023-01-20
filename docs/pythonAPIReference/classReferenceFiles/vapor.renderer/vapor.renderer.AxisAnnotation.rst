.. _vapor.renderer.AxisAnnotation:


vapor.renderer.AxisAnnotation
-----------------------------


Help on class AxisAnnotation in vapor.renderer:

vapor.renderer.AxisAnnotation = class AxisAnnotation(vapor.params.ParamsWrapper)
 |  vapor.renderer.AxisAnnotation(p: cppyy.gbl.VAPoR.ParamsBase)
 |  
 |  Wraps VAPoR::AxisAnnotation
 |  class that indicates location and direction of view
 |  
 |  Method resolution order:
 |      AxisAnnotation
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetAxisAnnotationEnabled(...)
 |      bool VAPoR::AxisAnnotation::GetAxisAnnotationEnabled()
 |  
 |  GetAxisBackgroundColor(...)
 |      vector<double> VAPoR::AxisAnnotation::GetAxisBackgroundColor()
 |      
 |      void VAPoR::AxisAnnotation::GetAxisBackgroundColor(float bgColor[])
 |  
 |  GetAxisColor(...)
 |      vector<double> VAPoR::AxisAnnotation::GetAxisColor()
 |  
 |  GetAxisDigits(...)
 |      long VAPoR::AxisAnnotation::GetAxisDigits()
 |  
 |  GetAxisFontSize(...)
 |      int VAPoR::AxisAnnotation::GetAxisFontSize()
 |  
 |  GetAxisOrigin(...)
 |      vector<double> VAPoR::AxisAnnotation::GetAxisOrigin()
 |  
 |  GetAxisTextHeight(...)
 |      long VAPoR::AxisAnnotation::GetAxisTextHeight()
 |  
 |  GetLatLonAxesEnabled(...)
 |      bool VAPoR::AxisAnnotation::GetLatLonAxesEnabled()
 |  
 |  GetMaxTics(...)
 |      vector<double> VAPoR::AxisAnnotation::GetMaxTics()
 |  
 |  GetMinTics(...)
 |      vector<double> VAPoR::AxisAnnotation::GetMinTics()
 |  
 |  GetNumTics(...)
 |      vector<double> VAPoR::AxisAnnotation::GetNumTics()
 |  
 |  GetShowAxisArrows(...)
 |      bool VAPoR::AxisAnnotation::GetShowAxisArrows()
 |  
 |  GetTicDirs(...)
 |      vector<double> VAPoR::AxisAnnotation::GetTicDirs()
 |  
 |  GetTicSize(...)
 |      vector<double> VAPoR::AxisAnnotation::GetTicSize()
 |  
 |  GetTicWidth(...)
 |      double VAPoR::AxisAnnotation::GetTicWidth()
 |  
 |  GetXTicDir(...)
 |      int VAPoR::AxisAnnotation::GetXTicDir()
 |  
 |  GetYTicDir(...)
 |      int VAPoR::AxisAnnotation::GetYTicDir()
 |  
 |  GetZTicDir(...)
 |      int VAPoR::AxisAnnotation::GetZTicDir()
 |  
 |  SetAxisAnnotationEnabled(...)
 |      void VAPoR::AxisAnnotation::SetAxisAnnotationEnabled(bool val)
 |  
 |  SetAxisBackgroundColor(...)
 |      void VAPoR::AxisAnnotation::SetAxisBackgroundColor(vector< double > color)
 |  
 |  SetAxisColor(...)
 |      void VAPoR::AxisAnnotation::SetAxisColor(vector< double > color)
 |  
 |  SetAxisDigits(...)
 |      void VAPoR::AxisAnnotation::SetAxisDigits(long val)
 |  
 |  SetAxisFontSize(...)
 |      void VAPoR::AxisAnnotation::SetAxisFontSize(int size)
 |  
 |  SetAxisOrigin(...)
 |      void VAPoR::AxisAnnotation::SetAxisOrigin(vector< double > orig)
 |  
 |  SetAxisTextHeight(...)
 |      void VAPoR::AxisAnnotation::SetAxisTextHeight(long val)
 |  
 |  SetLatLonAxesEnabled(...)
 |      void VAPoR::AxisAnnotation::SetLatLonAxesEnabled(bool val)
 |  
 |  SetMaxTics(...)
 |      void VAPoR::AxisAnnotation::SetMaxTics(vector< double > ticmaxs)
 |  
 |  SetMinTics(...)
 |      void VAPoR::AxisAnnotation::SetMinTics(vector< double > ticmins)
 |  
 |  SetNumTics(...)
 |      void VAPoR::AxisAnnotation::SetNumTics(vector< double > ticnums)
 |  
 |  SetShowAxisArrows(...)
 |      void VAPoR::AxisAnnotation::SetShowAxisArrows(bool val)
 |  
 |  SetTicDirs(...)
 |      void VAPoR::AxisAnnotation::SetTicDirs(vector< double > ticdirs)
 |  
 |  SetTicSize(...)
 |      void VAPoR::AxisAnnotation::SetTicSize(vector< double > ticsizes)
 |  
 |  SetTicWidth(...)
 |      void VAPoR::AxisAnnotation::SetTicWidth(double val)
 |  
 |  SetXTicDir(...)
 |      void VAPoR::AxisAnnotation::SetXTicDir(double dir)
 |  
 |  SetYTicDir(...)
 |      void VAPoR::AxisAnnotation::SetYTicDir(double dir)
 |  
 |  SetZTicDir(...)
 |      void VAPoR::AxisAnnotation::SetZTicDir(double dir)
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

