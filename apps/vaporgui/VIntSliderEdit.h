#pragma once

#include <string>
#include "VHBoxWidget.h"
#include "VSliderEditInterface.h"

class VIntRangeMenu;
class VIntLineEdit;

//! \class VIntSliderEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a VSlider and a VIntLineEdit, that
//! provides synchronization between the two widgets, and support
//! for a menu that allows setting and getting the VSlider range,
//! and the numeric representation of the VIntLineEdit

class VIntSliderEdit : public VSliderEditInterface {
    Q_OBJECT

public:
    VIntSliderEdit(int min = 0, int max = 10, int value = 3, bool rangeChangable = false);

    //! Set the minimum allowable value for the VSlider and VIntLineEdit
    void SetMinimum(int min);

    //! Set the maximum allowable value for the VSlider and VIntLineEdit
    void SetMaximum(int max);

    void AllowUserRange(bool allowed = true);

    //! Get the value associated with the VSlider and VIntLineEdit
    int GetValue() const;

    //! Get the minimum allowable value for the VSlider and VIntLineEdit
    int GetMinimum() const;

    //! Get the maximum allowable value for the VSlider and VIntLineEdit
    int GetMaximum() const;

    //! Get the number of digits displayed by the VIntLineEdit
    virtual int GetNumDigits() const;

    //! Set the number of digits displayed by the VIntLineEdit
    virtual void SetNumDigits(int numDigits);

    //! Get whether the VIntLineEdit is being displayed with scientific notation
    virtual bool GetSciNotation() const;

    //! Set whether the VIntLineEdit is being displayed with scientific notation
    virtual void SetSciNotation(bool sciNotation);

    //! Show the context menu options for the entire widget, triggered on right-click
    virtual void ShowContextMenu(const QPoint &pos);

public slots:
    //! Set the current value displayed by the slider and line edit
    void SetValue(int value);

protected:
    virtual void _makeContextMenu();
    void         _sliderChanged(int value);
    void         _sliderChangedIntermediate(int value);

    int  _value;
    bool _rangeChangable;

    VIntRangeMenu *_menu;
    VIntLineEdit * _lineEdit;

signals:
    void ValueChanged(int value);
    void ValueChangedIntermediate(int value);
    void MinimumChanged(int min);
    void MaximumChanged(int max);
};
