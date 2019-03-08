/*
 * This class performances advection calculations.
 */

#ifndef ADVECTION_H
#define ADVECTION_H

#include "vapor/VelocityField.h"
#include "vapor/Particle.h"
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
        RK4   =  1      // Runge-Kutta 4th order
    };

    // Constructor and destructor
    Advection();
   ~Advection();

    int  Advect( ADVECTION_METHOD method = RK4 );

    void SetBaseStepSize(  float deltaT );
    void UseVelocityField( const VelocityField* p );
    void UseSeedParticles( std::vector<Particle>& seeds );

    size_t GetNumberOfStreams() const;
    const std::vector<Particle>& GetStreamAt( size_t i ) const;

    //
    // Output a file that could be plotted by gnuplot
    //   Command:  splot "filename" u 1:2:3 w lines
    //   Tutorial: http://lowrank.net/gnuplot/datafile-e.html
    //
    int OutputStreamsGnuplot( const std::string& filename ) const;

    int IsReady() const;

private:
    const VelocityField*                    _vField;
    std::vector< std::vector<Particle> >    _streams;
    float   _lowerAngle,    _upperAngle;    // Thresholds for step size adjustment
    float   _lowerAngleCos, _upperAngleCos; // Cosine values of the threshold angles
    float   _baseDeltaT;

    // Advection methods here could assume all input is valid.
    int _advectEuler( const Particle& p0, float deltaT, // Input
                            Particle& p1 ) const;       // Output
    int _advectRK4(   const Particle& p0, float deltaT, // Input
                            Particle& p1 ) const;       // Output

    // Get an adjust factor for deltaT based on how curvy the past two steps are.
    //   A value in range (0.0, 1.0) means shrink deltaT.
    //   A value in range (1.0, inf) means enlarge deltaT.
    //   A value equals to 1.0 means not touching deltaT.
    float _calcAdjustFactor( const Particle& past2, 
                             const Particle& past1, 
                             const Particle& current ) const;
};

};

#endif
