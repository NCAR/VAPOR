#pragma once

#include "PWidget.h"
#include <functional>

class VPushButton;

class PButton : public PWidget {
    typedef std::function<void(VAPoR::ParamsBase*)> Callback;
    VPushButton *_button;
    const Callback _cb;
public:
    PButton(std::string label, Callback cb);
protected:
    void updateGUI() const override {}
private:
    void clicked();
};
