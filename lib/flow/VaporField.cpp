#include "VaporField.h"

using namespace flow;

VaporField::VaporField()
{
    _velocityU = nullptr;
    _velocityV = nullptr;
    _velocityW = nullptr;
    _value     = nullptr;
}

VaporField::~VaporField()
{
    _velocityU = nullptr;
    _velocityV = nullptr;
    _velocityW = nullptr;
    _value     = nullptr;
}

int
VaporField::Get( float t, const glm::vec3& pos, glm::vec3& vel ) const
{
    return 0;
}

bool
VaporField::InsideField( const glm::vec3& pos ) const
{
    return false;
}

void
VaporField::UseVelocityField( const VGrid* u, const VGrid* v, const VGrid* w )
{
    _velocityU = u;
    _velocityV = v;
    _velocityW = w;
}

void 
VaporField::UseValueField( const VGrid* val )
{
    _value = val;
}
