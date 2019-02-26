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
    virtual int  Get( float time, const glm::vec3& pos,             // input 
                                        glm::vec3& vel ) const = 0; // output

    // 
    // The base class implements a basic bounding box test.
    // Children classes can implement more complex schemes.
    //
    virtual bool InsideField( const glm::vec3& pos ) const;

    // Class members
    bool      isSteady;
    bool      isPeriodic;

protected:
    template <class T>
    T lerp( const T& v1, const T& v2, float a ) const
    {
        return glm::mix( v1, v2, a );
    }

    glm::vec3 _fieldMin, _fieldMax;
};
};

#endif
