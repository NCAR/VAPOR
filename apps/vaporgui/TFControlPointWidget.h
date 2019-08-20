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
    TFControlPointWidget();
    
    void Update(VAPoR::RenderParams *rParams);
    
public slots:
    void SelectOpacityControlPoint(int index);
    void SelectColorControlPoint(int index);
    void DeselectControlPoint();
    
protected:
    void paintEvent(QPaintEvent *event);
    bool isUsingNormalizedValue() const;
    
private:
    QLineEdit *_locationEdit;
    QComboBox *_locationEditType;
    QLineEdit *_valueEdit;
    
    int _opacityId = -1;
    int _colorId = -1;
};
