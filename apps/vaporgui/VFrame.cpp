#include "VFrame.h"

#include <QVBoxLayout>

VFrame::VFrame()
{
    setLayout(new QVBoxLayout);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(12);
    setFrameStyle(QFrame::NoFrame);
}

void VFrame::addWidget(QWidget *widget)
{
    layout()->addWidget(widget);
    _child_widgets.push_front(widget);
    _num_of_children++;
}

int VFrame::getNumOfChildWidgets() const { return _num_of_children; }

int VFrame::hideChildAtIdx(int idx)
{
    if (_child_widgets.empty())
        return 1;
    else if (idx < 0 || idx >= _num_of_children)
        return 1;
    else {
        auto itr = _child_widgets.before_begin();
        for (int i = 0; i < _num_of_children - idx; i++) ++itr;
        (*itr)->hide();
        return 0;
    }
}

int VFrame::showChildAtIdx(int idx)
{
    if (_child_widgets.empty())
        return 1;
    else if (idx < 0 || idx >= _num_of_children)
        return 1;
    else {
        auto itr = _child_widgets.before_begin();
        for (int i = 0; i < _num_of_children - idx; i++) ++itr;
        (*itr)->show();
        return 0;
    }
}
