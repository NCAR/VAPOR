#include <vapor/ControlExecutive.h>
#include "ControlExecWidget.h"

ControlExecWidget::ControlExecWidget(VAPoR::ControlExecutive *ce)
{
    VAssert(ce);
    _controlExec = ce;
}
