#include "vapor/VelocityField.h"

using namespace flow;

VelocityField::VelocityField()
{ 
    IsSteady       = true;
    IsPeriodic     = false;
    HasScalarValue = false;
}

VelocityField::~VelocityField()
{ }
