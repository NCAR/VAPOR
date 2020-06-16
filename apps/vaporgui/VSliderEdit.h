#pragma once

#include <string>
#include "VContainer.h"

class QMenu;
class VSlider;
class VNumericLineEdit;
class VIntLineEditAction;
class VDoubleLineEditAction;
class VCheckBoxAction;
class VSpinBoxAction;

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

    void SetValue( double value );
    void SetRange( double min, double max );
    void SetMinimum( double min );
    void SetMaximum( double max );
    void SetSciNotation( bool sci );
    void SetNumDigits( int digits );

    double GetValue() const;
    double GetMinimum() const;
    double GetMaximum() const;
    bool   GetScientific() const;
    int    GetNumDigits() const;

protected:
    void MakeContextMenu();

    VNumericLineEdit* _lineEdit;
    VSlider*          _slider;

    double     _minValid;
    double     _maxValid;
    double     _value;
    bool       _isIntType;
    bool       _scientific;
    int        _decDigits;

    QMenu*                 _menu;
    VIntLineEditAction*    _minIntAction;
    VIntLineEditAction*    _maxIntAction;
    VDoubleLineEditAction* _minDoubleAction;
    VDoubleLineEditAction* _maxDoubleAction;
    VSpinBoxAction*        _decimalAction;
    VCheckBoxAction*       _scientificAction;

public slots:
    void ShowContextMenu( const QPoint& );

protected slots:
    void _lineEditChanged( const std::string& value );

    //void _sliderChanged( double value );
    void _sliderChangedIntermediate( double value );

    void _decimalDigitsChanged( int value );
    //void _scientificClicked( bool value );
    void _minRangeChanged( double value );
    void _maxRangeChanged( double value );

signals:
    void ValueChanged( double value );
    void ValueChanged( int    value );

    void ValueChangedIntermediate( double value );
    void ValueChangedIntermediate( int    value );

    void FormatChanged();
};


class VSliderEdit2 : public VContainer {
    Q_OBJECT

public:
    VSliderEdit2();
    /*VSliderEdit( 
        double min   = 0., 
        double max   = 1., 
        double value = 0.,
        bool intType = false
    );

    VSliderEdit(
        bool intType
    );*/

    //void SetValue( double value );
    //void SetRange( double min, double max );
    //void SetMinimum( double min );
    //void SetMaximum( double max );
    void SetSciNotation( bool sci );
    void SetNumDigits( int digits );

    //double GetValue() const;
    //double GetMinimum() const;
    //double GetMaximum() const;
    bool   GetSciNotation() const;
    int    GetNumDigits() const;

protected:
    void _makeContextMenu();
    //void _lineEditChanged( const std::string& value );

    VNumericLineEdit* _lineEdit;
    VSlider*          _slider;

    /*double     _minValid;
    double     _maxValid;
    double     _value;
    bool       _isIntType;*/
    bool       _sciNotation;
    int        _decDigits;

    QMenu*                 _menu;
    /*VIntLineEditAction*    _minIntAction;
    VIntLineEditAction*    _maxIntAction;
    VDoubleLineEditAction* _minDoubleAction;
    VDoubleLineEditAction* _maxDoubleAction;*/
    VSpinBoxAction*        _decimalAction;
    VCheckBoxAction*       _scientificAction;

public slots:
    void ShowContextMenu( const QPoint& );

protected slots:

    //void _sliderChanged( double value );
    //void _sliderChangedIntermediate( double value );

    //void _decimalDigitsChanged( int value );
    //void _scientificClicked( bool value );
    //void _minRangeChanged( double value );
    //void _maxRangeChanged( double value );

signals:
    /*void ValueChanged( double value );
    void ValueChanged( int    value );

    void ValueChangedIntermediate( double value );
    void ValueChangedIntermediate( int    value );*/

    void FormatChanged();
};


class VIntSliderEdit : public VSliderEdit2 {
    Q_OBJECT

public:
    VIntSliderEdit( int min=0, int max=10, int value=3 );

    void SetValue( int value );
    void SetMinimum( int min );
    void SetMaximum( int max );

    int GetValue() const;
    int GetMinimum() const;
    int GetMaximum() const;
    
protected:
    int _min;
    int _max;
    int _value;    

    VIntLineEditAction* _minRangeAction;
    VIntLineEditAction* _maxRangeAction;
    
    //void _sliderChanged( int value );
    void _sliderChangedIntermediate( int value );

signals:
    void ValueChanged( int value );
    void ValueChangedIntermediate( int value );
    void MinimumChanged( int min );
    void MaximumChanged( int max );
};
