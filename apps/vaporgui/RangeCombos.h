#ifdef WIN32
    #pragma warning(disable : 4100)
#endif

#ifndef RANGECOMBOS_H
    #define RANGECOMBOS_H

    #include <QWidget>
    #include "Combo.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QGroupBox;
class QLineEdit;
class QMontereySlider;
class QValidator;
QT_END_NAMESPACE

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
    void Update(double minValid, double maxValid, double minValue, double maxValue);

    QMontereySlider *  GetSliderMin() const { return (_minWidget->GetSlider()); };
    QLineEdit *GetLineEditMin() const { return (_minWidget->GetLineEdit()); };

    QMontereySlider *  GetSliderMax() const { return (_maxWidget->GetSlider()); };
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
