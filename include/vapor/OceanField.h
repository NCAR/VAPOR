/*
 * A derived Field that simulates the ocean flow.
 */

#ifndef OCEANFIELD_H
#define OCEANFIELD_H

#include "vapor/Field.h"

namespace flow
{
class OceanField : public Field
{
public:
    OceanField();
   ~OceanField();
 
    int  GetVelocity(   float time, const glm::vec3& pos, glm::vec3& vel ) const;
    int  GetScalar  (   float time, const glm::vec3& pos, float& val     ) const;
    bool InsideVolume(  float time, const glm::vec3& pos ) const;
    int  GetExtents(    float time, glm::vec3& minExt, glm::vec3& maxExt ) const;
};

};

#endif
