.. _vapor.utils.FlowRenderer:


vapor.utils.FlowRenderer
------------------------


Help on class FlowRenderer in vapor.utils:

vapor.utils.FlowRenderer = class FlowRenderer(Renderer)
 |  vapor.utils.FlowRenderer(renderParams: cppyy.gbl.VAPoR.RenderParams, id: str)
 |  
 |  Wraps VAPoR::FlowParams
 |  
 |  Method resolution order:
 |      FlowRenderer
 |      Renderer
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetFlowDirection(...)
 |      int VAPoR::FlowParams::GetFlowDirection()
 |          Get the current flow renderer's advection direction.
 |      Returns
 |          int - The advection direction for the current flow renderer. (0 = forward, 1 = backward, 2 = bi-directional)
 |  
 |  GetFlowOutputMoreVariables(...)
 |      std::vector<std::string> VAPoR::FlowParams::GetFlowOutputMoreVariables()
 |          If more than one variable is being sampled along flowlines and is being written to an output file, this returns those variables.
 |      Returns
 |          std::vector<std::string> - A vector containing the variables being written to the specified output file name.
 |  
 |  GetFlowlineOutputFilename(...)
 |      std::string VAPoR::FlowParams::GetFlowlineOutputFilename()
 |          This will return the file path to the text file that data will be written to when outputing flow lines.
 |      Returns
 |          string - The file path of the data file that contains sample values along streamlines/pathlines.
 |  
 |  GetGridNumOfSeeds(...)
 |      std::vector<long> VAPoR::FlowParams::GetGridNumOfSeeds()
 |          Returns the number of seed points on the X, Y, and Z axes if the seeding distribution is Gridded, as determined by GetSeedGenMode()
 |      Returns
 |          std::vector<long> - Number of seeds distributed on the X, Y, and Z axes.
 |  
 |  GetIntegrationRegion(self) -> vapor.renderer.BoundingBox
 |  
 |  GetIsSteady(...)
 |      bool VAPoR::FlowParams::GetIsSteady()
 |          Gets the type of flow rendering algorithm being used. Sets the type of flow rendering algorithm being used.
 |          Steady flow (streamlines) renders time-invariant trajectories that follow a vector field at a single timestep.  Unsteady flow (pathlines) render time-variant trajectories that advect through the timeseries of a loaded dataset.
 |      Parameters
 |          bool - Steady/streamlines = true, Unsteady/pathlines = false
 |      Returns
 |          bool - Steady/streamlines = true, Unsteady/pathlines = false
 |  
 |  GetPeriodic(...)
 |      std::vector<bool> VAPoR::FlowParams::GetPeriodic()
 |          Inquires whether the current flow advection scheme is periodic.
 |          IE - Do pathlines or streamlines continue on the opposite side of the domain when the exit it? Similar to when PAC-MAN exits the right side of the screen, and re-enters on the left.  Note: this result vector could be of size 2 or 3.
 |      Returns
 |          std::vector<bool> - A vector consisting of booleans that indicate periodicity on the X, Y, and Z axes. (false = non-periodic, true = periodic)
 |  
 |  GetPhongAmbient(self) -> float
 |      Specifies the Phong Ambient lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  GetPhongDiffuse(self) -> float
 |      Specifies the Phong Diffuse lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  GetPhongShininess(self) -> float
 |      Specifies the Phong Shininess lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  GetPhongSpecular(self) -> float
 |      Specifies the Phong Specular lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  GetRakeBiasStrength(...)
 |      long VAPoR::FlowParams::GetRakeBiasStrength()
 |          When randomly seeding flowlines with bias towards along a chosen variable's distribution, this returns the bias strength.
 |          Negative bias will place seeds at locations where the bias value has low values. Positive bias will place seeds where the bias variable has high values.
 |      Returns
 |          int - The bias of the seed distribution.
 |  
 |  GetRakeBiasVariable(...)
 |      std::string VAPoR::FlowParams::GetRakeBiasVariable()
 |          Returns the bias variable that randomly seeded flow-lines are distributed towards if the seed generation mode is "Random w/ Bias."
 |      Returns
 |          string - The variable that seeds are biased distributed for.
 |  
 |  GetRakeRegion(self) -> vapor.renderer.BoundingBox
 |  
 |  GetRandomNumOfSeeds(...)
 |      long VAPoR::FlowParams::GetRandomNumOfSeeds()
 |          Returns the number of seed points randomly generated if the seeding distribution is randomly generated, as determined by GetSeedGenMode()
 |      Returns
 |          long - Number of seeds randomly distributed within the seeding rake region.
 |  
 |  GetRenderDensityFalloff(self) -> float
 |      Falloff parameter for the flow density rendering mode as specified in https://www.researchgate.net/publication/261329939_Trajectory_Density_Projection_for_Vector_Field_Visualization Applies data of type: double. Typical values: 0.5 to 10.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  GetRenderDensityToneMapping(self) -> float
 |      ToneMapping parameter for the flow density rendering mode as specified in https://www.researchgate.net/publication/261329939_Trajectory_Density_Projection_for_Vector_Field_Visualization Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  GetRenderFadeTail(self) -> bool
 |      Applies transparency to the tails of pathlines and streamlines. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  GetRenderFadeTailLength(self) -> int
 |      Specifies the length of a faded flow line when animating steady flow. Applies data of type: int. Typical values: 1 to 100. Valid values: INT_MIN to INT_MAX.
 |  
 |  GetRenderFadeTailStart(self) -> int
 |      Specifies the starting integration step for fading a flow line's tail. Applies data of type: int. Typical values: 1 to 100. Valid values: INT_MIN to INT_MAX.
 |  
 |  GetRenderFadeTailStop(self) -> int
 |      Specifies the stopping integration step for fading a flow line's tail. Applies data of type: int. Typical values: 1 to 100. Valid values: INT_MIN to INT_MAX.
 |  
 |  GetRenderGeom3D(self) -> bool
 |      Toggles between rendering 2d glyphs and 3d geometry of the render type. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  GetRenderGlyphOnlyLeading(self) -> bool
 |      When rendering samples, only draw the leading sample in a path. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  GetRenderGlyphStride(self) -> int
 |      When rendering samples, draw every N samples. Applies data of type: int. Typical values: 1 to 20. Valid values: INT_MIN to INT_MAX.
 |  
 |  GetRenderGlyphType(self) -> int
 |      When rendering samples, determines whether samples are rendered as circles or arrows. Applies data of type: long. Valid values: 0 = FloatParams::GlyphTypeSphere, 1 = FloatParams::GlyphTypeArrow.
 |  
 |  GetRenderRadiusScalar(self) -> float
 |      Scales the radius of the flow tube rendering. Applies data of type: double. Typical values: 0.1 to 5.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  GetRenderShowStreamDir(self) -> bool
 |      Draws the direction of the flow stream. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  GetRenderType(self) -> int
 |      The rendering type that represents the flow paths. See RenderType enum class.
 |  
 |  GetSeedGenMode(...)
 |      int VAPoR::FlowParams::GetSeedGenMode()
 |          Get the mode for generating seeds (points of origin) for the flow renderer.
 |      Returns
 |          int - The current seed generation mode for the flow renderer. 0 = Gridded, 1 = Random, 2 = Random with bias, 3 = List of seeds
 |  
 |  GetSeedInjInterval(...)
 |      int VAPoR::FlowParams::GetSeedInjInterval()
 |          Returns the interval that new pathlines are injected into the scene.
 |      Returns
 |          int - The seed injection interval.
 |  
 |  GetSeedInputFilename(...)
 |      std::string VAPoR::FlowParams::GetSeedInputFilename()
 |          Get the file name/path to a file containing a list of seed points to advect from.
 |          See https://vapor.readthedocs.io/en/readthedocs/usage/flowRenderer.html#seed-distribution-settings
 |      Returns
 |          string - A file path containing a defined list of seed points to advect from
 |  
 |  GetSteadyNumOfSteps(...)
 |      long VAPoR::FlowParams::GetSteadyNumOfSteps()
 |          Get the target number of steps to advect a steady flow line (aka a streamline). Set the target number of steps to advect a steady flow line (aka a streamline).
 |          Note 1: Advection can terminate before hitting the specified target number of steps. Common reasons are 1) it travels  out of the volume, and 2) it enters a "sink" where velocity is zero and no longer travels.  Note 2: The advection step size is adjusted internally based on the current curvature, so even with the same steps  being advected, the lengths of advected trajectories can still differ.
 |      Parameters
 |          long - The number of steps a steady flow line targets to advect.
 |      Returns
 |          long - The number of steps a steady flow line targets to advect.
 |  
 |  GetVelocityMultiplier(...)
 |      double VAPoR::FlowParams::GetVelocityMultiplier()
 |          Get the multiplier being applied to the flow advection algorithm.
 |          If there happens to be a mismatch between the units of your data's domain and the units of a variable such as wind speed,  you can scale the wind field with this parameter. IE - If your data's domain is written in kilometers but your wind  vectors are in meters, you can apply a velocity multiplyer of 0.001 to correct the mismatch.
 |      Returns
 |          double - Velocity field multiplier for flow rendering
 |  
 |  SetFlowDirection(...)
 |      void VAPoR::FlowParams::SetFlowDirection(int)
 |          Set the current flow renderer's advection direction.
 |      Parameters
 |          int - The advection direction for the current flow renderer. (0 = forward, 1 = backward, 2 = bi-directional)
 |  
 |  SetFlowlineOutputFilename(...)
 |      void VAPoR::FlowParams::SetFlowlineOutputFilename(const std::string &)
 |          Sets the file path to the text file that flowline output will be written to.
 |      Parameters
 |          string - The file path of the data file that contains sample data along streamlines/pathlines.
 |  
 |  SetGridNumOfSeeds(...)
 |      void VAPoR::FlowParams::SetGridNumOfSeeds(const std::vector< long > &)
 |          Sets the number of seed points on the X, Y, and Z axes if the seeding distribution is Gridded, as determined by GetSeedGenMode()
 |      Returns
 |          std::vector<long> - Number of seeds distributed on the X, Y, and Z axes.
 |  
 |  SetIsSteady(...)
 |      void VAPoR::FlowParams::SetIsSteady(bool steady)
 |          Sets the type of flow rendering algorithm being used.
 |          Steady flow (streamlines) renders time-invariant trajectories that follow a vector field at a single timestep.  Unsteady flow (pathlines) render time-variant trajectories that advect through the timeseries of a loaded dataset.
 |      Parameters
 |          bool - Steady/streamlines = true, Unsteady/pathlines = false
 |  
 |  SetPeriodic(...)
 |      void VAPoR::FlowParams::SetPeriodic(const std::vector< bool > &)
 |          Gets whether the current flow advection scheme is periodic. Inquires whether the current flow advection scheme is periodic.
 |          IE - Do pathlines or streamlines continue on the opposite side of the domain when the exit it? Similar to when PAC-MAN exits the right side of the screen, and re-enters on the left.  Note: this result vector could be of size 2 or 3.
 |      Parameters
 |          std::vector<bool> - A vector consisting of booleans that indicate periodicity on the X, Y, and Z axes. (false = non-periodic, true = periodic)
 |      Returns
 |          std::vector<bool> - A vector consisting of booleans that indicate periodicity on the X, Y, and Z axes. (false = non-periodic, true = periodic)
 |  
 |  SetPhongAmbient(self, value: float)
 |      Specifies the Phong Ambient lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  SetPhongDiffuse(self, value: float)
 |      Specifies the Phong Diffuse lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  SetPhongShininess(self, value: float)
 |      Specifies the Phong Shininess lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  SetPhongSpecular(self, value: float)
 |      Specifies the Phong Specular lighting coefficient ( https://en.wikipedia.org/wiki/Phong_reflection_model ). Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  SetRakeBiasStrength(...)
 |      void VAPoR::FlowParams::SetRakeBiasStrength(long)
 |          When randomly seeding flowlines with bias towards along a chosen variable's distribution, this sets the bias strength.
 |      Parameters
 |          long - The bias of the seed distribution.
 |  
 |  SetRakeBiasVariable(...)
 |      void VAPoR::FlowParams::SetRakeBiasVariable(const std::string &)
 |          Sets the bias variable that randomly seeded flow-lines are distributed towards if the seed generation mode is "Random w/ Bias."
 |      Returns
 |          string - The variable that seeds are biased distributed for.
 |  
 |  SetRandomNumOfSeeds(...)
 |      void VAPoR::FlowParams::SetRandomNumOfSeeds(long)
 |          Sets the number of seed points randomly generated if the seeding distribution is randomly generated, as determined by GetSeedGenMode()
 |      Parameters
 |          long - Number of seeds randomly distributed within the seeding rake region.
 |  
 |  SetRenderDensityFalloff(self, value: float)
 |      Falloff parameter for the flow density rendering mode as specified in https://www.researchgate.net/publication/261329939_Trajectory_Density_Projection_for_Vector_Field_Visualization Applies data of type: double. Typical values: 0.5 to 10.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  SetRenderDensityToneMapping(self, value: float)
 |      ToneMapping parameter for the flow density rendering mode as specified in https://www.researchgate.net/publication/261329939_Trajectory_Density_Projection_for_Vector_Field_Visualization Applies data of type: double. Typical values: 0.0 to 1.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  SetRenderFadeTail(self, value: bool)
 |      Applies transparency to the tails of pathlines and streamlines. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  SetRenderFadeTailLength(self, value: int)
 |      Specifies the length of a faded flow line when animating steady flow. Applies data of type: int. Typical values: 1 to 100. Valid values: INT_MIN to INT_MAX.
 |  
 |  SetRenderFadeTailStart(self, value: int)
 |      Specifies the starting integration step for fading a flow line's tail. Applies data of type: int. Typical values: 1 to 100. Valid values: INT_MIN to INT_MAX.
 |  
 |  SetRenderFadeTailStop(self, value: int)
 |      Specifies the stopping integration step for fading a flow line's tail. Applies data of type: int. Typical values: 1 to 100. Valid values: INT_MIN to INT_MAX.
 |  
 |  SetRenderGeom3D(self, value: bool)
 |      Toggles between rendering 2d glyphs and 3d geometry of the render type. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  SetRenderGlyphOnlyLeading(self, value: bool)
 |      When rendering samples, only draw the leading sample in a path. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  SetRenderGlyphStride(self, value: int)
 |      When rendering samples, draw every N samples. Applies data of type: int. Typical values: 1 to 20. Valid values: INT_MIN to INT_MAX.
 |  
 |  SetRenderGlyphType(self, value: int)
 |      When rendering samples, determines whether samples are rendered as circles or arrows. Applies data of type: long. Valid values: 0 = FloatParams::GlyphTypeSphere, 1 = FloatParams::GlyphTypeArrow.
 |  
 |  SetRenderRadiusScalar(self, value: float)
 |      Scales the radius of the flow tube rendering. Applies data of type: double. Typical values: 0.1 to 5.0. Valid values: DBL_MIN to DBL_MAX.
 |  
 |  SetRenderShowStreamDir(self, value: bool)
 |      Draws the direction of the flow stream. Applies data of type: bool. Valid values: 0 = off, 1 = on.
 |  
 |  SetRenderType(self, value: int)
 |      The rendering type that represents the flow paths. See RenderType enum class.
 |  
 |  SetSeedGenMode(...)
 |      void VAPoR::FlowParams::SetSeedGenMode(int)
 |          Set the mode for generating seeds (points of origin) for the flow renderer.
 |      Parameters
 |          int - The current seed generation mode for the flow renderer. 0 = Gridded, 1 = Random, 2 = Random with bias, 3 = List of seeds
 |  
 |  SetSeedInjInterval(...)
 |      void VAPoR::FlowParams::SetSeedInjInterval(int)
 |          Sets the interval w.r.t. the time steps that new pathlines are injected into the scene. For example, 1 means that seeds are injected at every time step, and 2 means that seeds are injected at every other time step. Note "time step" refers to the data set time step, not the integration time step
 |      Parameters
 |          int - The seed injection interval.
 |  
 |  SetSeedInputFilename(...)
 |      void VAPoR::FlowParams::SetSeedInputFilename(const std::string &)
 |          Set the file name/path to a file containing a list of seed points to advect from. Get the file name/path to a file containing a list of seed points to advect from.
 |          See https://vapor.readthedocs.io/en/readthedocs/usage/flowRenderer.html#seed-distribution-settings
 |      Parameters
 |          string - A file path containing a defined list of seed points to advect from
 |      Returns
 |          string - A file path containing a defined list of seed points to advect from
 |  
 |  SetSteadyNumOfSteps(...)
 |      void VAPoR::FlowParams::SetSteadyNumOfSteps(long)
 |          Set the target number of steps to advect a steady flow line (aka a streamline).
 |          Note 1: Advection can terminate before hitting the specified target number of steps. Common reasons are 1) it travels  out of the volume, and 2) it enters a "sink" where velocity is zero and no longer travels.  Note 2: The advection step size is adjusted internally based on the current curvature, so even with the same steps  being advected, the lengths of advected trajectories can still differ.
 |      Parameters
 |          long - The number of steps a steady flow line targets to advect.
 |  
 |  SetVelocityMultiplier(...)
 |      void VAPoR::FlowParams::SetVelocityMultiplier(double)
 |          Set the multiplier being applied to the flow advection algorithm. Get the multiplier being applied to the flow advection algorithm.
 |          If there happens to be a mismatch between the units of your data's domain and the units of a variable such as wind speed,  you can scale the wind field with this parameter. IE - If your data's domain is written in kilometers but your wind  vectors are in meters, you can apply a velocity multiplyer of 0.001 to correct the mismatch.
 |      Parameters
 |          double - Velocity field multiplier for flow rendering
 |      Returns
 |          double - Velocity field multiplier for flow rendering
 |  
 |  ----------------------------------------------------------------------
 |  Data and other attributes defined here:
 |  
 |  FlowDir = Enum Class
 |      Enum with the following options:
 |          FORWARD
 |          BACKWARD
 |          BI_DIR
 |  
 |  
 |  FlowSeedMode = Enum Class
 |      Enum with the following options:
 |          UNIFORM
 |          RANDOM
 |          RANDOM_BIAS
 |          LIST
 |  
 |  
 |  GlpyhType = Enum Class
 |      Enum with the following options:
 |          GlpyhTypeSphere
 |          GlpyhTypeArrow
 |  
 |  
 |  RenderType = Enum Class
 |      Enum with the following options:
 |          RenderTypeStream
 |          RenderTypeSamples
 |          RenderTypeDensity
 |  
 |  
 |  VaporName = b'Flow'
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

