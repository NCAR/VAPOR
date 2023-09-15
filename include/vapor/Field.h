/*
 * The base class of all possible fields for flow integration.
 */

#ifndef FIELD_H
#define FIELD_H

#include <string>
#include <array>
#include <glm/glm.hpp>
#include <vapor/common.h>

namespace flow {
class FLOW_API Field {
public:
    // Constructor and destructor.
    // This class complies with rule of zero.
    Field() = default;
    virtual ~Field() = default;

    //
    // If a given position at a given time is inside of this field
    //
    virtual bool InsideVolumeVelocity(double time, const glm::vec3 &pos) const = 0;
    virtual bool InsideVolumeScalar(double time, const glm::vec3 &pos) const = 0;

    //
    // Retrieve the number of time steps in this field
    //
    virtual int GetNumberOfTimesteps() const = 0;

    //
    // Get the field value at a certain position, at a certain time.
    //
    virtual int GetScalar(double time, const glm::vec3 &pos,    // input
                          float &val) const = 0;                // output

    //
    // Get the velocity value at a certain position, at a certain time.
    //
    virtual int GetVelocity(double time, const glm::vec3 &pos,    // input
                            glm::vec3 &vel) const = 0;            // output

    //
    // Returns the number of empty velocity variable names.
    // It is 3 when the object is newly created, or is used to represent a scalar field
    //
    int GetNumOfEmptyVelocityNames() const;

    //
    // Provide an option to cache and lock certain parameters.
    // Both functions return 0 on success.
    //
    virtual auto LockParams() -> int = 0;
    virtual auto UnlockParams() -> int = 0;

    // Class members
    bool                       IsSteady = false;
    std::string                ScalarName = "";
    std::array<std::string, 3> VelocityNames = {{"", "", ""}};
};
};    // namespace flow

#endif
