#pragma once

#include <string>
#include <QComboBox>
#include "VContainer.h"

class VComboBox2 : public VContainer {
    Q_OBJECT

public:
    VComboBox2( const std::vector<std::string> &values );

    void SetOptions( const std::vector<std::string> &values );

    int GetCurrentIndex() const;
    std::string GetCurrentString() const;

private:
    QComboBox* _combo;

public slots:
    void emitComboChanged( QString value);

signals:
    void ValueChanged( std::string value );
};
