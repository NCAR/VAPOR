#pragma once

#include "VScrollArea.h"
#include "VGroup.h"
#include "VContainer.h"

class VScrollGroup : public VScrollArea, public WidgetGroupWrapper<VScrollGroup, QWidget, VGroup> {
    Q_OBJECT

    VContainer *_container;
    VGroup *_group;
public:
    VScrollGroup(List children = {})
    : VScrollArea(_container = new VContainer(_group = new VGroup())),
      WidgetGroupWrapper(_group)
    {
        _container->AddBottomStretch();
        _group->AddM(children);//
    }
};