#ifndef FLOWPARAMS_H
#define FLOWPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vector>
#include <utility>

namespace VAPoR {

//
// These two enums are used across params, GUI, and renderer.
// Note: use static_cast to cast between them and int types.
//
enum class FlowSeedMode : int { UNIFORM = 0, RANDOM = 1, RANDOM_BIAS = 2, LIST = 3 };
enum class FlowDir : int { FORWARD = 0, BACKWARD = 1, BI_DIR = 2 };

class FlowParams;
class PARAMS_API FakeRakeBox : public Box {
    string _tag;

public:
    using Box::Box;
    FlowParams *parent = nullptr;

    void Initialize(string tag);
    void SetExtents(const vector<double> &minExt, const vector<double> &maxExt) override;
};

class PARAMS_API FlowParams : public RenderParams {
    StateSave    _fakeRakeStateSave;
    FakeRakeBox *_fakeRakeBox = nullptr;
    FakeRakeBox *_fakeIntegrationBox = nullptr;
    bool         _initialized = false;

    void   _setRakeCenter(int dim, double center);
    double _getRakeCenter(int dim);

public:
    enum RenderType { RenderTypeStream, RenderTypeSamples, RenderTypeDensity };
    enum GlpyhType { GlpyhTypeSphere, GlpyhTypeArrow };

    // Constructors
    FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave);
    FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    FlowParams(const FlowParams &rhs);
    FlowParams &operator=(const FlowParams &rhs);

    virtual ~FlowParams();

    virtual int Initialize() override;

    static std::string GetClassType() { return ("FlowParams"); }

    //! \copydoc RenderParams::SetDefaultVariables()
    void SetDefaultVariables(int dim = 3, bool secondaryColormapVariable = false) override;

    //! Sets the type of flow rendering algorithm being used.
    //! \details Steady flow (streamlines) renders time-invariant trajectories that follow a vector field at a single timestep.\n
    //! Unsteady flow (pathlines) render time-variant trajectories that advect through the timeseries of a loaded dataset.
    //! \param[in] bool - Steady/streamlines = true, Unsteady/pathlines = false
    void SetIsSteady(bool steady);
    
    //! Gets the type of flow rendering algorithm being used.
    //! \copydetails FlowParams::SetIsSteady(bool)
    //! \retval bool - Steady/streamlines = true, Unsteady/pathlines = false
    bool GetIsSteady() const;

    //! Get the multiplier being applied to the flow advection algorithm.
    //! \details If there happens to be a mismatch between the units of your data's domain and the units of a variable such as wind speed,\n
    //! you can scale the wind field with this parameter.  IE - If your data's domain is written in kilometers but your wind\n
    //! vectors are in meters, you can apply a velocity multiplyer of 0.001 to correct the mismatch.
    //! \retval double - Velocity field multiplier for flow rendering
    double GetVelocityMultiplier() const;

    //! Get the multiplier being applied to the first step size of flow integration.
    //! \details VAPOR estimates a step size to be applied to the first step of flow integration,\n
    //!          and then dynamically adjusts that value afterwards. However, VAPOR may fail to produce a proper estimate \n
    //!          (mostly too big of an estimate) and result in obviously wrong flow lines. \n
    //!          In such cases, users may manually apply a multiplier to the first step size.
    //! \retval double - First step size multiplier for flow rendering.
    double GetFirstStepSizeMultiplier() const;
    
    //! Set the multiplier being applied to the flow advection algorithm.
    //! \copydetails FlowParams::GetVelocityMultiplier()
    //! \param[in] double - Velocity field multiplier for flow rendering
    void SetVelocityMultiplier(double);
    
    //! Set the multiplier being applied to the first step size of flow advection.
    //! \copydetails FlowParams::GetFirstStepSizeMultiplier()
    //! \param[in] double - Velocity field multiplier for flow rendering
    void SetFirstStepSizeMultiplier(double);

    //! Get the target number of steps to advect a steady flow line (aka a streamline).
    //! \copydetails FlowParams::SetSteadyNumOfSteps()
    //! \retval long - The number of steps a steady flow line targets to advect.
    long GetSteadyNumOfSteps() const;
    
    //! Set the target number of steps to advect a steady flow line (aka a streamline).
    //! \details Note 1: Advection can terminate before hitting the specified target number of steps. Common reasons are 1) it travels \n
    //!         out of the volume, and 2) it enters a "sink" where velocity is zero and no longer travels.\n
    //! Note 2: The advection step size is adjusted internally based on the current curvature, so even with the same steps\n
    //!         being advected, the lengths of advected trajectories can still differ.
    //! \param[in] long - The number of steps a steady flow line targets to advect.
    void SetSteadyNumOfSteps(long);

