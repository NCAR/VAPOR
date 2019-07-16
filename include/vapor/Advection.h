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

    //
    // Major action function
    //
    // Advect one step as long as the particle is within spatial and temporal boundary
    int  AdvectOneStep(  Field* velocityField, float deltaT, 
                         ADVECTION_METHOD method = ADVECTION_METHOD::RK4 );
    // Advect as many steps as necessary to reach a certain time: targetT.
    int  AdvectTillTime( Field* velocityField, float deltaT, float targetT,
                         ADVECTION_METHOD method = ADVECTION_METHOD::RK4 );
    // Retrieve field values of a particle based on its location, and put the result in
    // the "value" field or the "properties" field of a particle
    //   If "skipNonZero" is true, then this function only overwrites zeros.
    //   Otherwise, it will overwrite values anyway.
    int CalculateParticleValues(     Field* scalarField, bool skipNonZero );
    int CalculateParticleProperties( Field* scalarField  );

    // Reset all particle values to zero
    void ResetParticleValues( );
    // Clear all existing properties of a particle
    void ClearParticleProperties( );

    // Set advection basics
    void UseSeedParticles( const std::vector<Particle>& seeds );

    // Retrieve the resulting particles as "streams."
    size_t GetNumberOfStreams() const;
    const  std::vector<Particle>& GetStreamAt( size_t i ) const;

    // Retrieve the maximum number of steps
    size_t GetMaxNumOfSteps() const;

    //
    // Output a file that could be plotted by gnuplot
    //   Command:  splot "filename" u 1:2:3 w lines
    //   Tutorial: http://lowrank.net/gnuplot/datafile-e.html
    // Input seed points from a CVS file. 
    //   This CVS file should follow gnuplot conventions, meaning:
    //   - lines starting with # are treated as comments
    //   - empty lines are omitted
    //   - each line should have at least three columns for X, Y, Z position.
    //     An optional 4th column is used to indicate time.
    //     The rest columns are omitted.
    //
    int  OutputStreamsGnuplot( const std::string& filename, bool append = false ) const;
    int  InputStreamsGnuplot(  const std::string& filename );

    // Query properties (most are properties of the velocity field)
    int  CheckReady() const;

    // Specify periodicity on each dimension
    void  SetXPeriodicity( bool, float min = 0.0f, float max = 0.0f );
    void  SetYPeriodicity( bool, float min = 0.0f, float max = 0.0f );
    void  SetZPeriodicity( bool, float min = 0.0f, float max = 0.0f );

private:
    std::vector< std::vector<Particle> >    _streams;
    const float _lowerAngle,    _upperAngle;    // Thresholds for step size adjustment
    float       _lowerAngleCos, _upperAngleCos; // Cosine values of the threshold angles
    std::vector<size_t>         _separatorCount;// how many separators does each stream have.
                                                // This is used to know how many steps are there in a stream.

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

    // If the advection is performed in a periodic fashion along one or more dimensions.
    // These variables are **not** intended to be decided by Advection, but by someone
    // who's more knowledgeable about the field.
    bool        _isPeriodic[3];         // is it periodic in X, Y, Z dimensions ?
    glm::vec2   _periodicBounds[3];     // periodic boundaries in X, Y, Z dimensions

    // Adjust input "val" according to the bound specified by min and max.
    // Returns the value after adjustment.
    float       _applyPeriodic( float val, float min, float max ) const;

};
};

#endif
