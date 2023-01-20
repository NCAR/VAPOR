.. _vapor.animation.NavigationUtils:


vapor.animation.NavigationUtils
-------------------------------


Help on class NavigationUtils in vapor.animation:

vapor.animation.NavigationUtils = class NavigationUtils(cppyy.gbl.Wasp.MyBase)
 |  Method resolution order:
 |      NavigationUtils
 |      cppyy.gbl.Wasp.MyBase
 |      CPPInstance
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  AlignView(...)
 |      static void NavigationUtils::AlignView(VAPoR::ControlExec* ce, int axis)
 |  
 |  ConfigureTrackball(...)
 |      static void NavigationUtils::ConfigureTrackball(VAPoR::ControlExec* ce, Trackball& trackball)
 |  
 |  GetActiveViewpointParams(...)
 |      static VAPoR::ViewpointParams* NavigationUtils::GetActiveViewpointParams(VAPoR::ControlExec* ce)
 |  
 |  GetAnimationParams(...)
 |      static AnimationParams* NavigationUtils::GetAnimationParams(VAPoR::ControlExec* ce)
 |  
 |  GetCameraDirection(...)
 |      static std::vector<double> NavigationUtils::GetCameraDirection(VAPoR::ControlExec* ce)
 |  
 |  GetCameraPosition(...)
 |      static std::vector<double> NavigationUtils::GetCameraPosition(VAPoR::ControlExec* ce)
 |  
 |  GetCameraProperties(...)
 |      static void NavigationUtils::GetCameraProperties(VAPoR::ControlExec* ce, std::vector<double>* position, std::vector<double>* direction, std::vector<double>* up, std::vector<double>* target)
 |  
 |  GetCameraTarget(...)
 |      static std::vector<double> NavigationUtils::GetCameraTarget(VAPoR::ControlExec* ce)
 |  
 |  GetCameraUp(...)
 |      static std::vector<double> NavigationUtils::GetCameraUp(VAPoR::ControlExec* ce)
 |  
 |  GetCurrentTimeStep(...)
 |      static long NavigationUtils::GetCurrentTimeStep(VAPoR::ControlExec* ce)
 |  
 |  GetGUIStateParams(...)
 |      static GUIStateParams* NavigationUtils::GetGUIStateParams(VAPoR::ControlExec* ce)
 |  
 |  LookAt(...)
 |      static void NavigationUtils::LookAt(VAPoR::ControlExec* ce, const std::vector<double>& position, const std::vector<double>& target, const std::vector<double>& up)
 |  
 |  SetAllCameras(...)
 |      static void NavigationUtils::SetAllCameras(VAPoR::ControlExec* ce, const double[3] position, const double[3] direction, const double[3] up, const double[3] origin)
 |      static void NavigationUtils::SetAllCameras(VAPoR::ControlExec* ce, const double[3] position, const double[3] direction, const double[3] up)
 |      static void NavigationUtils::SetAllCameras(VAPoR::ControlExec* ce, const std::vector<double>& position, const std::vector<double>& direction, const std::vector<double>& up, const std::vector<double>& origin)
 |      static void NavigationUtils::SetAllCameras(VAPoR::ControlExec* ce, const std::vector<double>& position, const std::vector<double>& direction, const std::vector<double>& up)
 |      static void NavigationUtils::SetAllCameras(VAPoR::ControlExec* ce, const double[16] matrix, const double[3] origin)
 |      static void NavigationUtils::SetAllCameras(VAPoR::ControlExec* ce, const std::vector<double>& matrix, const std::vector<double>& origin)
 |      static void NavigationUtils::SetAllCameras(VAPoR::ControlExec* ce, const Trackball& trackball)
 |  
 |  SetCameraDirection(...)
 |      static void NavigationUtils::SetCameraDirection(VAPoR::ControlExec* ce, const std::vector<double>& v)
 |  
 |  SetCameraPosition(...)
 |      static void NavigationUtils::SetCameraPosition(VAPoR::ControlExec* ce, const std::vector<double>& v)
 |  
 |  SetCameraTarget(...)
 |      static void NavigationUtils::SetCameraTarget(VAPoR::ControlExec* ce, const std::vector<double>& v)
 |  
 |  SetCameraUp(...)
 |      static void NavigationUtils::SetCameraUp(VAPoR::ControlExec* ce, const std::vector<double>& v)
 |  
 |  SetHomeViewpoint(...)
 |      static void NavigationUtils::SetHomeViewpoint(VAPoR::ControlExec* ce)
 |  
 |  SetTimestep(...)
 |      static void NavigationUtils::SetTimestep(VAPoR::ControlExec* ce, size_t ts)
 |  
 |  UseHomeViewpoint(...)
 |      static void NavigationUtils::UseHomeViewpoint(VAPoR::ControlExec* ce)
 |  
 |  ViewAll(...)
 |      static void NavigationUtils::ViewAll(VAPoR::ControlExec* ce)
 |  
 |  __assign__(...)
 |      NavigationUtils& NavigationUtils::operator=(const NavigationUtils&)
 |      NavigationUtils& NavigationUtils::operator=(NavigationUtils&&)
 |  
 |  __init__(...)
 |      NavigationUtils::NavigationUtils()
 |      NavigationUtils::NavigationUtils(const NavigationUtils&)
 |      NavigationUtils::NavigationUtils(NavigationUtils&&)
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from cppyy.gbl.Wasp.MyBase:
 |  
 |  EnableErrMsg(...)
 |      static bool Wasp::MyBase::EnableErrMsg(bool enable)
 |  
 |  GetDiagMsg(...)
 |      static const char* Wasp::MyBase::GetDiagMsg()
 |  
 |  GetDiagMsgCB(...)
 |      static void(*)(const char*) Wasp::MyBase::GetDiagMsgCB()
 |  
 |  GetEnableErrMsg(...)
 |      static bool Wasp::MyBase::GetEnableErrMsg()
 |  
 |  GetErrCode(...)
 |      static int Wasp::MyBase::GetErrCode()
 |  
 |  GetErrMsg(...)
 |      static const char* Wasp::MyBase::GetErrMsg()
 |  
 |  GetErrMsgCB(...)
 |      static void(*)(const char*,int) Wasp::MyBase::GetErrMsgCB()
 |  
 |  SetDiagMsg(...)
 |      static void Wasp::MyBase::SetDiagMsg(const char* format)
 |  
 |  SetDiagMsgCB(...)
 |      static void Wasp::MyBase::SetDiagMsgCB(Wasp::MyBase::DiagMsgCB_T cb)
 |  
 |  SetDiagMsgFilePtr(...)
 |      static void Wasp::MyBase::SetDiagMsgFilePtr(FILE* fp)
 |  
 |  SetErrCode(...)
 |      static void Wasp::MyBase::SetErrCode(int err_code)
 |  
 |  SetErrMsg(...)
 |      static void Wasp::MyBase::SetErrMsg(const char* format)
 |      static void Wasp::MyBase::SetErrMsg(int errcode, const char* format)
 |  
 |  SetErrMsgCB(...)
 |      static void Wasp::MyBase::SetErrMsgCB(Wasp::MyBase::ErrMsgCB_T cb)
 |  
 |  SetErrMsgFilePtr(...)
 |      static void Wasp::MyBase::SetErrMsgFilePtr(FILE* fp)
 |      static const __sFILE* Wasp::MyBase::SetErrMsgFilePtr()
 |  
 |  getClassName(...)
 |      const std::string& Wasp::MyBase::getClassName()
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from cppyy.gbl.Wasp.MyBase:
 |  
 |  DiagMsgCB
 |  
 |  ErrMsgCB
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes inherited from cppyy.gbl.Wasp.MyBase:
 |  
 |  DiagMsg = ''
 |  
 |  DiagMsgFilePtr = nullptr
 |  
 |  DiagMsgSize = 0
 |  
 |  Enabled = True
 |  
 |  ErrCode = 0
 |  
 |  ErrMsg = ''
 |  
 |  ErrMsgFilePtr = <cppyy.LowLevelView object>
 |  
 |  ErrMsgSize = 0
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from CPPInstance:
 |  
 |  __add__(self, value, /)
 |      Return self+value.
 |  
 |  __bool__(self, /)
 |      True if self else False
 |  
 |  __destruct__(...)
 |      call the C++ destructor
 |  
 |  __dispatch__(...)
 |      dispatch to selected overload
 |  
 |  __eq__(self, value, /)
 |      Return self==value.
 |  
 |  __ge__(self, value, /)
 |      Return self>=value.
 |  
 |  __getitem__(...)
 |      pointer dereferencing
 |  
 |  __gt__(self, value, /)
 |      Return self>value.
 |  
 |  __hash__(self, /)
 |      Return hash(self).
 |  
 |  __invert__(self, /)
 |      ~self
 |  
 |  __le__(self, value, /)
 |      Return self<=value.
 |  
 |  __lt__(self, value, /)
 |      Return self<value.
 |  
 |  __mul__(self, value, /)
 |      Return self*value.
 |  
 |  __ne__(self, value, /)
 |      Return self!=value.
 |  
 |  __neg__(self, /)
 |      -self
 |  
 |  __pos__(self, /)
 |      +self
 |  
 |  __radd__(self, value, /)
 |      Return value+self.
 |  
 |  __repr__(self, /)
 |      Return repr(self).
 |  
 |  __rmul__(self, value, /)
 |      Return value*self.
 |  
 |  __rsub__(self, value, /)
 |      Return value-self.
 |  
 |  __rtruediv__(self, value, /)
 |      Return value/self.
 |  
 |  __smartptr__(...)
 |      get associated smart pointer, if any
 |  
 |  __str__(self, /)
 |      Return str(self).
 |  
 |  __sub__(self, value, /)
 |      Return self-value.
 |  
 |  __truediv__(self, value, /)
 |      Return self/value.
 |  
 |  ----------------------------------------------------------------------
 |  Static methods inherited from CPPInstance:
 |  
 |  __new__(*args, **kwargs) from cppyy.CPPScope
 |      Create and return a new object.  See help(type) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from CPPInstance:
 |  
 |  __python_owns__
 |      If true, python manages the life time of this object

