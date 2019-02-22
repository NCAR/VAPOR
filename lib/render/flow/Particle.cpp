#include "Particle.h"

using namespace flow;

Particle::Particle()
{
    time  = 0.0f;
    value = 0.0f;
}

Particle::Particle( const glm::vec3& loc, float t )
{
    location = loc;
    time     = t;
    value = 0.0f;
}

Particle::Particle( const float* loc, float t )
{
    location.x = loc[0];
    location.y = loc[1];
    location.z = loc[2];
    time       = t;
    value = 0.0f;
}

Particle::Particle( float x, float y, float z, float t )
{
    location.x = x;
    location.y = y;
    location.z = z;
    time       = t;
    value = 0.0f;
}

Particle::~Particle( )
{ }
