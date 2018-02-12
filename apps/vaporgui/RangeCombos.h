#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#ifndef RANGECOMBOS_H
#define RANGECOMBOS_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSlider;
class QValidator;
QT_END_NAMESPACE

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

    void Update(double value) {
        Update(_minValid, _maxValid, value);
    }

    // Returns a pointer the QSlider object
    //
    QSlider *GetSlider() const { return (_slider); };

    // Returns a pointer the QLineEdit object
    //
    QLineEdit *GetLineEdit() const { return (_lineEdit); };

    // Returns the currently set value
    //
    double GetValue() const { return (_value); };

    // set how many digits to show for floating-point precision
    // This setting won't change anything for displaying integers.
    // The input precision value should be at least 1.
    void SetPrecision(int precision);

    void SetEnabled(bool on);

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

  protected:
    double _minValid;
    double _maxValid;
    double _value;
    bool _intType;

    int _floatPrecision; // how many digits after the decimal point?

    QLineEdit *_lineEdit;
    QValidator *_lineEditValidator;
    QSlider *_slider;
};

//
// class RangeCombo
//
// This class manages a pair of Combo objects, one for
// a minimum value and one for a maximum value. The class ensures that
// the minimum is always less than or equal to the maximum by making
// adjustments as necessary.
//
class RangeCombo : public QWidget {
    Q_OBJECT

  public:
    RangeCombo(Combo *minWidget, Combo *maxWidget);

    // This method must be called whenever the minimax or maximum allowable
    // valid value changes, or the current minimum or maximum value
    // is changed externally. I.e. Update() provides a means to change
    // the internal state of the class. If minValid > maxValid, maxValid
    // will be set to minValid. If minValue or maxValid is outside of
    // minValid and maxValid it will be set to minValid. if minValue is
    // greater than maxValue, maxValue will be set to minValue
    //
    void Update(
        double minValid, double maxValid, double minValue, double maxValue);

    QSlider *GetSliderMin() const { return (_minWidget->GetSlider()); };
    QLineEdit *GetLineEditMin() const { return (_minWidget->GetLineEdit()); };

    QSlider *GetSliderMax() const { return (_maxWidget->GetSlider()); };
    QLineEdit *GetLineEditMax() const { return (_maxWidget->GetLineEdit()); };

  public slots:
    void setValueMin(double);
    void setValueMax(double);

  signals:
    void valueChanged(double minValue, double maxValue);

  private:
    Combo *_minWidget;
    Combo *_maxWidget;
};

#endif
