#include "Particle.h"

using namespace VAPoR::flow;

Particle::Particle()
{
    propertiesF.resize( 0 );    // memory considerations.
}

Particle::Particle( const glm::vec3& loc )
{
    location = loc;
    propertiesF.resize( 0 );
}

Particle::Particle( const float* loc )
{
    location.x = loc[0];
    location.y = loc[1];
    location.z = loc[2];
    propertiesF.resize( 0 );
}

Particle::Particle( float x, float y, float z )
{
    location.x = x;
    location.y = y;
    location.z = z;
    propertiesF.resize( 0 );
}
