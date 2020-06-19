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
class VIntRangeMenu;

class VAbstractSliderEdit : public VContainer {
    Q_OBJECT

public:
    virtual void SetSciNotation( bool sci ) = 0;
    virtual void SetNumDigits( int digits ) = 0;

    virtual bool   GetSciNotation() const = 0;
    virtual int    GetNumDigits() const = 0;

    virtual void ShowContextMenu( const QPoint& ) = 0;

protected:
    VAbstractSliderEdit();
    virtual void _makeContextMenu() = 0;

    VSlider*          _slider;

    VSpinBoxAction*        _decimalAction;
    VCheckBoxAction*       _scientificAction;

signals:
    void FormatChanged();
};


/*class VIntSliderEdit : public VAbstractSliderEdit {
    Q_OBJECT

public:
    VIntSliderEdit( int min=0, int max=10, int value=3 );

    void SetValue( int value );
    void SetMinimum( int min );
    void SetMaximum( int max );

    int GetValue() const;
    int GetMinimum() const;
    int GetMaximum() const;
    
    virtual int  GetNumDigits() const;
    virtual void SetNumDigits( int numDigits );

    virtual bool GetSciNotation() const;
    virtual void SetSciNotation( bool sciNotation );

protected:
    virtual void _makeContextMenu();
    void _sliderChanged( int value );
    void _sliderChangedIntermediate( int value );

    int _value;    

    VIntLineEdit* _lineEdit;
    VIntLineEditAction* _minRangeAction;
    VIntLineEditAction* _maxRangeAction;
    

signals:
    void ValueChanged( int value );
    void ValueChangedIntermediate( int value );
    void MinimumChanged( int min );
    void MaximumChanged( int max );
};

class VDoubleSliderEdit : public VAbstractSliderEdit {
    Q_OBJECT

public:
    VDoubleSliderEdit( double min=0, double max=10, double value=3 );

    void SetValue( double value );
    void SetMinimum( double min );
    void SetMaximum( double max );

    double GetValue() const;
    double GetMinimum() const;
    double GetMaximum() const;
    
    virtual int GetNumDigits() const;
    virtual void SetNumDigits( int numDigits );

    virtual bool GetSciNotation() const;
    virtual void SetSciNotation( bool sciNotation );

protected:
    virtual void _makeContextMenu();
    void _sliderChanged( double value );
    void _sliderChangedIntermediate( double value );

    double _value;    

    VIntLineEdit* _lineEdit;
    VIntLineEditAction* _minRangeAction;
    VIntLineEditAction* _maxRangeAction;
    

signals:
    void ValueChanged( double value );
    void ValueChangedIntermediate( double value );
    void MinimumChanged( double min );
    void MaximumChanged( double max );
};*/
