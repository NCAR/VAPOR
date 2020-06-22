#pragma once

#include <string>
#include "VContainer.h"
#include "VAbstractSliderEdit.h"

class QMenu;
class VSlider;
//class VNumericLineEdit;
//class VIntLineEditAction;
//class VIntLineEdit;
class VDoubleLineEdit;
class VDoubleLineEditAction;
//class VCheckBoxAction;
//class VSpinBoxAction;
//class VIntRangeMenu;
class VDoubleRangeMenu;

class VDoubleSliderEdit : public VAbstractSliderEdit {
    Q_OBJECT

public:
    VDoubleSliderEdit( double min=0, double max=10, double value=3 );

    void SetMinimum( double min );
    void SetMaximum( double max );

    double GetValue() const;
    double GetMinimum() const;
    double GetMaximum() const;
    
    virtual int GetNumDigits() const;
    virtual void SetNumDigits( int numDigits );

    virtual bool GetSciNotation() const;
    virtual void SetSciNotation( bool sciNotation );

    virtual void ShowContextMenu( const QPoint& pos );

public slots:
    void SetValue( double value );

protected:
    virtual void _makeContextMenu();
    void _sliderChanged( double value );
    void _sliderChangedIntermediate( double value );

    double _value;    

    VDoubleLineEdit*  _lineEdit;
    VDoubleRangeMenu* _menu;
    

signals:
    void ValueChanged( double value );
    void ValueChangedIntermediate( double value );
    void MinimumChanged( double min );
    void MaximumChanged( double max );
};
