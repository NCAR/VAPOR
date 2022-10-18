#pragma once

#include "PWidget.h"
#include <functional>

class VPushButton;

typedef std::function<void(VAPoR::ParamsBase *)> Callback;
typedef std::function<void(std::string)> Callback2;

//! \class PButton
//! \brief PWidget wrapper for VPushButton.
//! \author Stas Jaroszynski
//!
//! Calls the callback when clicked.
//! Please don't capture in the callback.

class PButton : public PWidget {
    VPushButton *                                    _button;
    bool                                             _disableUndo = false;

public:
    PButton(std::string label, Callback cb);
    // @copydoc VAPoR::ParamsMgr::SetSaveStateUndoEnabled(bool)
    PButton *DisableUndo();

protected:
    const Callback                                   _cb;
    const Callback2                                   _cb2;
    void updateGUI() const override {}
    bool requireParamsMgr() const override { return _disableUndo; }
    virtual void clicked();
};
