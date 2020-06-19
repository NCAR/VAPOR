#pragma once

#include <string>
#include "VContainer.h"

class QMenu;
class VSlider;
class VNumericLineEdit;
class VIntLineEditAction;
class VIntLineEdit;
class VDoubleLineEditAction;
class VCheckBoxAction;
class VSpinBoxAction;
class VIntRangeMenu;

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

    VDoubleLineEdit* _lineEdit;
    QDoubleRangeMenu* _menu;
    

signals:
    void ValueChanged( double value );
    void ValueChangedIntermediate( double value );
    void MinimumChanged( double min );
    void MaximumChanged( double max );
};
