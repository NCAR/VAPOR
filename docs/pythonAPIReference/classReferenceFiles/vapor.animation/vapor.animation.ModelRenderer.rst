.. _vapor.animation.ModelRenderer:


vapor.animation.ModelRenderer
-----------------------------


Help on class ModelRenderer in vapor.animation:

vapor.animation.ModelRenderer = class ModelRenderer(Renderer)
 |  vapor.animation.ModelRenderer(renderParams: cppyy.gbl.VAPoR.RenderParams, id: str)
 |  
 |  Wraps VAPoR::ModelParams
 |  
 |  Method resolution order:
 |      ModelRenderer
 |      Renderer
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetFile(self) -> str
 |      Path to a 3D model file or Vapor vms scene file. 3D models can be in formats supported by the ASSIMP library. A description of the Vapor vms scene format can be found at https://vapor.readthedocs.io/en/readthedocs/usage/modelRenderer.html Applies to data of type: string
 |  
 |  SetFile(self, value: str)
 |      Path to a 3D model file or Vapor vms scene file. 3D models can be in formats supported by the ASSIMP library. A description of the Vapor vms scene format can be found at https://vapor.readthedocs.io/en/readthedocs/usage/modelRenderer.html Applies to data of type: string
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes defined here:
 |  
 |  SlicePlaneOrientationMode = <class 'vapor.renderer.SlicePlaneOrientati...
 |  
 |  VaporName = b'Model'
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from Renderer:
 |  
 |  GetAuxVariableNames(...)
 |      vector<string> VAPoR::RenderParams::GetAuxVariableNames()
 |          Get the auxiliary variable names, e.g. "position along flow"
 |          The default is a vector of length containing the empty string.
 |      Returns
 |          vector<string> variable name
 |  
 |  GetColorMapVariableName(...)
 |      string VAPoR::RenderParams::GetColorMapVariableName()
 |          Get the color mapping variable name if any
 |      Returns
 |          string variable name
 |  
 |  GetColorbarAnnotation(self) -> vapor.annotations.ColorbarAnnotation
 |  
 |  GetCompressionLevel(...)
 |      int VAPoR::RenderParams::GetCompressionLevel()
 |          virtual method indicates current Compression level.
 |      Returns
 |          integer compression level, 0 is most compressed
 |  
 |  GetFieldVariableNames(...)
 |      vector<string> VAPoR::RenderParams::GetFieldVariableNames()
 |          Get the field variable names, e.g. used in flow integration.
 |      Returns
 |          vector<string> variable names. A vector of length 3 containing variable names. The default is 3 empty variable names.
 |  
 |  GetHeightVariableName(...)
 |      string VAPoR::RenderParams::GetHeightVariableName()
 |          Determine variable name being used for terrain height (above or below sea level)
 |      Returns
 |          const string& variable name
 |  
 |  GetPrimaryTransferFunction(self) -> vapor.transferfunction.TransferFunction
 |      Returns the transfer function for the primary rendered variable.
 |      This is usually the variable that is being colormapped and would be
 |      represented by the colorbar.
 |  
 |  GetRefinementLevel(...)
 |      int VAPoR::RenderParams::GetRefinementLevel()
 |          Virtual method indicates current number of refinements of this Params.
 |      Returns
 |          integer number of refinements
 |  
 |  GetRenderRegion(self) -> vapor.renderer.BoundingBox
 |  
 |  GetTransferFunction(self, varname: str) -> vapor.transferfunction.TransferFunction
 |  
 |  GetTransform(...)
 |      Transform* VAPoR::RenderParams::GetTransform()
 |  
 |  GetVariableName(...)
 |      string VAPoR::RenderParams::GetVariableName()
 |          Get the primary variable name, e.g. used in color mapping or rendering. The default is the empty string, which indicates a no variable.
 |      Returns
 |          string variable name
 |  
 |  GetXFieldVariableName(...)
 |      std::string VAPoR::RenderParams::GetXFieldVariableName()
 |          Get the X field variable name, e.g. used in flow integration.
 |      Returns
 |          std::string X field variable name.
 |  
 |  GetYFieldVariableName(...)
 |      std::string VAPoR::RenderParams::GetYFieldVariableName()
 |          Get the Y field variable name, e.g. used in flow integration.
 |      Returns
 |          std::string Y field variable name.
 |  
 |  GetZFieldVariableName(...)
 |      std::string VAPoR::RenderParams::GetZFieldVariableName()
 |          Get the Z field variable name, e.g. used in flow integration.
 |      Returns
 |          std::string Z field variable name.
 |  
 |  IsEnabled(...)
 |      bool VAPoR::RenderParams::IsEnabled()
 |          Determine if this params has been enabled for rendering
 |          Default is false.
 |      Returns
 |          bool true if enabled
 |  
 |  ResetUserExtentsToDataExents(...)
 |      int VAPoR::RenderParams::ResetUserExtentsToDataExents(string var="")
 |  
 |  SetAuxVariableNames(...)
 |      void VAPoR::RenderParams::SetAuxVariableNames(vector< string > varName)
 |          Specify auxiliary variable name; e.g. "Position along Flow" The default is a vector of length containing the empty string.
 |      Parameters
 |          string varNames. If any element is "0" the element will be quietly set to the empty string, "".
 |  
 |  SetColorMapVariableName(...)
 |      void VAPoR::RenderParams::SetColorMapVariableName(string varname)
 |          Specify the variable being used for color mapping
 |      Parameters
 |          string varName. If any varName is "0" it will be quietly set to the empty string, "".
 |  
 |  SetCompressionLevel(...)
 |      void VAPoR::RenderParams::SetCompressionLevel(int val)
 |          Virtual method sets current Compression level.
 |      Parameters
 |          val compression level, 0 is most compressed
 |  
 |  SetDimensions(self, dim: int)
 |  
 |  SetEnabled(...)
 |      void VAPoR::RenderParams::SetEnabled(bool val)
 |          Enable or disable this params for rendering
 |          This should be executed between start and end capture which provides the appropriate undo/redo support Accordingly this will not make an entry in the undo/redo queue.
 |          Default is false.
 |      Parameters
 |          bool true to enable, false to disable.
 |  
 |  SetFieldVariableNames(...)
 |      void VAPoR::RenderParams::SetFieldVariableNames(vector< string > varNames)
 |          Specify field variable names; e.g. used in flow integration can be 0 or 3 strings
 |      Parameters
 |          string varNames. If any element is "0" the element will be quietly set to the empty string, "".
 |  
 |  SetHeightVariableName(...)
 |      void VAPoR::RenderParams::SetHeightVariableName(string varname)
 |          Specify the variable being used for height Overrides method on RenderParams
 |      Parameters
 |          string varName. If any varName is "0" it will be quietly set to the empty string, "".
 |      Returns
 |          int 0 if successful;
 |  
 |  SetRefinementLevel(...)
 |      void VAPoR::RenderParams::SetRefinementLevel(int numrefinements)
 |          Virtual method sets current number of refinements of this Params.
 |      Parameters
 |          int refinements
 |  
 |  SetUseSingleColor(...)
 |      void VAPoR::RenderParams::SetUseSingleColor(bool val)
 |          Turn on or off the use of single constant color (versus color map)
 |      Parameters
 |          val true will enable constant color
 |  
 |  SetVariableName(self, name: str)
 |  
 |  SetXFieldVariableName(...)
 |      void VAPoR::RenderParams::SetXFieldVariableName(std::string varName)
 |          Set the X field variable name, e.g. used in flow integration.
 |      Parameters
 |          std::string varName for X field
 |  
 |  SetYFieldVariableName(...)
 |      void VAPoR::RenderParams::SetYFieldVariableName(std::string varName)
 |          Set the Y field variable name, e.g. used in flow integration.
 |      Parameters
 |          std::string varName for Y field
 |  
 |  SetZFieldVariableName(...)
 |      void VAPoR::RenderParams::SetZFieldVariableName(std::string varName)
 |          Set the Z field variable name, e.g. used in flow integration.
 |      Parameters
 |          std::string varName for Z field
 |  
 |  UseSingleColor(...)
 |      bool VAPoR::RenderParams::UseSingleColor()
 |      Indicate if a single (constant) color is being used
 |  
 |  __init__(self, renderParams: cppyy.gbl.VAPoR.RenderParams, id: str)
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

