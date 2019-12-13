#pragma once

#include "PLineItem.h"

class QColorWidget;

class PColorSelector : public PLineItem {
    Q_OBJECT

    QColorWidget *_colorWidget;

public:
    PColorSelector(const std::string &tag, const std::string &label);

    static QColor              VectorToQColor(const std::vector<double> &v);
    static std::vector<double> QColorToVector(const QColor &c);

protected:
    void updateGUI() const override;

private slots:
    void colorChanged(QColor color);
};
