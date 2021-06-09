#pragma once

#include "PWidgetWrapper.h"


namespace VAPoR {
class ControlExec;
}


class PFramebufferSettingsSection : public PWidgetWrapper {
    VAPoR::ControlExec *_ce;

public:
    PFramebufferSettingsSection(VAPoR::ControlExec *ce);

protected:
    VAPoR::ParamsBase *getWrappedParams() const override;
};
