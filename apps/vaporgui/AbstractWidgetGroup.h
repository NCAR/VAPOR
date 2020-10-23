#pragma once

#include <initializer_list>
#include <vector>

//! \class AbstractWidgetGroup
//! \brief Provides an interface that standardizes widget grouping classes
//! \author Stas Jaroszynski

template<class This, class Widget> class AbstractWidgetGroup {
protected:
    std::vector<Widget *> _children;

public:
    typedef std::initializer_list<Widget *> List;

    //! Adds a widget.
    virtual This *Add(Widget *w)
    {
        _children.push_back(w);
        return (This *)this;
    }
    //! Adds multiple widgets.
    //! Has a different name for compiler reasons.
    This *AddM(const List &list)
    {
        for (const auto &w : list) Add(w);
        return (This *)this;
    }
};

//! \class WidgetGroupWrapper
//! \brief Automates the creation of widget groups that wrap other widget groups
//! \author Stas Jaroszynski

template<class This, class Widget, class That> class WidgetGroupWrapper : public AbstractWidgetGroup<This, Widget> {
    That *_group;

public:
    WidgetGroupWrapper(That *group) : _group(group) {}

    virtual This *Add(Widget *w) override
    {
        _group->Add(w);
        return AbstractWidgetGroup<This, Widget>::Add(w);
    }
};
