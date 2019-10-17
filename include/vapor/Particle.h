/*
 * Defines a particle used in flow integration.
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include "vapor/common.h"
#include <glm/glm.hpp>
#include <forward_list>

namespace flow {
enum FLOW_ERROR_CODE    // these enum values are available in the flow namespace.
{
    ADVECT_HAPPENED = 1,
    SUCCESS = 0,
    OUT_OF_FIELD = -1,
    NO_FIELD_YET = -2,
    NO_SEED_PARTICLE_YET = -3,
    FILE_ERROR = -4,
    TIME_ERROR = -5,
    GRID_ERROR = -6,
    SIZE_MISMATCH = -7
};

// Particle is not expected to serve as a base class.
class FLOW_API Particle final {
public:
    glm::vec3 location{0.0f, 0.0f, 0.0f};
    float     time = 0.0f;
    float     value = 0.0f;

    // Constructors.
    // This class complies with rule of zero.
    Particle() = default;
    Particle(const glm::vec3 &loc, float t, float val = 0.0f);
    Particle(const float *loc, float t, float val = 0.0f);
    Particle(float x, float y, float z, float t, float val = 0.0f);

    //
    // The "property" field allows the user to keep one or more arbitrary values that
    // are associated with this particle. It's up to the user to keep a record on
    // what these values at each index stand for.
    //
    void AttachProperty(float v);
    // This function will throw an exception when idx is out of bound
    float RetrieveProperty(int idx) const;
    void  ClearProperties();
    int   GetNumOfProperties() const;

    // A particle could be set to be at a special state.
    void SetSpecial(bool isSpecial);
    bool IsSpecial() const;

private:
    std::forward_list<float> _properties;
    // Note on the choice of using a forward_list:
    // Forward_list takes 8 bytes, whereas a vector or list take 24 bytes!
    int _nProperties = 0;
    // Note on the property counter: since forward_list doesn't have a .size()
    // of itself, we keep a counter by ourselves.
    // Also, we consider an unsigned short type is big enough for this use case.
};

};    // namespace flow

#endif
