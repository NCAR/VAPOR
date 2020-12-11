#pragma once

#include <QWidget>
#include <VDoubleLineEdit.h>
#include <QComboBox>
#include <string>

namespace VAPoR {
    class RenderParams;
}

//! \class TFInfoWidget
//! The TFInfoWidget displays details and allow users to manually edit values for the control
//! points used by the TFMapWidget

class TFInfoWidget : public QWidget {
    Q_OBJECT
    
public:
    enum ValueFormat {
        Mapped = 0,
        Percent = 1
    };
    
    TFInfoWidget(const std::string &variableNameTag);
    
    void Update(VAPoR::RenderParams *rParams);
    void DeselectControlPoint();
    void SetNormalizedValue(float value);
    
protected:
    void paintEvent(QPaintEvent *event);
    void updateValue();
    void updateValueEditValidator();
    bool isUsingNormalizedValue() const;
    bool isUsingMappedValue() const;
    float toMappedValue(float normalized) const;
    float toNormalizedValue(float mapped) const;
    float getValueFromEdit() const;
    
    virtual void controlPointChanged() {};
    
protected:
    VDoubleLineEdit *_valueEdit;
    QComboBox *_valueEditType;
    
    float _min = 0;
    float _max = 1;
    
protected:
    float _value;
    const std::string _variableNameTag;
    
private slots:
    void valueEditTypeChanged(int);
    void valueEditChanged();
};
