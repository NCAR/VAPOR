#pragma once

#include "PWidget.h"
#include "AbstractWidgetGroup.h"
#include <vector>
#include "VGroup.h"

class VGroup;

//! \class PGroup
//! Groups together PWidgets. See ParamsWidgetDemo for example use cases.
//! \copydoc PWidget

class PGroup : public PWidget, public WidgetGroupWrapper<PGroup, PWidget, VGroup> {
    Q_OBJECT

    VGroup *_widget;

public:
    PGroup();
    PGroup(const List &widgets);

protected:
    PGroup(VGroup *w);
    void updateGUI() const override;
};

//! \class PSubGroup
//! Groups together PWidgets in a subgroup.
//! \copydoc PGroup

class PSubGroup : public PGroup {
    Q_OBJECT

public:
    PSubGroup();
    PSubGroup(const List &widgets);
};
