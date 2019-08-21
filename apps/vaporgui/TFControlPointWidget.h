#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

namespace VAPoR {
    class RenderParams;
}

class TFControlPointWidget : public QWidget {
    Q_OBJECT
    
public:
    
    enum ValueFormat {
        Mapped = 0,
        Percent = 1
    };
    
    TFControlPointWidget();
    
    void Update(VAPoR::RenderParams *rParams);
    
public:
//    void SelectOpacityControlPoint(int index);
//    void SelectColorControlPoint(int index);
    void DeselectControlPoint();
    void SetNormalizedValue(float value);
    void SetOpacity(float opacity);
    void SetControlPoint(float value, float opacity);
    
protected:
    void paintEvent(QPaintEvent *event);
    void updateValue();
    void updateOpacity();
    bool isUsingNormalizedValue() const;
    bool isUsingMappedValue() const;
    float toMappedValue(float normalized) const;
    float toNormalizedValue(float mapped) const;
    float getValueFromEdit() const;
    float getOpacityFromEdit() const;
    
private:
    QLineEdit *_valueEdit;
    QComboBox *_valueEditType;
    QLineEdit *_opacityEdit;
    
//    int _opacityId = -1;
//    int _colorId = -1;
    float _min = 0;
    float _max = 1;
    float _value;
    float _opacity;
    
signals:
    void ControlPointChanged(float value, float opacity);
    
private slots:
    void valueEditTypeChanged(int);
    void valueEditChanged();
    void opacityEditChanged();
};
