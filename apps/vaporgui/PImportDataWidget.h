#pragma once

#include "PSection.h"
#include "PLineItem.h"

namespace VAPoR {
    class ControlExec;
}
class VGroup;
class PGroup;
class QRadioButton;
class PFilesOpenSelector;
class MainForm;
class VHBoxWidget;
class VLabel;

class PImportDataWidget : public PWidget {
    Q_OBJECT

    PGroup*             _group;
    VSection*           _section;
    PFilesOpenSelector* _selector;
    VHBoxWidget*        _hBox;
    VAPoR::ControlExec* _ce;
    MainForm* _mf;

public:
    PImportDataWidget(VAPoR::ControlExec* ce, MainForm *mf);

protected:
    void updateGUI() const override;

signals:
    void dataImported();

};
