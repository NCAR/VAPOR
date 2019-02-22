/* 
 * Defines a particle used in flow integration.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <forward_list>

namespace flow
{
enum ERROR_CODE
{
    SUCCESS      =  0,
    OUT_OF_FIELD = -1,
    OUT_OF_RANGE = -2
};

class Particle
{
public:
    glm::vec3                 location;
    float                     value;
    float                     time;

    // Constructor and destructor
    Particle();
    Particle( const glm::vec3& loc, float t );
    Particle( const float* loc, float t );
    Particle( float x, float y, float z, float t );
   ~Particle();

    void AttachProperty  (  float v );
    int  EditProperty    (  int idx, float v ); 
    int  RetrieveProperty(  int idx,            // Input
                            float& v ) const;   // Output
    void ClearProperties();

private:
    std::forward_list<float>  _properties;  // Forward_list takes only 8 bytes, whereas a vector takes 24 bytes!
};

};

#endif
