/*
 * This class performances advection calculations.
 */

#ifndef ADVECTION_H
#define ADVECTION_H

#include "VelocityField.h"
#include "Particle.h"
#include <string>

namespace flow
{
class Advection
{
public:
    // Constructor and destructor
    Advection();
   ~Advection();

    int advectEuler( float deltaT );
    int advectRK4(   float deltaT );

    void useVelocityField( VelocityField* p );
    void useSeedParticles( std::vector<Particle>& seeds );

private:
    VelocityField*                          _vField;

    std::vector< std::vector<Particle> >    _streams;
};

};

#endif
