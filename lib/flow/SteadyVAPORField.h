/*
 * A derived VelocityField that properly retrieves a steady field from VAPOR.
 */

#ifndef STEADYVAPORFIELD_H
#define STEADYVAPORFIELD_H

#include "VelocityField.h"
#include "vapor/StructuredGrid.h"
#include <unordered_map>

namespace flow
{
class SteadyVAPORField : public VelocityField
{

// Define a few alias
using VGrid = VAPoR::Grid;

public:
    SteadyVAPORField();
   ~SteadyVAPORField();

    int  Get( float time, const glm::vec3& pos, glm::vec3& vel ) const;
    bool InsideField(     float time, const glm::vec3& pos ) const;    

    //
    // Modifiers
    //
    void UseVelocityField( const VGrid* u, const VGrid* v, const VGrid* w );
    void UseValueField(    const VGrid* val );

private:
    // 
    // These variables keep the steady grids
    //
    const VGrid* _velocityU;
    const VGrid* _velocityV;
    const VGrid* _velocityW;
    const VGrid* _value;

    //
    // Detect if the current field is ready for operations (e.g., Get() )
    //
    bool  _isReady() const;
};

};

#endif
