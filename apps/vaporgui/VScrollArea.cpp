#include "VScrollArea.h"

VScrollArea::VScrollArea(QWidget *w)
{
    QScrollArea::setWidget(w);
    QScrollArea::setWidgetResizable(true);
    sizePolicy().setVerticalStretch(1);
//    setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
}