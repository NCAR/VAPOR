/* 
 * Defines a particle used in flow integration.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <forward_list>

namespace flow
{
class Particle
{
public:
    glm::vec3                 location;
    float                     value;
    float                     time;
    std::forward_list<float>  properties;   // Forward_list takes only 8 bytes, whereas a vector takes 24 bytes!

    // Constructor and destructor
    Particle();
    Particle( const glm::vec3& loc, float t );
    Particle( const float* loc, float t );
    Particle( float x, float y, float z, float t );
   ~Particle();
};

};

#endif
