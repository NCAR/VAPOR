#ifndef VAPORFIELD_H
#define VAPORFIELD_H

#include <list>
#include "vapor/Field.h"
#include "vapor/Particle.h"
#include "vapor/DataMgr.h"
#include "vapor/FlowParams.h"
#include "vapor/Grid.h"
#include "vapor/unique_ptr_cache.hpp"

namespace flow {
class FLOW_API VaporField final : public Field {
public:
    VaporField(size_t cache_limit);

    //
    // Functions from class Field
    //
    virtual bool InsideVolumeVelocity(float time, const glm::vec3 &pos) const override;
    virtual bool InsideVolumeScalar(float time, const glm::vec3 &pos) const override;
    virtual int  GetVelocity(float time, const glm::vec3 &pos,    // input
                             glm::vec3 &vel,                      // output
                             bool       checkInsideVolume = true) const override;
    virtual int  GetScalar(float time, const glm::vec3 &pos,    // input
                           float &scalar,                       // output
                           bool   checkInsideVolume = true) const override;
    virtual int  GetNumberOfTimesteps() const override;

    //
    // Functions for interaction with VAPOR components
    //
    void AssignDataManager(VAPoR::DataMgr *dmgr);
    void UpdateParams(const VAPoR::FlowParams *p);

    //
    // Find one index whose timestamp is just below a given time
    // I.e., _timestamps[floor] <= time
    //
    int LocateTimestamp(float   time,            // Input
                        size_t &floor) const;    // Output

    //
    // This wrapper class wraps a grid and a data manager pointer to ensure
    // the grid is properly destroyed.
    //
    class FLOW_API GridWrapper final {
    private:
        const VAPoR::Grid *const gridPtr;
        VAPoR::DataMgr *const    mgr;    // The pointer itself cannot be changed
    public:
        GridWrapper(const VAPoR::Grid *gp, VAPoR::DataMgr *mp) : gridPtr(gp), mgr(mp) {}
        // Rule of five
        GridWrapper(const GridWrapper &) = delete;
        GridWrapper &operator=(const GridWrapper &) = delete;
        GridWrapper(const GridWrapper &&) = delete;
        GridWrapper &operator=(const GridWrapper &&) = delete;
        ~GridWrapper()
        {
            if (mgr && gridPtr) {
                mgr->UnlockGrid(gridPtr);
                delete gridPtr;
            }
        }

        const VAPoR::Grid *grid() const { return gridPtr; }
    };

    //
    // Returns the intersection domain of 3 velocity variables
    //
    void GetFirstStepVelocityIntersection(glm::vec3 &minxyz, glm::vec3 &maxxyz);

private:
    // Member variables
    std::vector<float>       _timestamps;    // in ascending order
    VAPoR::DataMgr *         _datamgr = nullptr;
    const VAPoR::FlowParams *_params = nullptr;
    using cacheType = VAPoR::unique_ptr_cache<std::string, GridWrapper>;
    mutable cacheType _recentGrids;

    // Member functions
    std::string _paramsToString(size_t currentTS, const std::string &var, int refLevel, int compLevel, const std::vector<double> &min, const std::vector<double> &max) const;

    // Are the following member pointers set? 1) _datamgr, 2) _params
    bool _isReady() const;

    // _getAGrid will use _params to retrieve/generate grids.
    // In the case of failing to generate a requested grid, nullptr will be returned.
    // This failure will also be recorded to MyBase.
    const VAPoR::Grid *_getAGrid(size_t timestep, const std::string &varName) const;
};
};    // namespace flow

#endif