    //! Get the mode for generating seeds (points of origin) for the flow renderer.
    //! \retval int - The current seed generation mode for the flow renderer. 0 = Gridded, 1 = Random, 2 = Random with bias, 3 = List of seeds
    int  GetSeedGenMode() const;
    
    //! Set the mode for generating seeds (points of origin) for the flow renderer.
    //! \param[in] int - The current seed generation mode for the flow renderer. 0 = Gridded, 1 = Random, 2 = Random with bias, 3 = List of seeds
    void SetSeedGenMode(int);

    //! Enable or disable the writing of flow renderer data values to a text file.
    //! \param[in] bool - Enable (true) or disable (false) the writing of trajectory data values to a text file.
    void SetNeedFlowlineOutput(bool);
    
    //! Inquire whether the writing of flow renderer data values are being written to a text file.
    //! \retval bool - Enable (true) or disable (false) the writing of trajectory data values to a text file.
    bool GetNeedFlowlineOutput() const;

    //! Get the current flow renderer's advection direction.
    //! \retval int - The advection direction for the current flow renderer.  (0 = forward, 1 = backward, 2 = bi-directional)
    int  GetFlowDirection() const;
    
    //! Set the current flow renderer's advection direction.
    //! \param[in] int - The advection direction for the current flow renderer.  (0 = forward, 1 = backward, 2 = bi-directional)
    void SetFlowDirection(int);

    //! Get the file name/path to a file containing a list of seed points to advect from.
    //! \details See https://vapor.readthedocs.io/en/readthedocs/usage/flowRenderer.html#seed-distribution-settings
    //! \retval string - A file path containing a defined list of seed points to advect from
    std::string GetSeedInputFilename() const;
    
    //! Set the file name/path to a file containing a list of seed points to advect from.
    //! \copydetails FlowParams::GetSeedInputFilename()
    //! \param[in] string - A file path containing a defined list of seed points to advect from
    void SetSeedInputFilename(const std::string &);

    //! This will return the file path to the text file that data will be written to when outputing flow lines.
    //! \retval string - The file path of the data file that contains sample values along streamlines/pathlines.
    std::string GetFlowlineOutputFilename() const;
    
    //! Sets the file path to the text file that flowline output will be written to.
    //! \param[in] string - The file path of the data file that contains sample data along streamlines/pathlines.
    void SetFlowlineOutputFilename(const std::string &);

    //! If more than one variable is being sampled along flowlines and is being written to an output file, this returns those variables.
    //! \retval std::vector<std::string> - A vector containing the variables being written to the specified output file name.
    std::vector<std::string> GetFlowOutputMoreVariables() const;

    //! One or more variable to be sampled along flowlines and written to an output file.
    //! \param[in] std::vector<std::string> - A vector containing the variables being written to the specified output file name.
    void SetFlowOutputMoreVariables(std::vector<std::string> vars);

    //! Inquires whether the current flow advection scheme is periodic.
    //! \details IE - Do pathlines or streamlines continue on the opposite side of the domain when the exit it?  Similar to when PAC-MAN exits the right side of the screen, and re-enters on the left.\n
    //! Note: this result vector could be of size 2 or 3.
    //! \retval std::vector<bool> - A vector consisting of booleans that indicate periodicity on the X, Y, and Z axes.  (false = non-periodic, true = periodic)
    std::vector<bool> GetPeriodic() const;

    //! Gets whether the current flow advection scheme is periodic.
    //! \copydetails FlowParams::GetPeriodic()
    //! \param[in] std::vector<bool> - A vector consisting of booleans that indicate periodicity on the X, Y, and Z axes.  (false = non-periodic, true = periodic)
    void SetPeriodic(const std::vector<bool> &);

    /*
     * 6 values to represent a rake in this particular order:
     * xmin, xmax, ymin, ymax, zmin, zmax
     * If the rake wasn't set by users, it returns a vector containing nans.
     * If it represents a 2D area, then it will contain the first 4 elements.
     */
    Box *              GetRakeBox();
    std::vector<float> GetRake() const;
    void               SetRake(const std::vector<float> &);

    Box *GetIntegrationBox();
    void SetIntegrationVolume(const std::vector<float> &);

    //! Returns the number of seed points on the X, Y, and Z axes if the seeding distribution is Gridded, as determined by GetSeedGenMode() 
    //! \retval std::vector<long> - Number of seeds distributed on the X, Y, and Z axes.
    std::vector<long> GetGridNumOfSeeds() const;
    
    //! Sets the number of seed points on the X, Y, and Z axes if the seeding distribution is Gridded, as determined by GetSeedGenMode()
    //! \retval std::vector<long> - Number of seeds distributed on the X, Y, and Z axes.
    void SetGridNumOfSeeds(const std::vector<long> &);

    //! Returns the number of seed points randomly generated if the seeding distribution is randomly generated, as determined by GetSeedGenMode()
    //! \retval long - Number of seeds randomly distributed within the seeding rake region.
    long GetRandomNumOfSeeds() const;
    
