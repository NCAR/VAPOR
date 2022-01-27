#ifndef COMBO_H
#define COMBO_H

#include <QWidget>
#include <QLineEdit>
#include <QSlider>
#include <QValidator>

#ifdef Darwin
    #include "QMontereySlider.h"
    #define QSlider QMontereySlider
#endif

// class Combo
//
// Manages a paired QSlider and QLineEdit class, synchronizing values
// across both such that a single value is represented. The value
// must be within a specified range.
//
class Combo : public QWidget {
    Q_OBJECT

public:
    Combo(QLineEdit *edit, QSlider *slider, bool intType = false);

    // This method must be called whenever the minimax or maximum allowable
    // valid value changes, or the current value
    // is changed externally. I.e. Update() provides a means to change
    // the internal state of the class. If minValid > maxValid, maxValid
    // will be set to minValid. If value is outside of minValid and maxValid
    // it will be set to minValid.
    //
    void Update(double minValid, double maxValid, double value);

    void Update(double value) { Update(_minValid, _maxValid, value); }

    // Returns a pointer the QSlider object
    //
    QSlider *GetSlider() const { return (_slider); };

    // Returns a pointer the QLineEdit object
    //
    QLineEdit *GetLineEdit() const { return (_lineEdit); };

    // Returns the currently set value
    //
    double GetValue() const { return (_value); };

    // Set how many digits to show for floating-point precision
    // This setting won't change anything for displaying integers.
    // The input precision value should be at least 1.
    //
    void SetPrecision(int precision);

    // switch IntType
    void SetIntType(bool);

    void SetEnabled(bool);

private slots:
    // Slot for QLineEdit events
    //
    void setLineEdit();

    // Slot for QSlider events
    //
    void setSlider();
    void setSliderMini(int pos);

public slots:
    // Public slot for changing the class's value
    //
    void SetSliderLineEdit(double);

signals:
    // This signal is emitted whenever the value of the class changes
    //
    void valueChanged(double value);
    void valueChanged(int value);

private:
    double _minValid;
    double _maxValid;
    double _value;
    bool   _intType;
    int    _floatPrecision;    // how many digits after the decimal point?

    QLineEdit * _lineEdit;
    QValidator *_lineEditValidator;
    QSlider *   _slider;
};

#endif
