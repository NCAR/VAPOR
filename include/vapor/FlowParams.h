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
public:
    using Box::Box;
    FlowParams *parent = nullptr;
    void        SetExtents(const vector<double> &minExt, const vector<double> &maxExt) override;
};

class PARAMS_API FlowParams : public RenderParams {
    StateSave    _fakeRakeStateSave;
    FakeRakeBox *_fakeRakeBox = nullptr;
    bool         _initialized = false;

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

    // True  == Steady; False == Unteady
    void SetIsSteady(bool steady);
    bool GetIsSteady() const;

    double GetVelocityMultiplier() const;
    void   SetVelocityMultiplier(double);

    long GetSteadyNumOfSteps() const;
    void SetSteadyNumOfSteps(long);

    int  GetSeedGenMode() const;
    void SetSeedGenMode(int);

    void SetNeedFlowlineOutput(bool);
    bool GetNeedFlowlineOutput() const;

    int  GetFlowDirection() const;
    void SetFlowDirection(int);

    std::string GetSeedInputFilename() const;
    void        SetSeedInputFilename(const std::string &);

    std::string GetFlowlineOutputFilename() const;
    void        SetFlowlineOutputFilename(const std::string &);

    std::vector<std::string> GetFlowOutputMoreVariables() const;

    // Note: this result vector could be of size 2 or 3.
    std::vector<bool> GetPeriodic() const;
    void              SetPeriodic(const std::vector<bool> &);

    /*
     * 6 values to represent a rake in this particular order:
     * xmin, xmax, ymin, ymax, zmin, zmax
     * If the rake wasn't set by users, it returns a vector containing nans.
     * If it represents a 2D area, then it will contain the first 4 elements.
     */
    Box *              GetRakeBox();
    std::vector<float> GetRake() const;
    void               SetRake(const std::vector<float> &);

    /*
     *This result vector could be of size 2 or 3.
     */
    std::vector<long> GetGridNumOfSeeds() const;
    void              SetGridNumOfSeeds(const std::vector<long> &);

    /*
     * 3 or 2 values to represent the number of seeds inside of a rake
     * in the gridded seed generation mode.
     */
    long GetRandomNumOfSeeds() const;
    void SetRandomNumOfSeeds(long);

    /*
     * The number of seeds in the random seed generation mode.
     */
    std::string GetRakeBiasVariable() const;
    void        SetRakeBiasVariable(const std::string &);

    long GetRakeBiasStrength() const;
    void SetRakeBiasStrength(long);

    int  GetPastNumOfTimeSteps() const;
    void SetPastNumOfTimeSteps(int);

    int  GetSeedInjInterval() const;
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

    static const std::string RenderTypeTag;
    static const std::string RenderRadiusBaseTag;
    static const std::string RenderRadiusScalarTag;
    static const std::string RenderGeom3DTag;
    static const std::string RenderLightAtCameraTag;
    static const std::string RenderShowStreamDirTag;

    static const std::string RenderGlyphTypeTag;
    static const std::string RenderGlyphStrideTag;
    static const std::string RenderGlyphOnlyLeadingTag;

    static const std::string RenderDensityFalloffTag;
    static const std::string RenderDensityToneMappingTag;

    static const std::string RenderFadeTailTag;
    static const std::string RenderFadeTailStartTag;
    static const std::string RenderFadeTailStopTag;
    static const std::string RenderFadeTailLengthTag;

    static const std::string PhongAmbientTag;
    static const std::string PhongDiffuseTag;
    static const std::string PhongSpecularTag;
    static const std::string PhongShininessTag;

    static const std::string _isSteadyTag;
    static const std::string _velocityMultiplierTag;
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
