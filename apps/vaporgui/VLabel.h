#pragma once

#include "VContainer.h"
#include <string>

class QLabel;

class VLabel : public VContainer {
    QLabel *_ql;

public:
    VLabel(const std::string &text = "");
    void SetText(const std::string &text);
    void MakeSelectable();
};
