#ifndef HELLOSUBTABS_H
#define HELLOSUBTABS_H
#include "ui_HelloVariablesGUI.h"

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class HelloVariablesSubtab : public QWidget, public Ui_HelloVariablesGUI {
    Q_OBJECT

public:
    HelloVariablesSubtab(QWidget *parent);
    ~HelloVariablesSubtab(){};

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }

protected:
private:
    // VariablesWidget* _variablesWidget;
};

#endif
