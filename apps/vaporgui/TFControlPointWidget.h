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
    
public slots:
    void SelectOpacityControlPoint(int index);
    void SelectColorControlPoint(int index);
    void DeselectControlPoint();
    
protected:
    void paintEvent(QPaintEvent *event);
    bool isUsingNormalizedValue() const;
    bool isUsingMappedValue() const;
    float toMappedValue(float normalized) const;
    float toNormalizedValue(float mapped) const;
    
private:
    QLineEdit *_valueEdit;
    QComboBox *_valueEditType;
    QLineEdit *_opacityEdit;
    
    int _opacityId = -1;
    int _colorId = -1;
    VAPoR::RenderParams *_params;
    float _min = 0;
    float _max = 1;
    float _value;
    
private slots:
    void valueEditTypeChanged(int);
};
