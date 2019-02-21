/* 
 * Defines a particle used in flow integration.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <vector>

namespace VAPoR
{
namespace flow
{
class Particle
{
public:
    glm::vec3           _location;
    glm::vec4           _color;
    std::vector<float>  _propertiesF;

    // Constructor and destructor
    Particle();
    Particle( const glm::vec3& loc );
    Particle( const float* loc );
    Particle( float x, float y, float z );
   ~Particle();
};

};
};

#endif
