#pragma once

#include <string>
#include "VLineItem.h"
#include "VComboBox.h"


class VComboBox;

//! class VLineComboBox
//!
//! Wraps a VComboBox in a VLineItem, and provides vaporgui's standard 
//! setter/getter functions and signals.

class VLineComboBox : public VLineItem {
    Q_OBJECT

public:
    VLineComboBox( const std::string& label );

    void SetOptions( const std::vector<std::string>& values );
    void SetIndex( int index );
    void SetValue( const std::string& value );

    std::string GetValue() const;
    int GetCurrentIndex() const;
    int GetCount() const;

private:
    VComboBox* _combo;

public slots:
    void emitComboChanged( const std::string& value);

signals:
    void ValueChanged( std::string value );
    void IndexChanged( int index );
};
