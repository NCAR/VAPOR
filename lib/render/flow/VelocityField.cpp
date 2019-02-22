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
VelocityField::insideField( const glm::vec3& pos )
{
    glm::bvec3 tooSmall = glm::lessThan(    pos, _fieldMin );
    glm::bvec3 tooLarge = glm::greaterThan( pos, _fieldMax );
    if( glm::any( tooSmall ) || glm::any( tooLarge ) )
        return false;
    else
        return true;
}

template <class T>
T VelocityField::lerp( const T& v1, const T& v2, float a )
{
    return glm::mix( v1, v2, a );
}
