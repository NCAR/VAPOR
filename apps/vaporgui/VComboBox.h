#pragma once

#include <string>
#include <QComboBox>
#include "VContainer.h"

class VComboBox : public VContainer {
    Q_OBJECT

public:
    VComboBox( const std::vector<std::string> &values );

    void SetOptions( const std::vector<std::string> &values );
    void SetIndex( int index );
    void SetValue( const std::string& value );

    int GetCurrentIndex() const;
    std::string GetCurrentString() const;
    int GetCount() const;

private:
    QComboBox* _combo;

public slots:
    void emitComboChanged( QString value);

signals:
    void ValueChanged( std::string value );
    void ValueChanged( int index );
};
