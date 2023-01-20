.. _vapor.session.Session:


vapor.session.Session
---------------------


Help on class Session in vapor.session:

vapor.session.Session = class Session(cppyy.gbl.Session)
 |  Method resolution order:
 |      Session
 |      cppyy.gbl.Session
 |      cppyy.gbl.CPPInstance
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  CreatePythonDataset(self)
 |      Creates a python dataset or returns one if it already exists for the current session.
 |  
 |  DeleteRenderer(self, renderer: vapor.renderer.Renderer)
 |      void Session::DeleteRenderer(String name)
 |  
 |  GetAxisAnnotations(self) -> vapor.annotations.AxisAnnotation
 |  
 |  GetCamera(self)
 |  
 |  GetDataset(self, name) -> vapor.dataset.Dataset
 |  
 |  GetDatasets(self)
 |  
 |  GetRenderer(self, name)
 |  
 |  GetRenderers(self) -> list[vapor.renderer.Renderer]
 |  
 |  GetSceneAnnotations(self) -> vapor.annotations.SceneAnnotation
 |  
 |  NewRenderer(self, Class: vapor.renderer.Renderer, datasetName: str) -> vapor.renderer.Renderer
 |      std::string Session::NewRenderer(String type, String dataset = "")
 |  
 |  OpenDataset(self, datasetType: str, files: list[str])
 |      Open a dataset of type datasetType from a list of files.
 |      A list of supported dataset types can be retrieved from Dataset.GetDatasetTypes()
 |  
 |  RenderToImage(self, fast=False) -> <module 'PIL.Image' from '/Users/pearse/miniconda3/envs/readTheDocs/lib/python3.9/site-packages/PIL/Image.py'>
 |  
 |  SetResolution(self, width, height)
 |  
 |  Show(self)
 |  
 |  __init__(self)
 |      Session::Session()
 |      Session::Session(const Session&)
 |  
 |  getAnimationParams(...)
 |      AnimationParams* __cppyy_internal::Dispatcher1::getAnimationParams()
 |  
 |  getGUIStateParams(...)
 |      GUIStateParams* __cppyy_internal::Dispatcher1::getGUIStateParams()
 |  
 |  getParamsDatasetInfo(...)
 |      void __cppyy_internal::Dispatcher1::getParamsDatasetInfo(std::string arg0, std::string* arg1, std::vector<std::string>* arg2)
 |  
 |  getSettingsParams(...)
 |      SettingsParams* __cppyy_internal::Dispatcher1::getSettingsParams()
 |  
 |  getWinName(...)
 |      std::string __cppyy_internal::Dispatcher1::getWinName()
 |  
 |  loadAllParamsDatasets(...)
 |      void __cppyy_internal::Dispatcher1::loadAllParamsDatasets()
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes defined here:
 |  
 |  __cpp_cross__ = 'Session'
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from cppyy.gbl.Session:
 |  
 |  CloseAllDatasets(...)
 |      void Session::CloseAllDatasets()
 |  
 |  CloseDataset(...)
 |      void Session::CloseDataset(String name)
 |  
 |  GetDatasetNames(...)
 |      std::vector<std::string> Session::GetDatasetNames()
 |  
 |  GetPythonWinName(...)
 |      std::string Session::GetPythonWinName()
 |  
 |  GetRendererNames(...)
 |      std::vector<std::string> Session::GetRendererNames()
 |  
 |  Load(...)
 |      int Session::Load(String path)
 |  
 |  Render(...)
 |      int Session::Render(String imagePath, bool fast = false)
 |  
 |  Reset(...)
 |      void Session::Reset()
 |  
 |  Save(...)
 |      int Session::Save(String path)
 |  
 |  SetTimestep(...)
 |      void Session::SetTimestep(int ts)
 |  
 |  SetWaspMyBaseErrMsgFilePtrToSTDERR(...)
 |      static void Session::SetWaspMyBaseErrMsgFilePtrToSTDERR()
 |  
 |  __assign__(...)
 |      Session& Session::operator=(const Session&)
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from cppyy.gbl.Session:
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)
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

