/*
 * The base class of all possible fields for flow integration.
 */

#ifndef FIELD_H
#define FIELD_H

#include <glm/glm.hpp>
#include <string>

namespace flow {
class Field {
public:
    // Constructor and destructor.
    // This class complies with rule of zero.
    Field() = default;
    virtual ~Field() = default;

    //
    // If a given position at a given time is inside of this field
    //
    virtual bool InsideVolumeVelocity(float time, const glm::vec3 &pos) const = 0;
    virtual bool InsideVolumeScalar(float time, const glm::vec3 &pos) const = 0;

    //
    // Retrieve the number of time steps in this field
    //
    virtual int GetNumberOfTimesteps() const = 0;

    //
    // Get the field value at a certain position, at a certain time.
    // Users could control if this method checks position inside volume.
    //
    virtual int GetScalar(float time, const glm::vec3 &pos,    // input
                          float &val,                          // output
                          bool   checkInsideVolume = true) const = 0;

    //
    // Get the velocity value at a certain position, at a certain time.
    // Users could control if this method checks position inside volume.
    //
    virtual int GetVelocity(float time, const glm::vec3 &pos,    // input
                            glm::vec3 &vel,                      // output
                            bool       checkInsideVolume = true) const = 0;

    // Class members
    bool        IsSteady = false;
    std::string ScalarName;
    std::string VelocityNames[3];
};
};    // namespace flow

#endif
