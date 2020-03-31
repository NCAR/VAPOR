#pragma once

#include "TFInfoWidget.h"
#include <QLineEdit>

namespace VAPoR {
class RenderParams;
}

class TFHistogramInfoWidget : public TFInfoWidget {
    Q_OBJECT

    using TFInfoWidget::TFInfoWidget;

public:
    TFHistogramInfoWidget(const std::string &variableNameTag);

public slots:
    void SetControlPoint(float value);
    void Deselect();
};
