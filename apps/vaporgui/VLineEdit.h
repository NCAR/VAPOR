#pragma once

#include <string>

#include <QWidget>
#include <QLineEdit>

#include "VContainer.h"

class VLineEdit : public VContainer {
    Q_OBJECT

public:
    VLineEdit( const std::string& value = "");

    void SetValue( const std::string& value );

    std::string GetValue() const;

private:
    QLineEdit* _lineEdit;

public slots:
    void emitLineEditChanged();

signals:
    void ValueChanged( const std::string& value );
};
