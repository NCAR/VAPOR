#include "VSubGroup.h"
#include <QVBoxLayout>

VSubGroup::VSubGroup()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(12, 0, 0, 0);
    setLayout(layout);
}
