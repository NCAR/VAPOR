#ifndef VAPORFIELD_H
#define VAPORFIELD_H

#include "vapor/Field.h"
#include "vapor/Particle.h"
#include "vapor/DataMgr.h"
#include "vapor/FlowParams.h"
#include "vapor/Grid.h"
#include "vapor/ptr_cache.hpp"

namespace flow {

//
// Helper class: it is used to identify a specific grid.
//
class FLOW_API GridKey {
private:
    uint32_t              _timestep = std::numeric_limits<uint32_t>::max(); // almost impossible value
    int32_t               _refLev = -2;   // Impossible value
    int32_t               _compLev = -2;  // Impossible value
    std::string           _varName;
    VAPoR::CoordType      _ext_min = {0.0, 0.0, 0.0};
    VAPoR::CoordType      _ext_max = {0.0, 0.0, 0.0};

public:
    void Reset(uint32_t, int32_t, int32_t, std::string, VAPoR::CoordType, VAPoR::CoordType);

    bool emptyVar() const;

    // == operator to be used in the cache
    bool operator==(const GridKey &) const;
};

//
// Helper class: it wraps a grid and a data manager pointer to ensure
// the grid is properly destroyed.
//
class FLOW_API GridWrapper final {
private:
    const VAPoR::Grid *const _gridPtr;
    VAPoR::DataMgr *const    _mgr;    // The pointer itself cannot be changed

public:
    GridWrapper(const VAPoR::Grid *gp, VAPoR::DataMgr *mp);
    // Rule of five
    GridWrapper(const GridWrapper &) = delete;
    GridWrapper(const GridWrapper &&) = delete;
    GridWrapper &operator=(const GridWrapper &) = delete;
    GridWrapper &operator=(const GridWrapper &&) = delete;
    ~GridWrapper();

    // Access the real pointer
    const VAPoR::Grid *grid() const;
};

//
//  Note on variable names in a VaporField:
//  If a variable name is an empty string, then this variable is still valid,
//  but contains all zero values in it.
//
class FLOW_API VaporField final : public Field {
public:
    //
    // Functions from class Field
    //
    virtual bool InsideVolumeVelocity(double time, glm::vec3 pos) const override;
    virtual bool InsideVolumeScalar(double time, glm::vec3 pos) const override;
    virtual uint32_t  GetNumberOfTimesteps() const override;

    virtual int GetVelocity(double time, glm::vec3 pos,     // input
                            glm::vec3 &vel) const override; // output
    virtual int GetScalar(double time, glm::vec3 pos,       // input
                          float &scalar) const override;    // output

    //
    // Functions for interaction with VAPOR components
    //
    void AssignDataManager(VAPoR::DataMgr *dmgr);
    void UpdateParams(const VAPoR::FlowParams *);
    void ReleaseLockedGrids();    // Supposed to be invoked at the end of each paintGL event.

    //
    // Find one index whose timestamp is just below a given time
    // I.e., _timestamps[floor] <= time
    //
    int LocateTimestamp(double  time,            // Input
                        size_t &floor) const;    // Output

    //
    // Returns the intersection domain of 3 velocity variables at a specific time step.
    // It returns non-zeros upon failure.
    //
    int GetVelocityIntersection(size_t ts, glm::vec3 &minxyz, glm::vec3 &maxxyz) const;

    //
    // Calculate a reasonable deltaT based on the velocity speed and domain size.
    // This is used as a one-time operation when getting ready a velocity field.
    // It'll return 0 on success, and non-zero on error conditions.
    //
    int CalcDeltaTFromCurrentTimeStep(double &delT) const;

    //
    // It turns that interactions from FlowParams can be expensive.
    // Let's implement a mechanism to cache information and interact with them less frequently.
    // Specifically, the following two functions are used to `lock` into (and `unlock` from)
    // the current set of params from FlowParams, so we don't need to interact with it anymore.
    //
    virtual auto LockParams() -> int override;
    virtual auto UnlockParams() -> int override;

private:
    //
    // Member variables
    //
    std::vector<double>      _timestamps;    // in ascending order
    VAPoR::DataMgr *         _datamgr = nullptr;
    const VAPoR::FlowParams *_params = nullptr;

    using cacheType = VAPoR::ptr_cache<GridKey, GridWrapper, 6, true>;
    mutable cacheType _recentGrids;
    mutable std::mutex _grid_operation_mutex;

    // The following variables are cache states from DataMgr and Params.
    bool                               _params_locked = false;
    uint32_t                           _c_currentTS = 0;          // cached timestep
    int32_t                            _c_refLev = -2;            // cached ref levels
    int32_t                            _c_compLev = -2;           // cached ref levels
    float                              _c_vel_mult = 0.0f;        // cached velocity multiplier
    VAPoR::CoordType                   _c_ext_min;                // cached extents
    VAPoR::CoordType                   _c_ext_max;                // cached extents

    //
    // Member functions
    //
    // Are the following member pointers correctly set?
    //   1) _datamgr, 2) _params
    bool _isReady() const;

    // _getAGrid will use _params to retrieve/generate grids.
    // In the case of failing to generate a requested grid, nullptr will be returned.
    // This failure will also be recorded to MyBase.
    // Note: If a variable name is empty, we then return a ConstantField.
    const VAPoR::Grid *_getAGrid(uint32_t timestep, const std::string &varName) const;
};
};    // namespace flow

#endif
