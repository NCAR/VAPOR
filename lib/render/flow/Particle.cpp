#include "Particle.h"

using namespace VAPoR::flow;

Particle::Particle()
{
    propertiesF.resize( 0 );    // memory considerations.
}

Particle::Particle( const glm::vec3& loc )
{
    _location = loc;
    _propertiesF.resize( 0 );
}

Particle::Particle( const float* loc )
{
    _location.x = loc[0];
    _location.y = loc[1];
    _location.z = loc[2];
    _propertiesF.resize( 0 );
}

Particle::Particle( float x, float y, float z )
{
    _location.x = x;
    _location.y = y;
    _location.z = z;
    _propertiesF.resize( 0 );
}

Particle::~Particle( )
{ }
