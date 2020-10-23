#pragma once

#include <string>
#include "VHBoxWidget.h"

class VSlider;
class VCheckBoxAction;
class VSpinBoxAction;

//! \class VSliderEditInterface
//! \ingroup Public_GUI
//! \brief An interface class that needs to be reimplemented to support the
//! synchronization of a VSlider and a numeric V*LineEdit, and their menus.

class VSliderEditInterface : public VHBoxWidget {
    Q_OBJECT

public:
    //! Set use of scientific notation on the current type of line edit
    virtual void SetSciNotation(bool sci) = 0;

    //! Set the number of digits shown on the current line edit
    virtual void SetNumDigits(int digits) = 0;

    //! Retrieve whether the currnet line edit is using scientific notation
    virtual bool GetSciNotation() const = 0;

    //! Get the number of digits in use by the current line edit
    virtual int GetNumDigits() const = 0;

    //! Show the context menu for the slider-edit, triggered on right-click
    virtual void ShowContextMenu(const QPoint &) = 0;

    //! Return the size-hint for the current slider-edit
    virtual QSize sizeHint() const;

protected:
    VSliderEditInterface();
    virtual void _makeContextMenu() = 0;

    VSlider *        _slider;
    VSpinBoxAction * _decimalAction;
    VCheckBoxAction *_scientificAction;

signals:
    void FormatChanged();
};
