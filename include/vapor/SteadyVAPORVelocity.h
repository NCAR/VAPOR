/*
 * A derived Field that properly retrieves a steady field from VAPOR.
 */

#ifndef STEADYVAPORVelocity_H
#define STEADYVAPORVelocity_H

#include "vapor/VelocityField.h"
#include "vapor/Grid.h"

namespace flow
{
class SteadyVAPORVelocity : public VelocityField
{

// Define a few alias
using VGrid = VAPoR::Grid;

public:
    SteadyVAPORVelocity();
   ~SteadyVAPORVelocity();

    //
    // Retrieve velocity or field value
    //
    int  GetVelocity(   float time, const glm::vec3& pos, glm::vec3& vel ) const;
    bool InsideVolume(  float time, const glm::vec3& pos ) const;    
    int  GetExtents(    float time, glm::vec3& minExt, glm::vec3& maxExt ) const;

    //
    // Modifiers
    //
    void UseVelocities( const VGrid* u, const VGrid* v, const VGrid* w );

    // 
    // Since the grids are passed in, SteadyVAPORField does NOT destroy them by default.
    // However, SteadyVAPORField could perform this task if desired.
    //
    void DestroyGrids();

private:
    // 
    // These variables keep the steady grids
    //
    const VGrid* _velocityU;
    const VGrid* _velocityV;
    const VGrid* _velocityW;
};

};

#endif
