#pragma once

#include "PLineItem.h"

class QColorWidget;

//! \class PColorSelector
//! Creates a Qt Color Selector synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PColorSelector : public PLineItem {
    Q_OBJECT

    QColorWidget *_colorWidget;

public:
    PColorSelector(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

    static QColor              VectorToQColor(const std::vector<double> &v);
    static std::vector<double> QColorToVector(const QColor &c);

private slots:
    void colorChanged(QColor color);
};
