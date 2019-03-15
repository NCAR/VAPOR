/*
 * A derived Field that properly retrieves an unsteady field from VAPOR.
 */

#ifndef UNSTEADYVAPORFIELD_H
#define UNSTEADYVAPORFIELD_H

#include "vapor/VelocityField.h"
#include "vapor/Grid.h"

namespace flow
{
class UnsteadyVAPORField : public VelocityField
{

// Define a few alias
using VGrid = VAPoR::Grid;

public:
    UnsteadyVAPORField();
   ~UnsteadyVAPORField();

    int  GetVelocity(   float time, const glm::vec3& pos, glm::vec3& vel ) const;
    bool InsideVolume(  float time, const glm::vec3& pos ) const;    
    //
    // Note: if time falls in between two time steps, the extents of the nearest 
    // time step will be returned.
    //
    int  GetExtents(    float time, glm::vec3& minExt, glm::vec3& maxExt ) const;

    //
    // Modifiers
    //
    int  AddTimeStep( const VGrid* u, const VGrid* v, const VGrid* w, float time );

    // Clean up 
    void DestroyGrids();

    //
    // Find one index whose timestamp is just below a given time
    // I.e., _timestamps[floor] <= time 
    //
    int LocateTimestamp( float   time,                        // Input
                         size_t& floor ) const;               // Output
private:
    // 
    // These vectors keep grids for all time steps, 
    // and the user time stamp these grids represent
    //
    std::vector<const VGrid*>   _velArrU;
    std::vector<const VGrid*>   _velArrV;
    std::vector<const VGrid*>   _velArrW;
    std::vector<float>          _timestamps;    // always in ascending order

    template< typename T >
    size_t _binarySearch( const std::vector<T>& vec, T val, size_t begin, size_t end ) const;
};
};

#endif
