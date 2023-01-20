.. _vapor.dataset.RenderParams:


vapor.dataset.RenderParams
--------------------------


Help on class RenderParams in vapor.dataset:

vapor.dataset.RenderParams = class RenderParams(ParamsBase)
 |  Method resolution order:
 |      RenderParams
 |      ParamsBase
 |      cppyy.gbl.Wasp.MyBase
 |      cppyy.gbl.CPPInstance
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetActualColorMapVariableName(...)
 |      std::string VAPoR::RenderParams::GetActualColorMapVariableName()
 |  
 |  GetAuxVariableNames(...)
 |      std::vector<std::string> VAPoR::RenderParams::GetAuxVariableNames()
 |  
 |  GetBox(...)
 |      VAPoR::Box* VAPoR::RenderParams::GetBox()
 |  
 |  GetColorMapVariableName(...)
 |      std::string VAPoR::RenderParams::GetColorMapVariableName()
 |  
 |  GetColorbarPbase(...)
 |      VAPoR::ColorbarPbase* VAPoR::RenderParams::GetColorbarPbase()
 |  
 |  GetCompressionLevel(...)
 |      int VAPoR::RenderParams::GetCompressionLevel()
 |  
 |  GetConstantColor(...)
 |      void VAPoR::RenderParams::GetConstantColor(float[3] rgb)
 |      std::vector<float> VAPoR::RenderParams::GetConstantColor()
 |  
 |  GetConstantOpacity(...)
 |      float VAPoR::RenderParams::GetConstantOpacity()
 |  
 |  GetCurrentTimestep(...)
 |      unsigned long VAPoR::RenderParams::GetCurrentTimestep()
 |  
 |  GetCursorCoords(...)
 |      void VAPoR::RenderParams::GetCursorCoords(float[2] coords)
 |  
 |  GetDistribVariableNames(...)
 |      std::vector<std::string> VAPoR::RenderParams::GetDistribVariableNames()
 |  
 |  GetFieldVariableNames(...)
 |      std::vector<std::string> VAPoR::RenderParams::GetFieldVariableNames()
 |  
 |  GetFirstVariableName(...)
 |      std::string VAPoR::RenderParams::GetFirstVariableName()
 |  
 |  GetHeightVariableName(...)
 |      std::string VAPoR::RenderParams::GetHeightVariableName()
 |  
 |  GetHistoStretch(...)
 |      float VAPoR::RenderParams::GetHistoStretch()
 |  
 |  GetIsoValues(...)
 |      std::vector<double> VAPoR::RenderParams::GetIsoValues(const std::string& variable)
 |      std::vector<double> VAPoR::RenderParams::GetIsoValues()
 |  
 |  GetMapperFunc(...)
 |      VAPoR::MapperFunction* VAPoR::RenderParams::GetMapperFunc(std::string varname)
 |  
 |  GetOrientable(...)
 |      bool VAPoR::RenderParams::GetOrientable()
 |  
 |  GetRefinementLevel(...)
 |      int VAPoR::RenderParams::GetRefinementLevel()
 |  
 |  GetRenderDim(...)
 |      unsigned long VAPoR::RenderParams::GetRenderDim()
 |  
 |  GetSlicePlaneNormal(...)
 |      std::vector<double> VAPoR::RenderParams::GetSlicePlaneNormal()
 |  
 |  GetSlicePlaneOrigin(...)
 |      std::vector<double> VAPoR::RenderParams::GetSlicePlaneOrigin()
 |  
 |  GetSlicePlaneQuad(...)
 |      std::vector<std::array<double,3> > VAPoR::RenderParams::GetSlicePlaneQuad()
 |  
 |  GetSlicePlaneRotation(...)
 |      std::vector<double> VAPoR::RenderParams::GetSlicePlaneRotation()
 |  
 |  GetTransform(...)
 |      VAPoR::Transform* VAPoR::RenderParams::GetTransform()
 |  
 |  GetVariableName(...)
 |      std::string VAPoR::RenderParams::GetVariableName()
 |  
 |  GetXFieldVariableName(...)
 |      std::string VAPoR::RenderParams::GetXFieldVariableName()
 |  
 |  GetXSlicePlaneOrigin(...)
 |      double VAPoR::RenderParams::GetXSlicePlaneOrigin()
 |  
 |  GetYFieldVariableName(...)
 |      std::string VAPoR::RenderParams::GetYFieldVariableName()
 |  
 |  GetYSlicePlaneOrigin(...)
 |      double VAPoR::RenderParams::GetYSlicePlaneOrigin()
 |  
 |  GetZFieldVariableName(...)
 |      std::string VAPoR::RenderParams::GetZFieldVariableName()
 |  
 |  GetZSlicePlaneOrigin(...)
 |      double VAPoR::RenderParams::GetZSlicePlaneOrigin()
 |  
 |  HasIsoValues(...)
 |      bool VAPoR::RenderParams::HasIsoValues()
 |  
 |  Initialize(...)
 |      int VAPoR::RenderParams::Initialize()
 |  
 |  IsEnabled(...)
 |      bool VAPoR::RenderParams::IsEnabled()
 |  
 |  RemoveMapperFunc(...)
 |      void VAPoR::RenderParams::RemoveMapperFunc(std::string varname)
 |  
 |  ResetUserExtentsToDataExents(...)
 |      int VAPoR::RenderParams::ResetUserExtentsToDataExents(std::string var = "")
 |  
 |  SetAuxVariableNames(...)
 |      void VAPoR::RenderParams::SetAuxVariableNames(std::vector<std::string> varName)
 |  
 |  SetColorMapVariableName(...)
 |      void VAPoR::RenderParams::SetColorMapVariableName(std::string varname)
 |  
 |  SetColorbarPbase(...)
 |      void VAPoR::RenderParams::SetColorbarPbase(VAPoR::ColorbarPbase* pb)
 |  
 |  SetCompressionLevel(...)
 |      void VAPoR::RenderParams::SetCompressionLevel(int val)
 |  
 |  SetConstantColor(...)
 |      void VAPoR::RenderParams::SetConstantColor(const float[3] rgb)
 |      void VAPoR::RenderParams::SetConstantColor(std::vector<float> rgb)
 |  
 |  SetConstantOpacity(...)
 |      void VAPoR::RenderParams::SetConstantOpacity(float o)
 |  
 |  SetCurrentTimestep(...)
 |      void VAPoR::RenderParams::SetCurrentTimestep(size_t ts)
 |  
 |  SetCursorCoords(...)
 |      void VAPoR::RenderParams::SetCursorCoords(const float[2] coords)
 |  
 |  SetDefaultVariables(...)
 |      void VAPoR::RenderParams::SetDefaultVariables(int dim, bool secondaryColormapVariable)
 |  
 |  SetEnabled(...)
 |      void VAPoR::RenderParams::SetEnabled(bool val)
 |  
 |  SetFieldVariableNames(...)
 |      void VAPoR::RenderParams::SetFieldVariableNames(std::vector<std::string> varNames)
 |  
 |  SetHeightVariableName(...)
 |      void VAPoR::RenderParams::SetHeightVariableName(std::string varname)
 |  
 |  SetHistoStretch(...)
 |      void VAPoR::RenderParams::SetHistoStretch(float factor)
 |  
 |  SetIsoValues(...)
 |      void VAPoR::RenderParams::SetIsoValues(const std::string& variable, const std::vector<double>& values)
 |      void VAPoR::RenderParams::SetIsoValues(const std::vector<double>& values)
 |  
 |  SetMapperFunc(...)
 |      void VAPoR::RenderParams::SetMapperFunc(std::string varname, VAPoR::MapperFunction* tf)
 |  
 |  SetRefinementLevel(...)
 |      void VAPoR::RenderParams::SetRefinementLevel(int numrefinements)
 |  
 |  SetSlicePlaneQuad(...)
 |      void VAPoR::RenderParams::SetSlicePlaneQuad(const std::vector<VAPoR::CoordType>& quad)
 |  
 |  SetUseSingleColor(...)
 |      void VAPoR::RenderParams::SetUseSingleColor(bool val)
 |  
 |  SetVariableName(...)
 |      void VAPoR::RenderParams::SetVariableName(std::string varName)
 |  
 |  SetXFieldVariableName(...)
 |      void VAPoR::RenderParams::SetXFieldVariableName(std::string varName)
 |  
 |  SetXSlicePlaneOrigin(...)
 |      void VAPoR::RenderParams::SetXSlicePlaneOrigin(double xOrigin)
 |  
 |  SetYFieldVariableName(...)
 |      void VAPoR::RenderParams::SetYFieldVariableName(std::string varName)
 |  
 |  SetYSlicePlaneOrigin(...)
 |      void VAPoR::RenderParams::SetYSlicePlaneOrigin(double yOrigin)
 |  
 |  SetZFieldVariableName(...)
 |      void VAPoR::RenderParams::SetZFieldVariableName(std::string varName)
 |  
 |  SetZSlicePlaneOrigin(...)
 |      void VAPoR::RenderParams::SetZSlicePlaneOrigin(double zOrigin)
 |  
 |  UseAuxVariable(...)
 |      bool VAPoR::RenderParams::UseAuxVariable()
 |  
 |  UseSingleColor(...)
 |      bool VAPoR::RenderParams::UseSingleColor()
 |  
 |  __assign__(...)
 |      VAPoR::RenderParams& VAPoR::RenderParams::operator=(const VAPoR::RenderParams& rhs)
 |  
 |  __init__(...)
 |      RenderParams::RenderParams(VAPoR::DataMgr* dataMgr, VAPoR::ParamsBase::StateSave* ssave, const std::string& classname, int maxdim = 3)
 |      RenderParams::RenderParams(VAPoR::DataMgr* dataMgr, VAPoR::ParamsBase::StateSave* ssave, VAPoR::XmlNode* node, int maxdim = 3)
 |      RenderParams::RenderParams(const VAPoR::RenderParams& rhs)
 |  
 |  initializeBypassFlags(...)
 |      void VAPoR::RenderParams::initializeBypassFlags()
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes defined here:
 |  
 |  CustomHistogramDataTag = b'CustomHistogramData'
 |  
 |  CustomHistogramRangeTag = b'CustomHistogramRange'
 |  
 |  LightingEnabledTag = b'LightingEnabled'
 |  
 |  SampleRateTag = b'SampleRate'
 |  
 |  SliceOffsetTag = b'SliceOffsetTag'
 |  
 |  SlicePlaneNormalXTag = b'SlicePlaneNormalXTag'
 |  
 |  SlicePlaneNormalYTag = b'SlicePlaneNormalYTag'
 |  
 |  SlicePlaneNormalZTag = b'SlicePlaneNormalZTag'
 |  
 |  SlicePlaneOrientationMode = <class 'vapor.renderer.SlicePlaneOrientati...
 |  
 |  SlicePlaneOrientationModeTag = b'SlicePlaneOrientationModeTag'
 |  
 |  XSlicePlaneOriginTag = b'XSlicePlaneOrigin'
 |  
 |  XSlicePlaneRotationTag = b'XSlicePlaneRotation'
 |  
 |  YSlicePlaneOriginTag = b'YSlicePlaneOrigin'
 |  
 |  YSlicePlaneRotationTag = b'YSlicePlaneRotation'
 |  
 |  ZSlicePlaneOriginTag = b'ZSlicePlaneOrigin'
 |  
 |  ZSlicePlaneRotationTag = b'ZSlicePlaneRotation'
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from ParamsBase:
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
 |  __eq__(self, value, /)
 |      Return self==value.
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