    //! Sets the number of seed points randomly generated if the seeding distribution is randomly generated, as determined by GetSeedGenMode()
    //! \param[in] long - Number of seeds randomly distributed within the seeding rake region.
    void SetRandomNumOfSeeds(long);

    //! Returns the bias variable that randomly seeded flow-lines are distributed towards if the seed generation mode is "Random w/ Bias."
    //! \retval string - The variable that seeds are biased distributed for.
    std::string GetRakeBiasVariable() const;
    
    //! Sets the bias variable that randomly seeded flow-lines are distributed towards if the seed generation mode is "Random w/ Bias."
    //! \retval string - The variable that seeds are biased distributed for.
    void SetRakeBiasVariable(const std::string &);

    //! When randomly seeding flowlines with bias towards along a chosen variable's distribution, this returns the bias strength.  
    //! \details Negative bias will place seeds at locations where the bias value has low values.  Positive bias will place seeds where the bias variable has high values.
    //! \retval int - The bias of the seed distribution.
    long GetRakeBiasStrength() const;

    //! When randomly seeding flowlines with bias towards along a chosen variable's distribution, this sets the bias strength.
    //! \copydetails
    //! \param[in] long - The bias of the seed distribution.
    void SetRakeBiasStrength(long);


    int  GetPastNumOfTimeSteps() const;
    void SetPastNumOfTimeSteps(int);

    //! Returns the interval that new pathlines are injected into the scene.
    //! \retval int - The seed injection interval.
    int  GetSeedInjInterval() const;
    
    //! Sets the interval w.r.t. the time steps that new pathlines are injected into the scene.
    //! For example, 1 means that seeds are injected at every time step, and 2 means that seeds are injected at every other time step. Note  "time step" refers to the data set time step, not the integration time step
    //! \param[in] int - The seed injection interval.
    void SetSeedInjInterval(int);

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override
    {
        for (const auto &p : GetFieldVariableNames()) {
            if (!p.empty()) return _dataMgr->GetVarTopologyDim(p);
        }
        return GetBox()->IsPlanar() ? 2 : 3;
    }

    //! \copydoc RenderParams::GetActualColorMapVariableName()
    virtual string GetActualColorMapVariableName() const override
    {
        if (UseSingleColor())
            return "";
        else
            return GetColorMapVariableName();
    }

    //! Get the rake's center position on the X axis
    //! \retval X rake center
    double GetXRakeCenter();

    //! Set the rake's center position on the X axis
    //! \param[in] Rake center position on X axis
    void SetXRakeCenter(double center);

    //! Get the rake's center position on the Y axis
    //! \retval Y rake center
    double GetYRakeCenter();

    //! Set the rake's center position on the Y axis
    //! \param[in] Rake center position on Y axis
    void SetYRakeCenter(double center);

    //! Get the rake's center position on the Z axis
    //! \retval Z rake center
    double GetZRakeCenter();

    //! Set the rake's center position on the Z axis
    //! \param[in] Rake center position on Z axis
    void SetZRakeCenter(double center);

    //! The rendering type that represents the flow paths.
    //! See RenderType enum class.
    static const std::string RenderTypeTag;

    static const std::string RenderRadiusBaseTag;

    //! Scales the radius of the flow tube rendering.
    //! Applies data of type: double.
    //! Typical values: 0.1 to 5.0.
    //! Valid values: DBL_MIN to DBL_MAX.
    static const std::string RenderRadiusScalarTag;

    //! Toggles between rendering 2d glyphs and 3d geometry of the render type.
    //! Applies data of type: bool.
    //! Valid values: 0 = off, 1 = on.
    static const std::string RenderGeom3DTag;

    static const std::string RenderLightAtCameraTag;

    //! Draws the direction of the flow stream.
    //! Applies data of type: bool.
    //! Valid values: 0 = off, 1 = on.
    static const std::string RenderShowStreamDirTag;
    
    //! When rendering samples, determines whether samples are rendered as circles or arrows.
    //! Applies data of type: long.
    //! Valid values: 0 = FloatParams::GlyphTypeSphere, 1 = FloatParams::GlyphTypeArrow.
    static const std::string RenderGlyphTypeTag;

    //! When rendering samples, draw every N samples.
    //! Applies data of type: int.
    //! Typical values: 1 to 20.
    //! Valid values: INT_MIN to INT_MAX.
    static const std::string RenderGlyphStrideTag;

    //! When rendering samples, only draw the leading sample in a path.
    //! Applies data of type: bool.
    //! Valid values: 0 = off, 1 = on.
    static const std::string RenderGlyphOnlyLeadingTag;

