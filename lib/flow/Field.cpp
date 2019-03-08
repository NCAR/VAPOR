#include "vapor/Field.h"

using namespace flow;

Field::Field()
{ 
    IsSteady       = true;
    IsPeriodic     = false;
    HasScalarValue = false;
}

Field::~Field()
{ }
