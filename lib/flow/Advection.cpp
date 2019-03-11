#include <cstdio>

#include "vapor/Advection.h"

using namespace flow;

// Constructor;
Advection::Advection()
{
    _field      = nullptr;
    _baseDeltaT = 0.01f;
    _lowerAngle = 3.0f;
    _upperAngle = 15.0f;
    _lowerAngleCos = glm::cos( glm::radians( _lowerAngle ) );
    _upperAngleCos = glm::cos( glm::radians( _upperAngle ) );
}

// Destructor;
Advection::~Advection()
{
    if( _field )
    {
        delete _field;
        _field = nullptr;
    }
}

void
Advection::SetBaseStepSize( float f )
{
    _baseDeltaT = f;
}

void
Advection::UseVelocity( const VelocityField* p )
{
    if( _field )
        delete _field;
    _field = p;
}

void
Advection::UseSeedParticles( std::vector<Particle>& seeds )
{
    _streams.clear();
    _streams.resize( seeds.size() );
    for( size_t i = 0; i < seeds.size(); i++ )
        _streams[i].push_back( seeds[i] );
}

int
Advection::IsReady() const
{
    if( _field == nullptr )
        return NO_FIELD_YET;

    for( const auto& s : _streams )
    {
        if( s.size() < 1 )
            return NO_SEED_PARTICLE_YET;
    }

    return 0;
}

bool
Advection::IsSteady() const
{
    if( _field )
        return _field->IsSteady;
    else
        return false;
}
/*
bool
Advection::HasScalarValue() const
{
    if( _field )
        return _field->HasScalarValue;
    else
        return false;
} 
*/
const std::string 
Advection::GetVelocityNameU() const
{
    if( _field )
        return _field->VelocityNameU;
    else
        return std::string("");
}
const std::string 
Advection::GetVelocityNameV() const
{
    if( _field )
        return _field->VelocityNameV;
    else
        return std::string("");
}
const std::string 
Advection::GetVelocityNameW() const
{
    if( _field )
        return _field->VelocityNameW;
    else
        return std::string("");
}
/*
const std::string 
Advection::GetScalarName() const
{
    if( _field )
        return _field->ScalarName;
    else
        return std::string("");
}
*/

int
Advection::Advect( ADVECTION_METHOD method )
{
    int ready = IsReady();
    if( ready != 0 )
        return ready;

    for( auto& s : _streams )
    {
        const auto& p0 = s.back();
        if( !_field->InsideVolume( p0.time, p0.location ) )
            continue;

        float dt = _baseDeltaT;
        // If there are at least 3 particles in the stream, we also adjust dt
        if( s.size() > 2 )
        {
            const auto& past1 = s[ s.size()-2 ];
            const auto& past2 = s[ s.size()-3 ];
            dt  = p0.time - past1.time;     // step size used by last integration
            dt *= _calcAdjustFactor( past2, past1, p0 );
        }

        Particle p1;
        int rv;
        switch (method)
        {
            case EULER:
                rv = _advectEuler( p0, dt, p1 ); break;
            case RK4:
                rv = _advectRK4(   p0, dt, p1 ); break;
        }
    
        if( rv != 0 )
            continue;
/*
        if( _field->HasScalarValue )
        {
            rv = _field->GetScalar( p1.time, p1.location, p1.value );
            if( rv != 0 )
                continue;
        }
*/

        s.push_back( p1 );
    }

    return 0;
}

int
Advection::_advectEuler( const Particle& p0, float dt, Particle& p1 ) const
{
    glm::vec3 v0;
    int rv  = _field->GetVelocity( p0.time, p0.location, v0 );    
    assert( rv == 0 );
    p1.location = p0.location + dt * v0;
    p1.time     = p0.time + dt;
    return 0;
}

int
Advection::_advectRK4( const Particle& p0, float dt, Particle& p1 ) const
{
    glm::vec3 k1, k2, k3, k4;
    float dt2 = dt * 0.5f;
    int rv;
    rv = _field->GetVelocity( p0.time,       p0.location,            k1 );
    assert( rv == 0 );
    rv = _field->GetVelocity( p0.time + dt2, p0.location + dt2 * k1, k2 );
    if( rv != 0 )
        return rv;
    rv = _field->GetVelocity( p0.time + dt2, p0.location + dt2 * k2, k3 );
    if( rv != 0 )
        return rv;
    rv = _field->GetVelocity( p0.time + dt,  p0.location + dt  * k3, k4 );
    if( rv != 0 )
        return rv;
    p1.location = p0.location + dt / 6.0f * (k1 + 2.0f * (k2 + k3) + k4 );
    p1.time     = p0.time + dt;
    return 0;
}

float
Advection::_calcAdjustFactor( const Particle& p2, const Particle& p1, 
                              const Particle& p0                    ) const
{
    glm::vec3 p2p1 = p1.location - p2.location;
    glm::vec3 p1p0 = p0.location - p1.location;
    float denominator = glm::length( p2p1 ) * glm::length( p1p0 );
    float cosine;
    if( denominator < 1e-7 )
        return 1.0f;
    else
        cosine = glm::dot( p2p1, p1p0 ) / denominator;

    if( cosine > _lowerAngleCos )       // Less than "_lowerAngle" degrees
        return 1.25f;
    else if( cosine < _upperAngleCos )  // More than "_upperAngle" degrees
        return 0.5f;
    else
        return 1.0f;
}

int
Advection::OutputStreamsGnuplot( const std::string& filename ) const
{
    FILE* f = std::fopen( filename.c_str(), "w" );
    if( f == nullptr )
        return FILE_ERROR;

    std::fprintf( f, "%s\n", "# X      Y      Z" );
    for( const auto& s : _streams )
    {
        for( const auto& p : s )
            std::fprintf( f, "%f, %f, %f\n", p.location.x, p.location.y, p.location.z );
        std::fprintf( f, "\n\n" );
    }
    std::fclose( f );
    
    return 0;
}

size_t 
Advection::GetNumberOfStreams() const
{
    return _streams.size();
}

const std::vector<Particle>&
Advection::GetStreamAt( size_t i ) const
{
    return _streams.at(i);
}
