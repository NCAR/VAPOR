/*
 * A derived Field that properly retrieves a steady field from VAPOR.
 */

#ifndef STEADYVAPORScalar_H
#define STEADYVAPORScalar_H

#include "vapor/ScalarField.h"
#include "vapor/Grid.h"

namespace flow
{
class SteadyVAPORScalar : public ScalarField
{

// Define a few alias
using VGrid = VAPoR::Grid;

public:
    SteadyVAPORScalar();
   ~SteadyVAPORScalar();

    //
    // Retrieve scalar or field value
    //
    bool InsideVolume(  float time, const glm::vec3& pos ) const;    
    int  GetExtents(    float time, glm::vec3& minExt, glm::vec3& maxExt ) const;
    int  GetScalar(     float time, const glm::vec3& pos,   // input 
                        float& val) const = 0;              // output

    //
    // Modifiers
    //
    void UseGrid( const VGrid* g );

private:
    // 
    // These variables keep the steady grids
    //
    const VGrid* _grid;
};

};

#endif
