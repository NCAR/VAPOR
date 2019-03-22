#ifndef VAPORFIELD_H
#define VAPORFIELD_H

#include "vapor/Field.h"

namespace flow
{
class VaporField : public Field
{
public:
    VaporField();
    virtual ~VaporField();

    virtual bool InsideVolume( float time, const glm::vec3& pos ) const;
    virtual int  GetVelocity(  float time, const glm::vec3& pos,     // input 
                               glm::vec3& vel ) const;               // output
    virtual int  GetScalar(    float time, const glm::vec3& pos,     // input 
                               float& val) const ;                   // output

    virtual int GetNumberOfTimesteps() const;


protected:
    


};
};

#endif
