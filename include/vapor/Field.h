/*
 * The base class of all possible fields for flow integration.
 */

#ifndef FIELD_H
#define FIELD_H

#include <glm/glm.hpp>
#include <string>

namespace flow
{
class Field
{
public:
    // Constructor and destructor
    Field();
    virtual ~Field();

    // 
    // If a given position at a given time is inside of this field
    //
    virtual bool InsideVolume( float time, const glm::vec3& pos ) const = 0;

    //
    // Retrieve the extents of this field. 
    //
    // virtual int GetExtents( float time, glm::vec3& minExt, glm::vec3& maxExt ) const = 0;

    //
    // Retrieve the number of time steps in this field
    //
    virtual int GetNumberOfTimesteps() const = 0;

    //
    // Get the field value at a certain position, at a certain time.
    //  
    virtual int  GetScalar(  float time, const glm::vec3& pos,   // input 
                             float& val) const = 0;              // output

    //
    // Get the velocity value at a certain position, at a certain time.
    //  
    virtual int  GetVelocity( float time, const glm::vec3& pos,     // input 
                              glm::vec3& vel ) const = 0;           // output

    // Class members
    bool            IsSteady;
    std::string     ScalarName;
    std::string     VelocityNames[3];
};
};

#endif
