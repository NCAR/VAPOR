#pragma once

#include "TFInfoWidget.h"
#include <QLineEdit>

namespace VAPoR {
class RenderParams;
}

class TFIsoValueInfoWidget : public TFInfoWidget {
    Q_OBJECT

protected:
    void controlPointChanged();

signals:
    void ControlPointChanged(float value);

public slots:
    void SetControlPoint(float value);
    void Deselect();
};
