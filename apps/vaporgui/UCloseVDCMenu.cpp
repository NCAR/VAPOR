#include "UCloseVDCMenu.h"
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>


UCloseVDCMenu::UCloseVDCMenu(QMenu *parent, ControlExec *ce)
: QMenu("Close Dataset", parent), _ce(ce)
{ parent->addMenu(this); }


void UCloseVDCMenu::Update()
{
    this->clear();
    vector<string> dataSetNames = _ce->GetParams<GUIStateParams>()->GetOpenDataSetNames();
    if (dataSetNames.empty()) {
        this->setEnabled(false); // This doesn't really work on macOS (qt bug)
    } else {
        this->setEnabled(true);
        for (const auto &name : dataSetNames)
            this->addAction(QString::fromStdString(name), [this, name](){ _ce->CloseData(name); });
    }
}