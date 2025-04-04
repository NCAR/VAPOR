#include <QLayout>
#include "VProjectionStringFrame.h"
#include "PProjectionStringWidget.h"

VProjectionStringFrame::VProjectionStringFrame(PProjectionStringWidget *section) : VFrame(), _section(section) {
    layout()->addWidget(_section);
    show();
}

void VProjectionStringFrame::Update(ParamsBase *p, ParamsMgr *pm, DataMgr* dm) {
    _section->Update(p, pm);
}

void VProjectionStringFrame::closeEvent(QCloseEvent *event) {
    emit closed();
    QWidget::closeEvent(event);
}
