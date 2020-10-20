#ifndef VAPORFIELD_H
#define VAPORFIELD_H

#include <list>
#include "vapor/Field.h"
#include "vapor/Particle.h"
#include "vapor/DataMgr.h"
#include "vapor/FlowParams.h"
#include "vapor/Grid.h"
#include "vapor/unique_ptr_cache.hpp"

namespace flow
{

//
// Helper class: it is used to identify a specific grid.
//
class FLOW_API GridKey final
{
private:
    std::string varName; 

    // Buffer definition:
    // Offset   Data_Type   Information
    //
    // 0        uint32_t    currentTS
    // 4        int32_t     refLevel
    // 8        int32_t     compLevel
    // 12       double[3]   extent_min
    // 36       double[3]   extent_max
    // 60       float       DefaultZ
    //
    // Total size: 64
    std::array<uint8_t, 64> buf;

public:
    void Reset( uint32_t, int32_t, int32_t, std::string, const std::vector<double>&, 
                const std::vector<double>&, float );

    bool emptyVar() const;

    // == operator to be used in the cache
    bool operator==(const GridKey& ) const;
};


// 
// Helper class: it wraps a grid and a data manager pointer to ensure
// the grid is properly destroyed.
//
class FLOW_API GridWrapper final
{
private:
    const VAPoR::Grid*  const  gridPtr;
    VAPoR::DataMgr*     const  mgr;     // The pointer itself cannot be changed
public:
    GridWrapper( const VAPoR::Grid* gp, VAPoR::DataMgr* mp );
    // Rule of five
    GridWrapper(            const GridWrapper&  ) = delete;
    GridWrapper& operator=( const GridWrapper&  ) = delete;
    GridWrapper(            const GridWrapper&& ) = delete;
    GridWrapper& operator=( const GridWrapper&& ) = delete;
   ~GridWrapper();
    const VAPoR::Grid* grid() const;
};


//
//  Note on variable names in a VaporField:
//  If a variable name is an empty string, then this variable is still valid, 
//  but contains all zero values in it.
//
class FLOW_API VaporField final : public Field
{
public:
    VaporField( size_t cache_limit );

    //
    // Functions from class Field
    //
    virtual bool InsideVolumeVelocity( float time, const glm::vec3& pos ) const override;
    virtual bool InsideVolumeScalar(   float time, const glm::vec3& pos ) const override;
    virtual int  GetNumberOfTimesteps()                                   const override;

    virtual int  GetVelocity(  float time, const glm::vec3& pos,    // input 
                               glm::vec3& vel ) const override;     // output
    virtual int  GetScalar(    float time, const glm::vec3& pos,    // input 
                               float& scalar ) const override;      // output


    //
    // Functions for interaction with VAPOR components
    //
    void AssignDataManager( VAPoR::DataMgr*    dmgr );
    void UpdateParams(const VAPoR::FlowParams* p );

    //  
    // Find one index whose timestamp is just below a given time
    // I.e., _timestamps[floor] <= time 
    //  
    int LocateTimestamp( float   time,                        // Input
                         size_t& floor ) const;               // Output

    // 
    // Returns the intersection domain of 3 velocity variables at a specific time step.
    // It returns non-zeros upon failure.
    //
    int GetVelocityIntersection( size_t ts, glm::vec3& minxyz, glm::vec3& maxxyz ) const;

    // 
    // Store the default Z value for variables that are 2D grids in nature.
    // In this case, a 3D one-layer "GrownGrid" is created with 
    // the 3rd dimension being DefaultZ.
    //
    float DefaultZ = 0.0f;

    //
    // Calculate a reasonable deltaT based on the velocity speed and domain size.
    // This is used as a one-time operation when getting ready a velocity field.
    // It'll return 0 on success, and non-zero on error conditions.
    //
    int  CalcDeltaTFromCurrentTimeStep( float& delT ) const;

    //
    // It turns that interactions from FlowParams can be expensive.
    // Let's implement a mechanism to cache information and interact with them less frequently.
    // Specifically, the following two functions are used to `lock` into (and `unlock` from) 
    // the current set of params from FlowParams, so we don't need to interact with it anymore.
    //
    virtual auto LockParams()   -> int override;
    virtual auto UnlockParams() -> int override;


private:

    //
    // Member variables
    //
    std::vector<float>          _timestamps;    // in ascending order
    VAPoR::DataMgr*             _datamgr = nullptr;   
    const VAPoR::FlowParams*    _params  = nullptr;

    using cacheType = VAPoR::unique_ptr_cache< GridKey, GridWrapper >;
    mutable cacheType           _recentGrids;   // so this variable can be 
                                                // modified by a const function.
    mutable std::mutex          _grid_operation_mutex;  // Use `mutable` qualifier so this
                                                        // mutex can be used in const methods.

    // The following variables are cache states from DataMgr and Params.
    bool                        _params_locked = false;
    uint32_t                    _c_currentTS = 0;                // cached timestep
    int32_t                     _c_refLev = -2, _c_compLev = -2; // cached ref/comp levels
    float                       _c_vel_mult = 0.0;               // cached velocity multiplier
    std::vector<double>         _c_ext_min, _c_ext_max;          // cached extents

    //
    // Member functions
    //

    // Are the following member pointers correctly set? 
    //   1) _datamgr, 2) _params
    bool _isReady() const;

    // _getAGrid will use _params to retrieve/generate grids. 
    // In the case of failing to generate a requested grid, nullptr will be returned.
    // This failure will also be recorded to MyBase.
    // Note 1: If a variable name is empty, we then return a ConstantField.
    // Note 2: If a variable is essentially 2D, we then grow it to be 3D 
    //         and return a GrownGrid.
    const VAPoR::Grid* _getAGrid( size_t              timestep,
                                  const std::string&  varName ) const ;

};
};

#endif
