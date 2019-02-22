#include <cstdio>

#include "Advection.h"

using namespace flow;

Advection::Advection()
{
    _vField = nullptr;
}

Advection::~Advection()
{
    _vField = nullptr;
}

void
Advection::UseVelocityField( VelocityField* p )
{
    _vField = p;
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
Advection::_readyToAdvect() const
{
    if( _vField == nullptr )
        return NO_VECTOR_FIELD_YET;

    for( const auto& s : _streams )
    {
        if( s.size() < 1 )
            return NO_SEED_PARTICLE_YET;
    }

    return 0;
}

int
Advection::Advect( float dt, ADVECTION_METHOD method )
{
    int ready = _readyToAdvect();
    if( ready != 0 )
        return ready;

    for( auto& s : _streams )
    {
        auto& p0 = s.back();
        Particle p1;
        int rv;
        if( method == EULER )
            rv = _advectEuler( p0, dt, p1 );
        s.push_back( p1 );
    }

    return 0;
}

int
Advection::_advectEuler( const Particle& p0, float dt, Particle& p1 ) const
{
    glm::vec3 v0;
    int rv = _vField->Get( p0.time, p0.location, v0 );    
    assert( rv == 0 );
    v0 *= dt;
    p1.location = p0.location + v0;
    p1.time     = p0.time + dt;
    return 0;
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
