#pragma once

#include "TFInfoWidget.h"
#include <QLineEdit>

namespace VAPoR {
class RenderParams;
}

class TFHistogramInfoWidget : public TFInfoWidget {
    Q_OBJECT

public:
    TFHistogramInfoWidget();

public slots:
    void SetControlPoint(float value);
    void Deselect();
};
