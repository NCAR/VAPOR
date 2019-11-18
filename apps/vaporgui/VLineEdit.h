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

    void SetIsDouble( bool isDouble );

private:
    QLineEdit*  _lineEdit;
    std::string _value;
    bool        _isDouble;

public slots:
    void emitLineEditChanged();

signals:
    void ValueChanged( const std::string& value );
};
