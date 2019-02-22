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
Advection::useVelocityField( VelocityField* p )
{
    _vField = p;
}

void
Advection::useSeedParticles( std::vector<Particle>& seeds )
{
    _streams.clear();
    _streams.resize( seeds.size() );
    for( size_t i = 0; i < seeds.size(); i++ )
        _streams[i].push_back( seeds[i] );
}
