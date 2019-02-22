/*
 * The base class of all possible velocity fields for flow integration.
 */

#ifndef VELOCITYFIELD_H
#define VELOCITYFIELD_H

#include <glm/glm.hpp>

namespace flow
{
class VelocityField
{
public:
    enum ERROR_CODE
    {
        SUCCESS      =  0,
        OUT_OF_FIELD = -1
    };

    // Constructor and destructor
    VelocityField();
   ~VelocityField();

    // 
    // Get the velocity value at a certain position, at a certain time.
    //
    virtual int  Get( float time, const glm::vec3& pos,         // input 
                                        glm::vec3& vel ) = 0;   // output

    // 
    // The base class implements a basic bounding box test.
    // Children classes can implement more complex schemes.
    //
    virtual bool insideField( const glm::vec3& pos );

    // Class members
    bool      isSteady;
    bool      isPeriodic;

protected:
    template <class T>
    T lerp( const T& v1, const T& v2, float a )
    {
        return glm::mix( v1, v2, a );
    }

    glm::vec3 _fieldMin, _fieldMax;
};
};

#endif
