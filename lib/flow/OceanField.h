/*
 * A derived VelocityField that simulates the ocean flow.
 */

#ifndef OCEANFIELD_H
#define OCEANFIELD_H

#include "VelocityField.h"

namespace flow
{
class OceanField : public VelocityField
{
public:
    OceanField();
   ~OceanField();
 
    int  Get( float time, const glm::vec3& pos, glm::vec3& vel ) const;
    bool InsideField( float time, const glm::vec3& pos ) const;
};

};

#endif
