/*
 * A derived Field that properly retrieves a steady field from VAPOR.
 */

#ifndef UNSTEADYVAPORScalar_H
#define UNSTEADYVAPORScalar_H

#include "vapor/ScalarField.h"
#include "vapor/Grid.h"

namespace flow
{
class UnsteadyVAPORScalar : public ScalarField
{

// Define a few alias
using VGrid = VAPoR::Grid;

public:
    UnsteadyVAPORScalar();
   ~UnsteadyVAPORScalar();

    //
    // Retrieve scalar or field value
    //
    bool InsideVolume(  float time, const glm::vec3& pos ) const;    
    int  GetExtents(    float time, glm::vec3& minExt, glm::vec3& maxExt ) const;
    int  GetScalar(     float time, const glm::vec3& pos,   // input 
                        float& val) const;                  // output
    int  GetNumberOfTimesteps() const ;

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
