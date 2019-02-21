#include "Particle.h"

using namespace VAPoR::flow;

Particle::Particle()
{
    time = 0.0f;
}

Particle::Particle( const glm::vec3& loc, float t )
{
    location = loc;
    time     = t;
}

Particle::Particle( const float* loc, float t )
{
    location.x = loc[0];
    location.y = loc[1];
    location.z = loc[2];
    time       = t;
}

Particle::Particle( float x, float y, float z, float t )
{
    location.x = x;
    location.y = y;
    location.z = z;
    time       = t;
}

Particle::~Particle( )
{ }
