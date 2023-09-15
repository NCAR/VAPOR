#include "PButton.h"
#include "VPushButton.h"
#include <vapor/ParamsMgr.h>

PButton::PButton(std::string label, Callback cb) : PWidget("", _button = new VPushButton(label)), _cb(cb) { QObject::connect(_button, &VPushButton::ButtonClicked, this, &PButton::clicked); }

PButton *PButton::DisableUndo()
{
    _disableUndo = true;
    return this;
}

void PButton::clicked()
{
    if (_disableUndo) {
        auto pm = getParamsMgr();
        bool state = pm->GetSaveStateUndoEnabled();
        pm->SetSaveStateUndoEnabled(false);
        _cb(getParams());
        pm->SetSaveStateUndoEnabled(state);
    } else {
        _cb(getParams());
    }
}
