/*
 * A derived VelocityField that simulates the ocean flow.
 */

#ifndef OCEANFIELD_H
#define OCEANFIELD_H

#include "vapor/VelocityField.h"

namespace flow
{
class OceanField : public VelocityField
{
public:
    OceanField();
   ~OceanField();
 
    int  GetVelocity(   float time, const glm::vec3& pos, glm::vec3& vel ) const;
    int  GetFieldValue( float time, const glm::vec3& pos, float& val     ) const;
    bool InsideVolume(  float time, const glm::vec3& pos ) const;
};

};

#endif
