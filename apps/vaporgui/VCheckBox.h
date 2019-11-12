#pragma once

#include <string>

#include <QWidget>
#include <QCheckBox>

#include "VContainer.h"

class VCheckBox2 : public VContainer {
    Q_OBJECT

public:
    VCheckBox2( bool value );

    void SetValue( bool value );

    bool GetValue() const;

private:
    QCheckBox* _checkBox;

public slots:
    void emitCheckBoxChanged( bool checked);

signals:
    void ValueChanged( bool checked );
};
