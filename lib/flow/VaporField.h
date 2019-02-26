/*
 * A derived VelocityField that properly retrieves from VAPOR.
 */

#ifndef VAPORFIELD_H
#define VAPORFIELD_H

#include "VelocityField.h"
#include "vapor/StructuredGrid.h"

namespace flow
{
class VaporField : public VelocityField
{

// Define a few alias
using VGrid = VAPoR::Grid;

public:
    VaporField();
   ~VaporField();

    int  Get( float time, const glm::vec3& pos, glm::vec3& vel ) const;
    bool InsideField( const glm::vec3& pos ) const;    

    void UseVelocityField( const VGrid* u, const VGrid* v, const VGrid* w );
    void UseValueField(    const VGrid* val );

private:
    const VGrid* _velocityU;
    const VGrid* _velocityV;
    const VGrid* _velocityW;
    const VGrid* _value;
};
};

#endif
