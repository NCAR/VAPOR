/*
 * This class performances advection calculations.
 * It also holds all the particles resulting from an advection.
 */

#ifndef ADVECTION_H
#define ADVECTION_H

#include "vapor/Particle.h"
#include "vapor/Field.h"
#include <string>
#include <vector>

namespace flow
{
class Advection
{
public:
    enum class ADVECTION_METHOD
    {
        EULER =  0,
        RK4   =  1      // Runge-Kutta 4th order
    };

    // Constructor and destructor
    Advection();
   ~Advection();

    // Major action function
    int  Advect( Field* velocityField, float deltaT, 
                 ADVECTION_METHOD method = ADVECTION_METHOD::RK4 );

    // TODO: this function needs to be thought through.
    int CalculateParticleProperty( const Field* scalarField );

    // Set advection basics
    //void SetBaseStepSize(  float deltaT );
    //void UseVelocity( const VelocityField* p );
    void UseSeedParticles( std::vector<Particle>& seeds );

    // Retrieve the resulting particles as "streams."
    size_t GetNumberOfStreams() const;
    const  std::vector<Particle>& GetStreamAt( size_t i ) const;
    float  GetLatestAdvectionTime() const;

    //int    GetNumberOfTimesteps() const;

    // As part of the functionality of this class, it manages the 
    //    particles that it stores.
    //
    // 1) this function assigns values to the Particle.value field for an entire stream
    //int  AssignParticleValuesOfAStream( std::vector<float>& values, size_t idx );
    //
    // 2) this function assigns a value to the last Particle of a stream
    //int  AssignLastParticleValueOfAStream( float value, size_t idx );
    //
    // 3) this function copies the value field from second last particle to the last one
    //int  RepeatLastTwoParticleValuesOfAStream( size_t idx );
    //
    // 4) this function calls Particle::AttachProperty() function for a stream
    //int  AttachParticlePropertiesOfAStream( std::vector<float>& properties, size_t idx );
    //
    // 5) this function calls Particle::ClearProperties() function
    void ClearParticleProperties( );

    //
    // Output a file that could be plotted by gnuplot
    //   Command:  splot "filename" u 1:2:3 w lines
    //   Tutorial: http://lowrank.net/gnuplot/datafile-e.html
    //
    int  OutputStreamsGnuplot( const std::string& filename ) const;

    // Query properties (most are properties of the velocity field)
    int  CheckReady() const;
    //bool IsSteady() const;
    //bool IsAdvectionComplete() const;
    //void ToggleAdvectionComplete( bool b );
    //const std::string GetVelocityNameU() const;
    //const std::string GetVelocityNameV() const;
    //const std::string GetVelocityNameW() const;

private:
    //const VelocityField*                    _velocity;
    std::vector< std::vector<Particle> >    _streams;
    const float _lowerAngle,    _upperAngle;    // Thresholds for step size adjustment
    float       _lowerAngleCos, _upperAngleCos; // Cosine values of the threshold angles
    float       _latestAdvectionTime;

    // Advection methods here could assume all input is valid.
    int _advectEuler( Field*, const Particle&, float deltaT, // Input
                      Particle& p1 ) const;                  // Output
    int _advectRK4( Field*, const Particle&, float deltaT,   // Input
                    Particle& p1 ) const;                    // Output

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
