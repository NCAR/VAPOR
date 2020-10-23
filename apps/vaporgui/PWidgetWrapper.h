#pragma once

#include "PWidget.h"

//! \class PWidgetWrapper
//! \brief Provides a streamlined interface for a PWidget that wraps another PWidget.
//! \author Stas Jaroszynski

class PWidgetWrapper : public PWidget {
    PWidget *_child;

public:
    PWidgetWrapper(PWidget *p);
    PWidgetWrapper(std::string tag, PWidget *p);

protected:
    void updateGUI() const override;
};
