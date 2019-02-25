/*
 * This class performances advection calculations.
 */

#ifndef ADVECTION_H
#define ADVECTION_H

#include "VelocityField.h"
#include "Particle.h"
#include <string>
#include <vector>

namespace flow
{
class Advection
{
public:
    enum ADVECTION_METHOD
    {
        EULER =  0,
        RK4   =  1
    };

    // Constructor and destructor
    Advection();
   ~Advection();

    // Note: deltaT could be positive or negative!
    int Advect( float deltaT, ADVECTION_METHOD method = EULER );

    void UseVelocityField( VelocityField* p );
    void UseSeedParticles( std::vector<Particle>& seeds );

    //
    // Output a file that could be plotted by gnuplot
    //   Command:  splot "filename" u 1:2:3 w lines
    //   Tutorial: http://lowrank.net/gnuplot/datafile-e.html
    //
    int  OutputStreamsGnuplot( const std::string& filename ) const;

private:
    const VelocityField*                    _vField;
    std::vector< std::vector<Particle> >    _streams;

    int _readyToAdvect() const;

    // Advection methods here could assume all input is valid.
    int _advectEuler( const Particle& p0, float deltaT, // Input
                            Particle& p1 ) const;       // Output
    int _advectRK4(   const Particle& p0, float deltaT, // Input
                            Particle& p1 ) const;       // Output
};

};

#endif
