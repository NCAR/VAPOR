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
    ADVECT_HAPPENED      =  1,
    SUCCESS              =  0,
    OUT_OF_FIELD         = -1,
    NO_FIELD_YET         = -2,
    NO_SEED_PARTICLE_YET = -3,
    FILE_ERROR           = -4,
    TIME_ERROR           = -5,
    GRID_ERROR           = -6,
    SIZE_MISMATCH        = -7
};

class Particle
{
public:
    glm::vec3                 location;
    float                     time;
    float                     value;

    // Constructor and destructor
    Particle();
    Particle( const glm::vec3& loc, float t, float val = 0.0f );
    Particle( const float* loc, float t, float val = 0.0f );
    Particle( float x, float y, float z, float t, float val = 0.0f );
   ~Particle();

    void  AttachProperty  (  float v );
    // This function will throw an exception when idx is out of bound
    float RetrieveProperty(  int idx ) const;
    void  ClearProperties();
    int   GetNumOfProperties() const;

    // A particle could be set to be at a special state.
    void  SetSpecial( bool isSpecial );
    bool  IsSpecial() const; 

private:
    std::forward_list<float>  _properties;  // Forward_list takes only 8 bytes, whereas a vector takes 24 bytes!
};

};

#endif
