#pragma once

#include "RenderHolder.h"
#include "RenderEventRouter.h"

namespace VAPoR {class ControlExec;}
using namespace VAPoR;

class NewRendererDialogManager : public QObject {
    Q_OBJECT

    ControlExec *_ce;
    NewRendererDialog *_nrd;

public:
    NewRendererDialogManager(ControlExec *ce, QWidget *parent);

public slots:
    void Show();

private:
    static NewRendererDialog* ConstructNewRendererDialog(vector<RenderEventRouter*> routers);
    static NewRendererDialog* ConstructNewRendererDialog(ControlExec *ce);
};