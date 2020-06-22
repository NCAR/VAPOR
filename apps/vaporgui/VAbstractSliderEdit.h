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
