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

void
Particle::AttachProperty( float v )
{
    auto itr = _properties.cbegin();  
    while( itr != _properties.cend() )
        ++itr;
    _properties.insert_after( itr, v );
}

int
Particle::EditProperty( int idx, float v )
{
    auto itr = _properties.begin();
    for( int i = 0; i < idx; i++ )
    {
        if( itr == _properties.end() )
            return OUT_OF_RANGE;
        else
            ++itr;
    }

    if( itr == _properties.end() )
        return OUT_OF_RANGE;
    else
    {
        *itr = v;
        return 0;
    }
}

int
Particle::RetrieveProperty( int idx, float& v ) const
{
    auto itr = _properties.begin();
    for( int i = 0; i < idx; i++ )
    {
        if( itr == _properties.end() )
            return OUT_OF_RANGE;
        else
            ++itr;
    }

    if( itr == _properties.end() )
        return OUT_OF_RANGE;
    else
    {
        v = *itr;
        return 0;
    }

}
