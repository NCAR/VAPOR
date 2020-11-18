#pragma once

#include <string>
#include "VHBoxWidget.h"
#include "VSliderEditInterface.h"

class QMenu;
class VDoubleLineEdit;
class VDoubleRangeMenu;

//! \class VDoubleSliderEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a VSlider and a VDoubleLineEdit, that
//! provides synchronization between the two widgets, and support
//! for a menu that allows setting and getting the VSlider range,
//! and the numeric representation of the VDoubleLineEdit

class VDoubleSliderEdit : public VSliderEditInterface {
    Q_OBJECT

public:
    VDoubleSliderEdit(double min = 0, double max = 10, double value = 3, bool rangeChangable = false);

    //! Set the minimum allowable value for the VSlider and VDoubleLineEdit
    void SetMinimum(double min);

    //! Set the maximum allowable value for the VSlider and VDoubleLineEdit
    void SetMaximum(double max);

    void AllowUserRange(bool allowed = true);

    //! Get the value associated with the VSlider and VDoubleLineEdit
    double GetValue() const;

    //! Get the minimum allowable value for the VSlider and VDoubleLineEdit
    double GetMinimum() const;

    //! Get the maximum allowable value for the VSlider and VDoubleLineEdit
    double GetMaximum() const;

    //! Get the number of digits displayed by the VDoubleLineEdit
    virtual int GetNumDigits() const;

    //! Get whether the VDoubleLineEdit is being displayed with scientific notation
    virtual bool GetSciNotation() const;

public slots:
    //! Set the current value displayed by the slider and line edit
    void SetValue(double value);

    //! Set the number of digits displayed by the VDoubleLineEdit
    virtual void SetNumDigits(int numDigits);

    //! Set whether the VDoubleLineEdit is being displayed with scientific notation
    virtual void SetSciNotation(bool sciNotation);

    //! Show the context menu options for the entire widget, triggered on right-click
    virtual void ShowContextMenu(const QPoint &pos);

protected:
    virtual void _makeContextMenu();
    void         _sliderChanged(double value);
    void         _sliderChangedIntermediate(double value);

    double _value;
    bool   _rangeChangable;

    VDoubleLineEdit * _lineEdit;
    VDoubleRangeMenu *_menu;

signals:
    void ValueChanged(double value);
    void ValueChangedIntermediate(double value);
    void MinimumChanged(double min);
    void MaximumChanged(double max);
};
