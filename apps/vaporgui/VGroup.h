#pragma once

#include <QWidget>
#include "AbstractWidgetGroup.h"

//! \class VGroup
//! Creates a group for parameter controls in the sidebar

class VGroup : public QWidget, public AbstractWidgetGroup<VGroup, QWidget> {
public:
    VGroup();
    VGroup *Add(QWidget *w);
};

//! \class VSubGroup
//! Creates a subgroup for parameter controls in the sidebar

class VSubGroup : public VGroup {
public:
    VSubGroup();
};
