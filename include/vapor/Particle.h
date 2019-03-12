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
    ADVECT_HAPPENED = 1,
    SUCCESS      =  0,
    OUT_OF_FIELD = -1,
    OUT_OF_RANGE = -2,
    NO_FIELD_YET  = -3,
    NO_SEED_PARTICLE_YET = -4,
    FILE_ERROR           = -5,
    NOT_CONTAIN_TIME     = -6,
    GRID_ERROR           = -7,
    SIZE_MISMATCH        = -8
};

class Particle
{
public:
    glm::vec3                 location;
    float                     time;
    float                     value;

    // Constructor and destructor
    Particle();
    Particle( const glm::vec3& loc, float t );
    Particle( const float* loc, float t );
    Particle( float x, float y, float z, float t );
   ~Particle();

    void  AttachProperty  (  float v );
    // This function will throw an exception when idx is out of bound
    float RetrieveProperty(  int idx ) const;
    void  ClearProperties();
    int   GetNumOfProperties() const;

private:
    std::forward_list<float>  _properties;  // Forward_list takes only 8 bytes, whereas a vector takes 24 bytes!
};

};

#endif
