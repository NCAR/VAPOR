#pragma once

#include <string>
#include "VContainer.h"

class VSlider;
class VCheckBoxAction;
class VSpinBoxAction;

class VSliderEditInterface : public VContainer {
    Q_OBJECT

public:
    virtual void SetSciNotation( bool sci ) = 0;
    virtual void SetNumDigits( int digits ) = 0;

    virtual bool   GetSciNotation() const = 0;
    virtual int    GetNumDigits() const = 0;

    virtual void ShowContextMenu( const QPoint& ) = 0;

protected:
    VSliderEditInterface();
    virtual void _makeContextMenu() = 0;

    VSlider*         _slider;
    VSpinBoxAction*  _decimalAction;
    VCheckBoxAction* _scientificAction;

signals:
    void FormatChanged();
};
