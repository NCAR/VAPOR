.. _vapor.animation.DC:


vapor.animation.DC
------------------


Help on class DC in vapor.animation:

vapor.animation.DC = class DC(cppyy.gbl.Wasp.MyBase)
 |  Method resolution order:
 |      DC
 |      cppyy.gbl.Wasp.MyBase
 |      cppyy.gbl.CPPInstance
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  CloseVariable(...)
 |      int VAPoR::DC::CloseVariable(int fd)
 |  
 |  GetAtt(...)
 |      bool VAPoR::DC::GetAtt(std::string varname, std::string attname, std::vector<double>& values)
 |      bool VAPoR::DC::GetAtt(std::string varname, std::string attname, std::vector<long>& values)
 |      bool VAPoR::DC::GetAtt(std::string varname, std::string attname, std::string& values)
 |  
 |  GetAttNames(...)
 |      std::vector<std::string> VAPoR::DC::GetAttNames(std::string varname)
 |  
 |  GetAttType(...)
 |      VAPoR::DC::XType VAPoR::DC::GetAttType(std::string varname, std::string attname)
 |  
 |  GetAuxVarInfo(...)
 |      bool VAPoR::DC::GetAuxVarInfo(std::string varname, VAPoR::DC::AuxVar& var)
 |  
 |  GetAuxVarNames(...)
 |      std::vector<std::string> VAPoR::DC::GetAuxVarNames()
 |  
 |  GetBaseVarInfo(...)
 |      bool VAPoR::DC::GetBaseVarInfo(std::string varname, VAPoR::DC::BaseVar& var)
 |  
 |  GetCRatios(...)
 |      std::vector<unsigned long> VAPoR::DC::GetCRatios(std::string varname)
 |  
 |  GetCoordVarInfo(...)
 |      bool VAPoR::DC::GetCoordVarInfo(std::string varname, VAPoR::DC::CoordVar& cvar)
 |  
 |  GetCoordVarNames(...)
 |      std::vector<std::string> VAPoR::DC::GetCoordVarNames()
 |  
 |  GetDataVarInfo(...)
 |      bool VAPoR::DC::GetDataVarInfo(std::string varname, VAPoR::DC::DataVar& datavar)
 |  
 |  GetDataVarNames(...)
 |      std::vector<std::string> VAPoR::DC::GetDataVarNames()
 |      std::vector<std::string> VAPoR::DC::GetDataVarNames(int ndim)
 |  
 |  GetDimLens(...)
 |      int VAPoR::DC::GetDimLens(std::string varname, std::vector<size_t>& dims, long ts = -1)
 |  
 |  GetDimLensAtLevel(...)
 |      int VAPoR::DC::GetDimLensAtLevel(std::string varname, int level, std::vector<size_t>& dims_at_level, std::vector<size_t>& bs_at_level, long ts = -1)
 |  
 |  GetDimension(...)
 |      bool VAPoR::DC::GetDimension(std::string dimname, VAPoR::DC::Dimension& dimension, long ts)
 |  
 |  GetDimensionNames(...)
 |      std::vector<std::string> VAPoR::DC::GetDimensionNames()
 |  
 |  GetHyperSliceInfo(...)
 |      int VAPoR::DC::GetHyperSliceInfo(std::string varname, int level, std::vector<size_t>& dims, size_t& nslice, long ts = -1)
 |  
 |  GetMapProjection(...)
 |      std::string VAPoR::DC::GetMapProjection()
 |  
 |  GetMesh(...)
 |      bool VAPoR::DC::GetMesh(std::string mesh_name, VAPoR::DC::Mesh& mesh)
 |  
 |  GetMeshDimLens(...)
 |      bool VAPoR::DC::GetMeshDimLens(const std::string& mesh_name, std::vector<size_t>& dims, long ts = -1)
 |  
 |  GetMeshDimNames(...)
 |      bool VAPoR::DC::GetMeshDimNames(const std::string& mesh_name, std::vector<std::string>& dimnames)
 |  
 |  GetMeshNames(...)
 |      std::vector<std::string> VAPoR::DC::GetMeshNames()
 |  
 |  GetNumDimensions(...)
 |      unsigned long VAPoR::DC::GetNumDimensions(std::string varname)
 |  
 |  GetNumRefLevels(...)
 |      unsigned long VAPoR::DC::GetNumRefLevels(std::string varname)
 |  
 |  GetNumTimeSteps(...)
 |      int VAPoR::DC::GetNumTimeSteps(std::string varname)
 |  
 |  GetTimeCoordVarNames(...)
 |      std::vector<std::string> VAPoR::DC::GetTimeCoordVarNames()
 |  
 |  GetVar(...)
 |      int VAPoR::DC::GetVar(std::string varname, int level, int lod, float* data)
 |      int VAPoR::DC::GetVar(std::string varname, int level, int lod, double* data)
 |      int VAPoR::DC::GetVar(std::string varname, int level, int lod, int* data)
 |      int VAPoR::DC::GetVar(size_t ts, std::string varname, int level, int lod, float* data)
 |      int VAPoR::DC::GetVar(size_t ts, std::string varname, int level, int lod, double* data)
 |      int VAPoR::DC::GetVar(size_t ts, std::string varname, int level, int lod, int* data)
 |  
 |  GetVarConnVars(...)
 |      bool VAPoR::DC::GetVarConnVars(std::string varname, std::string& face_node_var, std::string& node_face_var, std::string& face_edge_var, std::string& face_face_var, std::string& edge_node_var, std::string& edge_face_var)
 |  
 |  GetVarCoordVars(...)
 |      bool VAPoR::DC::GetVarCoordVars(std::string varname, bool spatial, std::vector<std::string>& coord_vars)
 |  
 |  GetVarDimLens(...)
 |      bool VAPoR::DC::GetVarDimLens(std::string varname, bool spatial, std::vector<size_t>& dimlens, long ts = -1)
 |      bool VAPoR::DC::GetVarDimLens(std::string varname, std::vector<size_t>& sdimlens, size_t& time_dimlen, long ts = -1)
 |  
 |  GetVarDimNames(...)
 |      bool VAPoR::DC::GetVarDimNames(std::string varname, bool spatial, std::vector<std::string>& dimnames)
 |      bool VAPoR::DC::GetVarDimNames(std::string varname, std::vector<std::string>& sdimnames, std::string& time_dimname)
 |  
 |  GetVarDimensions(...)
 |      bool VAPoR::DC::GetVarDimensions(std::string varname, bool spatial, std::vector<VAPoR::DC::Dimension>& dimensions, long ts)
 |  
 |  GetVarGeometryDim(...)
 |      unsigned long VAPoR::DC::GetVarGeometryDim(std::string varname)
 |  
 |  GetVarTopologyDim(...)
 |      unsigned long VAPoR::DC::GetVarTopologyDim(std::string varname)
 |  
 |  Initialize(...)
 |      int VAPoR::DC::Initialize(const std::vector<std::string>& paths, const std::vector<std::string>& options = std::vector<string>())
 |  
 |  IsAuxVar(...)
 |      bool VAPoR::DC::IsAuxVar(std::string varname)
 |  
 |  IsCompressed(...)
 |      bool VAPoR::DC::IsCompressed(std::string varname)
 |  
 |  IsCoordVar(...)
 |      bool VAPoR::DC::IsCoordVar(std::string varname)
 |  
 |  IsDataVar(...)
 |      bool VAPoR::DC::IsDataVar(std::string varname)
 |  
 |  IsTimeVarying(...)
 |      bool VAPoR::DC::IsTimeVarying(std::string varname)
 |  
 |  OpenVariableRead(...)
 |      int VAPoR::DC::OpenVariableRead(size_t ts, std::string varname, int level = 0, int lod = 0)
 |  
 |  Read(...)
 |      int VAPoR::DC::Read(int fd, float* data)
 |      int VAPoR::DC::Read(int fd, double* data)
 |      int VAPoR::DC::Read(int fd, int* data)
 |  
 |  ReadRegion(...)
 |      int VAPoR::DC::ReadRegion(int fd, const std::vector<size_t>& min, const std::vector<size_t>& max, float* region)
 |      int VAPoR::DC::ReadRegion(int fd, const std::vector<size_t>& min, const std::vector<size_t>& max, double* region)
 |      int VAPoR::DC::ReadRegion(int fd, const std::vector<size_t>& min, const std::vector<size_t>& max, int* region)
 |  
 |  ReadSlice(...)
 |      int VAPoR::DC::ReadSlice(int fd, float* slice)
 |      int VAPoR::DC::ReadSlice(int fd, double* slice)
 |      int VAPoR::DC::ReadSlice(int fd, int* slice)
 |  
 |  VariableExists(...)
 |      bool VAPoR::DC::VariableExists(size_t ts, std::string varname, int reflevel = 0, int lod = 0)
 |  
 |  __assign__(...)
 |      VAPoR::DC& VAPoR::DC::operator=(const VAPoR::DC&)
 |  
 |  __init__(...)
 |      DC::DC()
 |      DC::DC(const VAPoR::DC&)
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes defined here:
 |  
 |  DOUBLE = (VAPoR::DC::XType::DOUBLE) : (int) 1
 |  
 |  FLOAT = (VAPoR::DC::XType::FLOAT) : (int) 0
 |  
 |  INT32 = (VAPoR::DC::XType::INT32) : (int) 4
 |  
 |  INT64 = (VAPoR::DC::XType::INT64) : (int) 5
 |  
 |  INT8 = (VAPoR::DC::XType::INT8) : (int) 3
 |  
 |  INVALID = (VAPoR::DC::XType::INVALID) : (int) -1
 |  
 |  TEXT = (VAPoR::DC::XType::TEXT) : (int) 6
 |  
 |  UINT8 = (VAPoR::DC::XType::UINT8) : (int) 2
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

