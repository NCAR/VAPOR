#pragma once

#include <string>
#include "VContainer.h"

class VSlider;
class VLineEdit;

//! class VSliderEdit
//!
//! Wraps a VSlider and a VLineEdit for selecting a numeric value within a
//! defined range.  Allows for integer and double value types.  This class
//! also provides vaporgui's standard setter/getter functions and signals.
class VSliderEdit : public VContainer {
    Q_OBJECT

public:
    VSliderEdit( 
        double min   = 0., 
        double max   = 1., 
        double value = 0.,
        bool intType = false
    );

    VSliderEdit(
        bool intType
    );

    void SetIntType( bool type );

    void SetValue( double value );
    void SetRange( double min, double max );

    double GetValue() const;

protected:
    VLineEdit* _lineEdit;
    VSlider*   _slider;
    double     _minValid;
    double     _maxValid;
    double     _value;
    bool       _isIntType;
    bool       _scientific;
    int        _decDigits;

public slots:
    void ShowContextMenu( const QPoint& );

protected slots:
    void _lineEditChanged( const std::string& value );

    void _sliderChanged( double value );
    void _sliderChangedIntermediate( double value );

    void _decimalDigitsChanged( int value );
    void _scientificClicked( bool value );
    void _minRangeChanged( double value );
    void _maxRangeChanged( double value );

signals:
    void ValueChanged( double value );
    void ValueChangedInt( int    value );

    void ValueChangedIntermediate( double value );
    void ValueChangedIntIntermediate( int    value );
};

class VIntSliderEdit : public VSliderEdit {
    Q_OBJECT

public:
    VIntSliderEdit(
        int min   = 0,
        int max   = 1,
        int value = 0
    );

private slots:
    void _sliderChanged( int value );
    void _sliderChangedIntermediate( int value );
    //void _minRangeChanged( int value );
    //void _maxRangeChanged( int value );

signals:
    void ValueChanged( int value );
    void ValueChangedIntermediate( int value );
};
