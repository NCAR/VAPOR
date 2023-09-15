#pragma once

#include <QWidget>
#include "AbstractWidgetGroup.h"

class QBoxLayout;

//! \class VGroup
//! Creates a group for parameter controls in the sidebar

class VGroup : public QWidget, public AbstractWidgetGroup<VGroup, QWidget> {
    Q_OBJECT
public:
    VGroup(List children = {});
    VGroup *Add(QWidget *w);
protected:
    VGroup(QBoxLayout *layout, List children);
};


class VHGroup : public VGroup {
    Q_OBJECT
public:
    VHGroup();
};


//! \class VSubGroup
//! Creates a subgroup for parameter controls in the sidebar

class VSubGroup : public VGroup {
    Q_OBJECT
public:
    VSubGroup(List children = {});
};
