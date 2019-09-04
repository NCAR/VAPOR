#include "QRangeSlider.h"
#include <QLineEdit>
#include <QWidget>

class QRangeSliderTextCombo : public QWidget {
    Q_OBJECT
    
    QRangeSlider *_slider;
    QLineEdit *_leftText;
    QLineEdit *_rightText;
    float _min, _max;
    float _left, _right;
    
public:
    QRangeSliderTextCombo();
    
    void SetRange(float min, float max);
    void SetValue(float left, float right);
    
private:
    void setValidator(QLineEdit *edit, QValidator *validator);
    void setTextboxes(float left, float right);
    
private slots:
    void sliderChangedIntermediate(float min, float max);
    void sliderChanged(float min, float max);
    void leftTextChanged();
    void rightTextChanged();
    
signals:
    void ValueChanged(float min, float max);
};
