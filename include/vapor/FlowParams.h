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

class PARAMS_API FlowParams : public RenderParams {
public:
    // Constructors
    FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave);
    FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    virtual ~FlowParams();

    //
    // (Pure virtual methods from RenderParams)
    //
    virtual bool IsOpaque() const override { return false; }
    virtual bool usingVariable(const std::string &varname) override { return false; }

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

    std::vector<bool> GetPeriodic() const;
    void              SetPeriodic(const std::vector<bool> &);

    /*
     * 6 values to represent a rake in this particular order:
     * xmin, xmax, ymin, ymax, zmin, zmax
     * If the rake wasn't set by users, it returns a vector containing nans.
     */
    std::vector<float> GetRake() const;
    void               SetRake(const std::vector<float> &);

    /*
     * 4 values to represent the number of seeds inside of a rake:
     * The first 3 values are for uniform seeding, with num of seeds in X, Y, Z direction.
     * The 4th value is for random seeding, which specifies the number of total seeds in the rake.
     */
    std::vector<long> GetRakeNumOfSeeds() const;
    void              SetRakeNumOfSeeds(const std::vector<long> &);

    std::string GetRakeBiasVariable() const;
    void        SetRakeBiasVariable(const std::string &);

    float GetRakeBiasStrength() const;
    void  SetRakeBiasStrength(float);

    int  GetPastNumOfTimeSteps() const;
    void SetPastNumOfTimeSteps(int);

    int  GetSeedInjInterval() const;
    void SetSeedInjInterval(int);

private:
    static const std::string _isSteadyTag;
    static const std::string _velocityMultiplierTag;
    static const std::string _steadyNumOfStepsTag;
    static const std::string _seedGenModeTag;
    static const std::string _seedInputFilenameTag;
    static const std::string _flowlineOutputFilenameTag;
    static const std::string _flowDirectionTag;
    static const std::string _needFlowlineOutputTag;
    static const std::string _periodicTag;
    static const std::string _rakeTag;
    static const std::string _rakeNumOfSeedsTag;
    static const std::string _rakeBiasVariable;
    static const std::string _rakeBiasStrength;
    static const std::string _pastNumOfTimeSteps;
    static const std::string _seedInjInterval;

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
