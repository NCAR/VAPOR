/*
 * The base class of all possible fields for flow integration.
 */

#ifndef FIELD_H
#define FIELD_H

#include <glm/glm.hpp>

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
    virtual int GetExtents( float time, glm::vec3& minExt, glm::vec3& maxExt ) const = 0;


    // Class members
    bool        IsSteady;
    bool        IsPeriodic;
};
};

#endif
