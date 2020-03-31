#pragma once

#include "TFInfoWidget.h"
#include <QLineEdit>
#include "QColorWidget.h"

namespace VAPoR {
class RenderParams;
}

class TFColorInfoWidget : public TFInfoWidget {
    Q_OBJECT

public:
    TFColorInfoWidget(const std::string &variableNameTag);
    void SetColor(const QColor &color);

protected:
    void controlPointChanged();

private:
    QColorWidget *_colorEdit;

signals:
    void ControlPointChanged(float value, QColor color);

public slots:
    void SetControlPoint(float value, const QColor &color);
    void DeselectControlPoint();

private slots:
    void colorEditChanged();
};
