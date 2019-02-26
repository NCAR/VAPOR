#include "VelocityField.h"

using namespace flow;

VelocityField::VelocityField()
{ 
    _fieldMin = glm::vec3( 0.0f );
    _fieldMax = glm::vec3( 0.0f );
    isSteady  = true;
    isPeriodic = false;
}

VelocityField::~VelocityField()
{ }

bool
VelocityField::InsideField( float time, const glm::vec3& pos ) const
{
    glm::bvec3 tooSmall = glm::lessThan(    pos, _fieldMin );
    glm::bvec3 tooLarge = glm::greaterThan( pos, _fieldMax );
    if( glm::any( tooSmall ) || glm::any( tooLarge ) )
        return false;
    else
        return true;
}