    //! Falloff parameter for the flow density rendering mode as specified in
    //! https://www.researchgate.net/publication/261329939_Trajectory_Density_Projection_for_Vector_Field_Visualization
    //! Applies data of type: double.
    //! Typical values: 0.5 to 10.0.
    //! Valid values: DBL_MIN to DBL_MAX.
    static const std::string RenderDensityFalloffTag;

    //! ToneMapping parameter for the flow density rendering mode as specified in
    //! https://www.researchgate.net/publication/261329939_Trajectory_Density_Projection_for_Vector_Field_Visualization
    //! Applies data of type: double.
    //! Typical values: 0.0 to 1.0.
    //! Valid values: DBL_MIN to DBL_MAX.
    static const std::string RenderDensityToneMappingTag;

    //! Applies transparency to the tails of pathlines and streamlines.
    //! Applies data of type: bool.
    //! Valid values: 0 = off, 1 = on.
    static const std::string RenderFadeTailTag;

    //! Specifies the starting integration step for fading a flow line's tail.
    //! Applies data of type: int.
    //! Typical values: 1 to 100.
    //! Valid values: INT_MIN to INT_MAX.
    static const std::string RenderFadeTailStartTag;


    //! Specifies the stopping integration step for fading a flow line's tail.
    //! Applies data of type: int.
    //! Typical values: 1 to 100.
    //! Valid values: INT_MIN to INT_MAX.
    static const std::string RenderFadeTailStopTag;

    //! Specifies the length of a faded flow line when animating steady flow.
    //! Applies data of type: int.
    //! Typical values: 1 to 100.
    //! Valid values: INT_MIN to INT_MAX.
    static const std::string RenderFadeTailLengthTag;

    //! Specifies the Phong Ambient lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Applies data of type: double.
    //! Typical values: 0.0 to 1.0.
    //! Valid values: DBL_MIN to DBL_MAX.
    static const std::string PhongAmbientTag;

    //! Specifies the Phong Diffuse lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Applies data of type: double.
    //! Typical values: 0.0 to 1.0.
    //! Valid values: DBL_MIN to DBL_MAX.
    static const std::string PhongDiffuseTag;

    //! Specifies the Phong Specular lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Applies data of type: double.
    //! Typical values: 0.0 to 1.0.
    //! Valid values: DBL_MIN to DBL_MAX.
    static const std::string PhongSpecularTag;

    //! Specifies the Phong Shininess lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Applies data of type: double.
    //! Typical values: 0.0 to 100.0.
    //! Valid values: DBL_MIN to DBL_MAX.
    static const std::string PhongShininessTag;

    static const std::string _isSteadyTag;
    static const std::string _velocityMultiplierTag;
    static const std::string _firstStepSizeMultiplierTag;
    static const std::string _steadyNumOfStepsTag;
    static const std::string _seedGenModeTag;
    static const std::string _seedInputFilenameTag;
    static const std::string _flowlineOutputFilenameTag;
    static const std::string _flowOutputMoreVariablesTag;
    static const std::string _flowDirectionTag;
    static const std::string _needFlowlineOutputTag;
    static const std::string _xPeriodicTag;
    static const std::string _yPeriodicTag;
    static const std::string _zPeriodicTag;
    static const std::string _rakeTag;
    static const std::string _doIntegrationTag;
    static const std::string _integrationScalarTag;
    static const std::string _integrationSetAllToFinalValueTag;
    static const std::string _integrationBoxTag;
    static const std::string _rakeBiasVariable;
    static const std::string _rakeBiasStrength;
    static const std::string _pastNumOfTimeSteps;
    static const std::string _seedInjInterval;
    static const std::string _xGridNumOfSeedsTag;
    static const std::string _yGridNumOfSeedsTag;
    static const std::string _zGridNumOfSeedsTag;
    static const std::string _randomNumOfSeedsTag;

    // maps between ints and "human readable" strings
    const std::vector<std::pair<int, std::string>> _seed2Str = {{static_cast<int>(FlowSeedMode::UNIFORM), ""},    // default value
                                                                {static_cast<int>(FlowSeedMode::UNIFORM), "UNIFORM"},
                                                                {static_cast<int>(FlowSeedMode::RANDOM), "RANDOM"},
                                                                {static_cast<int>(FlowSeedMode::RANDOM_BIAS), "RANDOM_BIAS"},
                                                                {static_cast<int>(FlowSeedMode::LIST), "LIST"}};

    const std::vector<std::pair<int, std::string>> _dir2Str = {{static_cast<int>(FlowDir::FORWARD), ""},    // default value
                                                               {static_cast<int>(FlowDir::FORWARD), "FORWARD"},
                                                               {static_cast<int>(FlowDir::BACKWARD), "BACKWARD"},
                                                               {static_cast<int>(FlowDir::BI_DIR), "BI_DIRECTIONAL"}};
};

}    // namespace VAPoR

#endif
