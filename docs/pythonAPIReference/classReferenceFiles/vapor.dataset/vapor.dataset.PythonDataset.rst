.. _vapor.dataset.PythonDataset:


vapor.dataset.PythonDataset
---------------------------


Help on class PythonDataset in vapor.dataset:

vapor.dataset.PythonDataset = class PythonDataset(Dataset)
 |  vapor.dataset.PythonDataset(dataMgr: cppyy.gbl.VAPoR.DataMgr, id: str, ses)
 |  
 |  Wraps VAPoR::PythonDataMgr
 |  DataMgr for data loaded from python scripts.
 |  
 |  Method resolution order:
 |      PythonDataset
 |      Dataset
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  AddNumpyData(self, name: str, arr: numpy.ndarray)
 |  
 |  AddXArrayData(self, varName: str, arr: xarray.core.dataarray.DataArray)
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from Dataset:
 |  
 |  GetCoordVarNames(...)
 |      std::vector<string> VAPoR::DataMgr::GetCoordVarNames()
 |          Return a list of names for all of the defined coordinate variables.
 |          This method returns a list of all coordinate variables defined in the data set.
 |      Returns
 |          list A vector containing a list of all the coordinate variable names
 |      See Also
 |          GetDataVarNames()
 |  
 |  GetDataRange(self, varname: str, atTimestep: int = 0)
 |  
 |  GetDataVarNames(...)
 |      std::vector<string> VAPoR::DataMgr::GetDataVarNames()
 |          Return a list of names for all of the defined data variables.
 |          This method returns a list of all data variables defined in the data set.
 |            Test  New in 3.0
 |      Returns
 |          list A vector containing a list of all the data variable names
 |      See Also
 |          GetCoordVarNames()
 |      
 |      std::vector<string> VAPoR::DataMgr::GetDataVarNames(int ndim, VarType type=VarType::Any)
 |  
 |  GetDimensionLength(...)
 |      long VAPoR::DataMgr::GetDimensionLength(string name, long ts)
 |          Returns the length of a dimension at a given timestep
 |      Returns
 |          length A negative int is returned on failure
 |  
 |  GetDimensionNames(...)
 |      std::vector<string> VAPoR::DataMgr::GetDimensionNames()
 |      Return names of all defined dimensions
 |      This method returns the list of names of all of the dimensions defined in the DC .
 |  
 |  GetName(self)
 |  
 |  GetNumTimeSteps(...)
 |      int VAPoR::DataMgr::GetNumTimeSteps(string varname)
 |          Return the time dimension length for a variable
 |          Returns the number of time steps (length of the time dimension) for which a variable is defined. If varname does not have a time coordinate 1 is returned. If varname is not defined as a variable a negative int is returned.
 |      Parameters
 |          varname A string specifying the name of the variable.
 |      Returns
 |          count The length of the time dimension, or a negative int if varname is undefined.
 |      See Also
 |          IsTimeVarying()
 |      
 |      int VAPoR::DataMgr::GetNumTimeSteps()
 |          Return the maximum time dimension length for this data set
 |          Returns the number of time steps (length of the time dimension) for any variable is defined.
 |  
 |  GetTimeCoordVarName(...)
 |      string VAPoR::DataMgr::GetTimeCoordVarName()
 |      Get time coordinate var name
 |      Return the name of the time coordinate variable. If no time coordinate variable is defined the empty string is returned.
 |  
 |  GetTransform(self)
 |  
 |  GetVarCoordVars(...)
 |      bool VAPoR::DataMgr::GetVarCoordVars(string varname, bool spatial, std::vector< string > &coord_vars)
 |          Return an ordered list of a data variable's coordinate names
 |          Returns a list of a coordinate variable names for the variable varname , ordered from fastest to slowest. If spatial is true and the variable is time varying the time coordinate variable name will be included. The time coordinate variable is always the slowest varying coordinate axis
 |      Parameters
 |          varname A valid variable name spatial If true only return spatial dimensions coordvars Ordered list of coordinate variable names.
 |      Returns
 |          Returns true upon success, false if the variable is not defined.
 |      
 |      vector<string> VAPoR::DataMgr::GetVarCoordVars(string varname, bool spatial)
 |  
 |  GetVarGeometryDim(...)
 |      size_t VAPoR::DataMgr::GetVarGeometryDim(string varname)
 |  
 |  GetVarTopologyDim(...)
 |      size_t VAPoR::DataMgr::GetVarTopologyDim(string varname)
 |  
 |  IsTimeVarying(...)
 |      bool VAPoR::DataMgr::IsTimeVarying(string varname)
 |          Return a boolean indicating whether a variable is time varying
 |          This method returns true if the variable named by varname is defined and it has a time axis dimension. If either of these conditions is not true the method returns false.
 |      Parameters
 |          varname A string specifying the name of the variable.
 |      Returns
 |          bool Returns true if variable varname exists and is time varying.
 |  
 |  NewRenderer(self, Class: vapor.renderer.Renderer) -> vapor.renderer.Renderer
 |  
 |  __init__(self, dataMgr: cppyy.gbl.VAPoR.DataMgr, id: str, ses)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  __repr__(self)
 |      Return repr(self).
 |  
 |  ----------------------------------------------------------------------
 |  Static methods inherited from Dataset:
 |  
 |  GetDatasetTypes()
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

