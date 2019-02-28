/*
 * The base class of all possible velocity fields for flow integration.
 */

#ifndef VELOCITYFIELD_H
#define VELOCITYFIELD_H

#include <glm/glm.hpp>
#include "Particle.h"

namespace flow
{
class VelocityField
{
public:
    // Constructor and destructor
    VelocityField();
   ~VelocityField();

    // 
    // Get the velocity value at a certain position, at a certain time.
    //
    virtual int  GetVelocity( float time, const glm::vec3& pos,     // input 
                              glm::vec3& vel ) const = 0;           // output

    // 
    // If a given position at a given time is inside of this field
    //
    virtual bool InsideVelocityField( float time, const glm::vec3& pos ) const = 0;

    // Class members
    bool      IsSteady;
    bool      IsPeriodic;
};
};

#endif
