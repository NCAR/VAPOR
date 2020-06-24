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
    PGroup(QWidget *w);
    void updateGUI() const override;
};

//! \class PSubGroup
//! Groups together PWidgets in a subgroup.
//! \copydoc PGroup

class PSubGroup : public PGroup {
    Q_OBJECT

public:
    PSubGroup();
};
