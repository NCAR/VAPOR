#include "PTotalTimestepsDisplay.h"
#include <vapor/ControlExecutive.h>
#include "VLabel.h"


PTotalTimestepsDisplay::PTotalTimestepsDisplay(VAPoR::ControlExec *ce) : PWidget("", _label = new VLabel), _ce(ce) {}


void PTotalTimestepsDisplay::updateGUI() const
{
    size_t totalTs = _ce->GetDataStatus()->GetTimeCoordinates().size();
    _label->SetText("Total timesteps:   " + std::to_string(totalTs));
}
