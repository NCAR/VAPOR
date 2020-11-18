#include <algorithm>
#include "vapor/Field.h"

using namespace flow;

int Field::GetNumOfEmptyVelocityNames() const
{
    return std::count_if(VelocityNames.begin(), VelocityNames.end(), [](const std::string &e) { return e.empty(); });
}
