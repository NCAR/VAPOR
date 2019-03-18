/*
 * The base class of all vector fields with three components for flow integration.
 */

#ifndef VECTORFIELD_H
#define VECTORFIELD_H

#include "vapor/Field.h"
#include <string>

namespace flow
{
class VelocityField : public Field
{
public:
    VelocityField();
    virtual ~VelocityField();

    // 
    // Get the velocity value at a certain position, at a certain time.
    //
    virtual int  GetVelocity( float time, const glm::vec3& pos,     // input 
                              glm::vec3& vel ) const = 0;           // output

    virtual int GetNumberOfTimesteps() const = 0;

    // Varuable names for 3 velocity components
    std::string VelocityNameU, VelocityNameV, VelocityNameW;
};
};

#endif

