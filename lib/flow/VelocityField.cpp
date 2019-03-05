#include "vapor/VelocityField.h"

using namespace flow;

VelocityField::VelocityField()
{ 
    IsSteady      = true;
    IsPeriodic    = false;
    HasFieldValue = false;
}

VelocityField::~VelocityField()
{ }
