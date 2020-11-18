#pragma once

#include "PWidget.h"
#include <functional>

class VPushButton;

//! \class PButton
//! \brief PWidget wrapper for VPushButton.
//! \author Stas Jaroszynski
//!
//! Calls the callback when clicked.
//! Please don't capture in the callback.

class PButton : public PWidget {
    typedef std::function<void(VAPoR::ParamsBase *)> Callback;
    VPushButton *                                    _button;
    const Callback                                   _cb;
    bool                                             _disableUndo = false;

public:
    PButton(std::string label, Callback cb);
    // @copydoc VAPoR::ParamsMgr::SetSaveStateUndoEnabled(bool)
    PButton *DisableUndo();

protected:
    void updateGUI() const override {}
    bool requireParamsMgr() const override { return _disableUndo; }

private:
    void clicked();
};
