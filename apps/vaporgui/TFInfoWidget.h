#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

namespace VAPoR {
    class RenderParams;
}

class TFInfoWidget : public QWidget {
    Q_OBJECT
    
public:
    
    enum ValueFormat {
        Mapped = 0,
        Percent = 1
    };
    
    TFInfoWidget();
    
    void Update(VAPoR::RenderParams *rParams);
    
public:
//    void SelectOpacityControlPoint(int index);
//    void SelectColorControlPoint(int index);
    void DeselectControlPoint();
    void SetNormalizedValue(float value);
    
protected:
    void paintEvent(QPaintEvent *event);
    void updateValue();
    bool isUsingNormalizedValue() const;
    bool isUsingMappedValue() const;
    float toMappedValue(float normalized) const;
    float toNormalizedValue(float mapped) const;
    float getValueFromEdit() const;
    
    virtual void controlPointChanged() = 0;
    
private:
    QLineEdit *_valueEdit;
    QComboBox *_valueEditType;
    
//    int _opacityId = -1;
//    int _colorId = -1;
    float _min = 0;
    float _max = 1;
    
protected:
    float _value;
    
private slots:
    void valueEditTypeChanged(int);
    void valueEditChanged();
};
