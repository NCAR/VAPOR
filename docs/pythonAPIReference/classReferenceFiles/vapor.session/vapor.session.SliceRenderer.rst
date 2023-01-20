.. _vapor.session.SliceRenderer:


vapor.session.SliceRenderer
---------------------------


Help on class SliceRenderer in vapor.session:

vapor.session.SliceRenderer = class SliceRenderer(Renderer)
 |  vapor.session.SliceRenderer(renderParams: cppyy.gbl.VAPoR.RenderParams, id: str)
 |  
 |  Wraps VAPoR::SliceParams
 |  Class that supports drawing Barbs based on 2D or 3D vector field.
 |  
 |  Method resolution order:
 |      SliceRenderer
 |      Renderer
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetSampleRate(self) -> float
 |      If a renderer samples data points through a plane (IE - Slice and Contour), this tag identifies the parameter for how many samples to take along that vector. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: long Valid values: 0 to LONG_MAX
 |  
 |  GetSliceOffset(self) -> float
 |      If a renderer can be offset from a point of origin (IE - Slice and Contour), this tag identifies the parameter for offsetting the renderer from that point. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: DBL_MIN to DBL_MAX
 |  
 |  GetSlicePlaneNormal(...)
 |      vector<double> VAPoR::RenderParams::GetSlicePlaneNormal()
 |          Return the renderer's 3 axis normal for creating ArbitrarilyOrientedRegularGrids.
 |      Returns
 |          vector<double> - Slice's rotation on X, Y, and Z axes, specified in the data's coordinate system Valid values - -1.0 to 1.0 for each axis component
 |  
 |  GetSlicePlaneNormalX(self) -> float
 |      If a renderer can be oriented orthogonally to a normal vector (IE - Slice and Contour), this tag identifies the normal's X component. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Typical values: -1.0 to 1.0 Valid values: DBL_MIN to DBL_MAX
 |  
 |  GetSlicePlaneNormalY(self) -> float
 |      If a renderer can be oriented orthogonally to a normal vector (IE - Slice and Contour), this tag identifies the normal's Y component. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Typical values: -1.0 to 1.0 Valid values: DBL_MIN to DBL_MAX
 |  
 |  GetSlicePlaneNormalZ(self) -> float
 |      If a renderer can be oriented orthogonally to a normal vector (IE - Slice and Contour), this tag identifies the normal's Z component. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Typical values: -1.0 to 1.0 Valid values: DBL_MIN to DBL_MAX
 |  
 |  GetSlicePlaneOrientationMode(self) -> int
 |      If a renderer can be oriented according to 1) a set of rotationis on the XYZ axes, or 2) according to the orthoganality of a specified normal (IE - Slice and Contour), this tag determines which method is being used to orient the the renderer. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: long Valid values: 0 = SlicePlaneOrientationMode::Rotation , 1 = SlicePlaneOrientationMode::Normal
 |  
 |  GetSlicePlaneOrigin(...)
 |      vector<double> VAPoR::RenderParams::GetSlicePlaneOrigin()
 |          Return the renderer's 3 axis origin for creating ArbitrarilyOrientedRegularGrids.
 |      Returns
 |          vector<double> - Slice's origin on X, Y, and Z axes, specified in the data's coordinate system Valid values - A point within the data domain
 |  
 |  GetSlicePlaneRotation(...)
 |      vector<double> VAPoR::RenderParams::GetSlicePlaneRotation()
 |          Return the renderer's 3 axis rotation for creating ArbitrarilyOrientedRegularGrids.
 |      Returns
 |          vector<double> - Slice's rotation on X, Y, and Z axes Valid values - -90.0 to 90.0 for each axis component
 |  
 |  GetXSlicePlaneOrigin(self) -> float
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates
 |  
 |  GetXSlicePlaneRotation(self) -> float
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the rotation about the X axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: -90.0 to 90.0
 |  
 |  GetYSlicePlaneOrigin(self) -> float
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the Y axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: A point within the data domain's Y axis coordinates
 |  
 |  GetYSlicePlaneRotation(self) -> float
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the rotation about the Y axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: -90.0 to 90.0
 |  
 |  GetZSlicePlaneOrigin(self) -> float
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the Z axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: A point within the data domain's Z axis coordinates
 |  
 |  GetZSlicePlaneRotation(self) -> float
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the rotation about the Z axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: -90.0 to 90.0
 |  
 |  SetSampleRate(self, value: float)
 |      If a renderer samples data points through a plane (IE - Slice and Contour), this tag identifies the parameter for how many samples to take along that vector. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: long Valid values: 0 to LONG_MAX
 |  
 |  SetSliceOffset(self, value: float)
 |      If a renderer can be offset from a point of origin (IE - Slice and Contour), this tag identifies the parameter for offsetting the renderer from that point. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: DBL_MIN to DBL_MAX
 |  
 |  SetSlicePlaneNormalX(self, value: float)
 |      If a renderer can be oriented orthogonally to a normal vector (IE - Slice and Contour), this tag identifies the normal's X component. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Typical values: -1.0 to 1.0 Valid values: DBL_MIN to DBL_MAX
 |  
 |  SetSlicePlaneNormalY(self, value: float)
 |      If a renderer can be oriented orthogonally to a normal vector (IE - Slice and Contour), this tag identifies the normal's Y component. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Typical values: -1.0 to 1.0 Valid values: DBL_MIN to DBL_MAX
 |  
 |  SetSlicePlaneNormalZ(self, value: float)
 |      If a renderer can be oriented orthogonally to a normal vector (IE - Slice and Contour), this tag identifies the normal's Z component. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Typical values: -1.0 to 1.0 Valid values: DBL_MIN to DBL_MAX
 |  
 |  SetSlicePlaneOrientationMode(self, value: int)
 |      If a renderer can be oriented according to 1) a set of rotationis on the XYZ axes, or 2) according to the orthoganality of a specified normal (IE - Slice and Contour), this tag determines which method is being used to orient the the renderer. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: long Valid values: 0 = SlicePlaneOrientationMode::Rotation , 1 = SlicePlaneOrientationMode::Normal
 |  
 |  SetXSlicePlaneOrigin(self, value: float)
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates
 |  
 |  SetXSlicePlaneRotation(self, value: float)
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the rotation about the X axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: -90.0 to 90.0
 |  
 |  SetYSlicePlaneOrigin(self, value: float)
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the Y axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: A point within the data domain's Y axis coordinates
 |  
 |  SetYSlicePlaneRotation(self, value: float)
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the rotation about the Y axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: -90.0 to 90.0
 |  
 |  SetZSlicePlaneOrigin(self, value: float)
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the Z axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: A point within the data domain's Z axis coordinates
 |  
 |  SetZSlicePlaneRotation(self, value: float)
 |      If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the rotation about the Z axis. If a renderer supports rotation about a point of origin (IE - Slice and Contour), this tag identifies the parameter for the origin's location on the X axis.
 |      This tag only applies when 3D data are sliced for contouring or pseudo-coloring slices Applies to data of type: double Valid values: A point within the data domain's X axis coordinates Applies to data of type: double Valid values: -90.0 to 90.0
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes defined here:
 |  
 |  SlicePlaneOrientationMode = <class 'vapor.renderer.SlicePlaneOrientati...
 |  
 |  VaporName = b'Slice'
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

