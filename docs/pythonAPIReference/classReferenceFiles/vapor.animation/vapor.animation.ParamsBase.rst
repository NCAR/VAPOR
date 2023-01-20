.. _vapor.animation.ParamsBase:


vapor.animation.ParamsBase
--------------------------


Help on class ParamsBase in vapor.animation:

vapor.animation.ParamsBase = class ParamsBase(cppyy.gbl.Wasp.MyBase)
 |  Method resolution order:
 |      ParamsBase
 |      cppyy.gbl.Wasp.MyBase
 |      cppyy.gbl.CPPInstance
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  BeginGroup(...)
 |      void VAPoR::ParamsBase::BeginGroup(const std::string& description)
 |  
 |  EndGroup(...)
 |      void VAPoR::ParamsBase::EndGroup()
 |  
 |  GetName(...)
 |      std::string VAPoR::ParamsBase::GetName()
 |  
 |  GetNode(...)
 |      VAPoR::XmlNode* VAPoR::ParamsBase::GetNode()
 |  
 |  GetValueDouble(...)
 |      double VAPoR::ParamsBase::GetValueDouble(const std::string tag, double defaultVal)
 |  
 |  GetValueDoubleVec(...)
 |      std::vector<double> VAPoR::ParamsBase::GetValueDoubleVec(const std::string tag)
 |      std::vector<double> VAPoR::ParamsBase::GetValueDoubleVec(const std::string tag, const std::vector<double>& defaultVal)
 |  
 |  GetValueLong(...)
 |      long VAPoR::ParamsBase::GetValueLong(const std::string tag, long defaultVal)
 |  
 |  GetValueLongVec(...)
 |      std::vector<long> VAPoR::ParamsBase::GetValueLongVec(const std::string tag)
 |      std::vector<long> VAPoR::ParamsBase::GetValueLongVec(const std::string tag, const std::vector<long>& defaultVal)
 |  
 |  GetValueString(...)
 |      std::string VAPoR::ParamsBase::GetValueString(const std::string tag, std::string defaultVal)
 |  
 |  GetValueStringVec(...)
 |      std::vector<std::string> VAPoR::ParamsBase::GetValueStringVec(const std::string tag)
 |      std::vector<std::string> VAPoR::ParamsBase::GetValueStringVec(const std::string tag, const std::vector<std::string>& defaultVal)
 |  
 |  IntermediateChange(...)
 |      void VAPoR::ParamsBase::IntermediateChange()
 |  
 |  SetParent(...)
 |      void VAPoR::ParamsBase::SetParent(VAPoR::ParamsBase* parent)
 |  
 |  SetValueDouble(...)
 |      void VAPoR::ParamsBase::SetValueDouble(const std::string& tag, std::string description, double value)
 |  
 |  SetValueDoubleVec(...)
 |      void VAPoR::ParamsBase::SetValueDoubleVec(const std::string& tag, std::string description, const std::vector<double>& values)
 |  
 |  SetValueLong(...)
 |      void VAPoR::ParamsBase::SetValueLong(const std::string& tag, std::string description, long value)
 |  
 |  SetValueLongVec(...)
 |      void VAPoR::ParamsBase::SetValueLongVec(const std::string& tag, std::string description, const std::vector<long>& values)
 |  
 |  SetValueString(...)
 |      void VAPoR::ParamsBase::SetValueString(const std::string& tag, std::string description, const std::string& value)
 |  
 |  SetValueStringVec(...)
 |      void VAPoR::ParamsBase::SetValueStringVec(const std::string& tag, std::string description, const std::vector<std::string>& values)
 |  
 |  __assign__(...)
 |      VAPoR::ParamsBase& VAPoR::ParamsBase::operator=(const VAPoR::ParamsBase& rhs)
 |  
 |  __eq__(self, value, /)
 |      Return self==value.
 |  
 |  __init__(...)
 |      ParamsBase::ParamsBase(VAPoR::ParamsBase::StateSave* ssave, const std::string& classname)
 |      ParamsBase::ParamsBase(VAPoR::ParamsBase::StateSave* ssave, VAPoR::XmlNode* node)
 |      ParamsBase::ParamsBase(const VAPoR::ParamsBase& rhs)
 |  
 |  __ne__(self, value, /)
 |      Return self!=value.
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
 |  Methods inherited from cppyy.gbl.CPPInstance:
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
 |  Static methods inherited from cppyy.gbl.CPPInstance:
 |  
 |  __new__(*args, **kwargs) from cppyy.CPPScope
 |      Create and return a new object.  See help(type) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from cppyy.gbl.CPPInstance:
 |  
 |  __python_owns__
 |      If true, python manages the life time of this object

