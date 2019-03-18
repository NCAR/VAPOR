/*
 * The base class of all possible scalar fields with 1 component for flow integration.
 */


#ifndef SCALARFIELD_H
#define SCALARFIELD_H

#include "vapor/Field.h"
#include <string>

namespace flow
{
class ScalarField : public Field
{
public:
    ScalarField();
    virtual ~ScalarField();

    // 
    // Get the field value at a certain position, at a certain time.
    //
    virtual int  GetScalar(  float time, const glm::vec3& pos,   // input 
                             float& val) const = 0;              // output

    virtual int GetNumberOfTimesteps() const = 0;

    std::string ScalarName;  // Varuable names for the value field

};
};

#endif
