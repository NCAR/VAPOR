
#include "vapor/Field.h"

using namespace flow;

int Field::GetNumOfEmptyVelocityNames() const
{
    int counter = 0;
    for (const auto &e : VelocityNames)
        if (e.empty()) counter++;

    return counter;
}
