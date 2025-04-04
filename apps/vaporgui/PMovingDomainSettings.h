#pragma once

#include "PSection.h"

//! \class PMovingDomainSettings
//! Applies parameters for moving domains
//! \copydoc PSection

class PMovingDomainSettings : public PSection {
    ControlExec *_ce;

public:
    PMovingDomainSettings(ControlExec *ce);

protected:
    bool isEnabled() const override;
};
