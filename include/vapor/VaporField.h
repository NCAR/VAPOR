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
class VaporField : public Field
{
public:
    VaporField( size_t cacheLimit );

    //
    // Functions from class Field
    //
    virtual bool InsideVolumeVelocity( float time, const glm::vec3& pos );
    virtual bool InsideVolumeScalar(   float time, const glm::vec3& pos );
    virtual int  GetVelocity(  float time, const glm::vec3& pos,     // input 
                               glm::vec3& vel ,                      // output
                               bool checkInsideVolume = true );
    virtual int  GetScalar(    float time, const glm::vec3& pos,     // input 
                               float& val,                           // output
                               bool checkInsideVolume = true );
    virtual int  GetNumberOfTimesteps() ;

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
    // This wrapper class wraps a grid and a data manager pointer to ensure
    // the grid is properly destroyed.
    //
    class GridWrapper
    {
    private:
        const VAPoR::Grid*  gridPtr;
        VAPoR::DataMgr*     mgr;
    public:
        GridWrapper( const VAPoR::Grid* gp, VAPoR::DataMgr* mp )
            : gridPtr( gp ), mgr( mp )
        {}   
        // Rule of 3
        GridWrapper(            const GridWrapper& ) = delete;
        GridWrapper& operator=( const GridWrapper& ) = delete;
       ~GridWrapper()
        {
            if( mgr && gridPtr )
            {
                mgr->UnlockGrid( gridPtr );
                delete gridPtr;
            }
        }
    };

    // 
    // Returns the intersection domain of 3 velocity variables
    //
    void GetFirstStepVelocityIntersection( glm::vec3& minxyz, glm::vec3& maxxyz );

protected:
   
    // Member variables
    std::vector<float>          _timestamps;    // in ascending order
    VAPoR::DataMgr*             _datamgr = nullptr;   
    const VAPoR::FlowParams*    _params  = nullptr;

    // Keep copies of recent grids.
    using cacheType = VAPoR::unique_ptr_cache< std::string, VAPoR::Grid >;
    cacheType                   _recentGrids;

    // Member functions
    template< typename T > 
    size_t _binarySearch( const std::vector<T>& vec, T val, size_t begin, size_t end ) const;

    std::string _paramsToString(  size_t currentTS, const std::string& var, int refLevel, 
            int compLevel, const std::vector<double>& min, const std::vector<double>& max ) const;

    // Are the following member variables pointers set?
    //  1) _datamgr and 2) _params
    bool _isReady() const;

    // _getAGrid will use _params to retrieve/generate grids. 
    int  _getAGrid( size_t              timestep,   // Input
                    std::string&        varName,    // Input
                    const VAPoR::Grid** gridpp  );  // Output
};
};

#endif
