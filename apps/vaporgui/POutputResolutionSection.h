#pragma once

#include "PWidgetWrapper.h"


namespace VAPoR {
class ControlExec;
}


class POutputResolutionSection : public PWidgetWrapper {
    VAPoR::ControlExec *_ce;

public:
    POutputResolutionSection(VAPoR::ControlExec *ce);

protected:
    VAPoR::ParamsBase *getWrappedParams() const override;
};
