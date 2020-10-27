#include "vapor/Particle.h"

using namespace flow;

Particle::Particle( const glm::vec3& loc, float t, float val )
{
    location = loc;
    time     = t;
    value    = val;
}

Particle::Particle( const float* loc, float t, float val )
{
    location.x = loc[0];
    location.y = loc[1];
    location.z = loc[2];
    time       = t;
    value      = val;
}

Particle::Particle( float x, float y, float z, float t, float val )
{
    location.x = x;
    location.y = y;
    location.z = z;
    time       = t;
    value      = val;
}

void Particle::AttachProperty( float v )
{
    auto before_itr = _properties.cbefore_begin();  
    for( auto itr = _properties.cbegin(); itr != _properties.cend(); ++itr )
        ++before_itr;

    _properties.insert_after( before_itr, v );
    _property_size++;
}

auto Particle::GetPropertyList() const -> std::pair< const std::forward_list<float>&, uint32_t >
{
    return {_properties, _property_size};
}

void Particle::ClearProperty()
{
    _properties.clear();
    _property_size = 0;
}
    
void Particle::RemoveProperty( size_t target_i )
{
    size_t current_i = 0;
    auto before_it = _properties.cbefore_begin();
    for( auto it = _properties.cbegin(); it != _properties.cend(); ++it ) {
        if( current_i == target_i ){
            _properties.erase_after( before_it );
            _property_size--;
            break;
        }
        ++current_i;
        ++before_it;
    }
}

void Particle::SetSpecial( bool isSpecial )
{
    // Give both "time" and "value" a nan to indicate the "special state."
    // Accidental assignment of nan to one of the two variables would not 
    // render a "special state."
    if( isSpecial ) {
        time  = std::nanf("1");
        value = std::nanf("1");
    }
    else {
        time  = 0.0f;
        value = 0.0f;
    }
}

bool Particle::IsSpecial() const
{
    return (std::isnan(time) && std::isnan(value));
}
