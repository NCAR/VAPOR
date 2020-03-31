#pragma once

#include "PWidget.h"
#include <vector>

//! \class PGroup
//! Groups together PWidgets. See ParamsWidgetDemo for example use cases.
//! \copydoc PWidget

class PGroup : public PWidget {
    Q_OBJECT

    QWidget *              _widget;
    std::vector<PWidget *> _children;

public:
    PGroup();
    //! Adds the PWidget to this group.
    PGroup *Add(PWidget *pw);

protected:
    void updateGUI() const override;
};
