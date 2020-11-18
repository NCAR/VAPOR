/*
 * This class performances advection calculations.
 * It also holds all the particles resulting from an advection.
 */

#ifndef ADVECTION_H
#define ADVECTION_H

#include "vapor/Particle.h"
#include "vapor/Field.h"
#include "vapor/common.h"
#include <string>
#include <vector>

namespace flow {
class FLOW_API Advection final {
public:
    enum class ADVECTION_METHOD {
        EULER = 0,
        RK4 = 1    // Runge-Kutta 4th order
    };

    // Constructor and destructor
    Advection();

    //
    // Major action functions
    //
    // Advect all particles as long as they are within spatial and temporal boundary
    // for a specified number if steps.
    int AdvectSteps(Field *velocityField, double deltaT, size_t maxSteps, ADVECTION_METHOD method = ADVECTION_METHOD::RK4);
    // Advect as many steps as necessary to reach a certain time: targetT.
    // Note: it only considers particles that have already passed startT.
    int AdvectTillTime(Field *velocityField, double startT, double deltaT, double targetT, ADVECTION_METHOD method = ADVECTION_METHOD::RK4);

    // Retrieve field values of a particle based on its location, and put the result in
    // the "value" field or the "properties" field of a particle
    //   If "skipNonZero" is true, then this function only overwrites zeros.
    //   Otherwise, it will overwrite values anyway.
    int CalculateParticleValues(Field *scalarField, bool skipNonZero);
    int CalculateParticleProperties(Field *scalarField);

    // Reset all particle values to zero
    void ResetParticleValues();
    // Clear all existing properties of a particle
    void ClearParticleProperties();
    // Clear particle property with a certain name.
    // If the the specified property name does not exist, then nothing is done.
    void RemoveParticleProperty(const std::string &);

    // Set advection basics
    void UseSeedParticles(const std::vector<Particle> &seeds);

    // Retrieve the resulting particles as "streams."
    size_t                       GetNumberOfStreams() const;
    const std::vector<Particle> &GetStreamAt(size_t i) const;

    // Retrieve the maximum number of particles in any stream
    size_t GetMaxNumOfPart() const;

    // Query properties (most are properties of the velocity field)
    int CheckReady() const;

    // Specify periodicity, and periodic bounds on each dimension
    void SetXPeriodicity(bool, float min, float max);
    void SetYPeriodicity(bool, float min, float max);
    void SetZPeriodicity(bool, float min, float max);

    // Retrieve the names of value variable and property variables.
    auto GetValueVarName() const -> std::string;
    auto GetPropertyVarNames() const -> std::vector<std::string>;

private:
    std::vector<std::vector<Particle>> _streams;
    std::string                        _valueVarName;
    std::vector<std::string>           _propertyVarNames;

    const float      _lowerAngle, _upperAngle;          // Thresholds for step size adjustment
    float            _lowerAngleCos, _upperAngleCos;    // Cosine values of the threshold angles
    std::vector<int> _separatorCount;                   // how many separators does each stream have.
                                                        // Useful to determine how many steps are there in a stream.
    // If the advection is performed in a periodic fashion along one or more dimensions.
    // These variables are **not** intended to be decided by Advection, but by someone
    // who's more knowledgeable about the field.
    bool      _isPeriodic[3];        // is it periodic in X, Y, Z dimensions ?
    glm::vec2 _periodicBounds[3];    // periodic boundaries in X, Y, Z dimensions

    // Advection methods here could assume all input is valid.
    int _advectEuler(Field *, const Particle &, double deltaT,    // Input
                     Particle &p1) const;                         // Output
    int _advectRK4(Field *, const Particle &, double deltaT,      // Input
                   Particle &p1) const;                           // Output

    // Get an adjust factor for deltaT based on how curvy the past two steps are.
    //   A value in range (0.0, 1.0) means shrink deltaT.
    //   A value in range (1.0, inf) means enlarge deltaT.
    //   A value equals to 1.0 means not touching deltaT.
    float _calcAdjustFactor(const Particle &past2, const Particle &past1, const Particle &current) const;

    // Adjust input "val" according to the bound specified by min and max.
    // Returns the value after adjustment.
    float _applyPeriodic(float val, float min, float max) const;
};
};    // namespace flow

#endif
