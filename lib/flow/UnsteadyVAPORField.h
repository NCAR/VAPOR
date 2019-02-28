/*
 * A derived VelocityField that properly retrieves an unsteady field from VAPOR.
 */

#ifndef UNSTEADYVAPORFIELD_H
#define UNSTEADYVAPORFIELD_H

#include "VelocityField.h"
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

    int  GetVelocity( float time, const glm::vec3& pos, glm::vec3& vel ) const;
    bool InsideVelocityField(     float time, const glm::vec3& pos ) const;    

    //
    // Modifiers
    //
    int  AddTimeStep( const VGrid* u, const VGrid* v, const VGrid* w,
                      const VGrid* value, float time );

     // 
     // Since the grids are passed in, SteadyVAPORField does NOT destroy them by default.
     // However, SteadyVAPORField could perform this task if desired.
     //
     void DestroyGrids();


private:
    // 
    // These vectors keep grids for all time steps, 
    // and the user time stamp these grids represent
    //
    std::vector<const VGrid*>   _velArrU;
    std::vector<const VGrid*>   _velArrV;
    std::vector<const VGrid*>   _velArrW;
    std::vector<const VGrid*>   _valueArr;
    std::vector<float>          _timestamps;    // always in ascending order

    //
    // Find one index whose timestamp is just below a given time
    // I.e., _timestamps[floor] <= time 
    //
    int _locateTimestamp( float   time,                        // Input
                          size_t& floor ) const;               // Output

    template< typename T >
    size_t _binarySearch( const std::vector<T>& vec, T val, size_t begin, size_t end ) const;

};

};

#endif
