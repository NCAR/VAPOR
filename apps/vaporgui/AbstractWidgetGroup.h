#pragma once

#include <initializer_list>
#include <vector>

template <class This, class Widget>
class AbstractWidgetGroup {
protected:
    std::vector<Widget *> _children;
public:
    typedef std::initializer_list<Widget *> List;
    
    virtual This *Add(Widget *w)
    {
        _children.push_back(w);
        return (This*)this;
    }
    This *AddM(const List &list)
    {
        for (const auto &w : list)
            Add(w);
        return (This*)this;
    }
};


template <class This, class Widget, class That>
class WidgetGroupWrapper : public AbstractWidgetGroup<This, Widget> {
    That *_group;
public:
    WidgetGroupWrapper(That *group)
    : _group(group) {}
    
    virtual This *Add(Widget *w) override
    {
        _group->Add(w);
        return AbstractWidgetGroup<This, Widget>::Add(w);
    }
};
