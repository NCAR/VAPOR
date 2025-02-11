#pragma once

#include "VContainer.h"
#include <string>

class QLabel;

class VLabel : public VContainer {
protected:
    QLabel *_ql;

public:
    VLabel(const std::string &text = "");
    void SetText(const std::string &text);
    void MakeSelectable();
};

class VHyperlink : public VLabel {
public:
    VHyperlink(const std::string &text = "", const std::string &url = "", bool bullet=false);
};
