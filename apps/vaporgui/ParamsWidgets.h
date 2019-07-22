#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <vapor/ParamsBase.h>

class ParamsWidget : public QWidget {
    Q_OBJECT
    
public:
    ParamsWidget(const std::string &tag, const std::string &label = "");
    virtual void Update(VAPoR::ParamsBase *p) = 0;
    
protected:
    VAPoR::ParamsBase *_params = nullptr;
    std::string _tag;
    std::string _label;
};




class ParamsWidgetCheckbox : public ParamsWidget {
    Q_OBJECT
    
    QCheckBox *_checkBox = nullptr;
    
public:
    ParamsWidgetCheckbox(const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);
    
private slots:
    void checkbox_clicked(bool checked);
};




class ParamsWidgetNumber : public ParamsWidget {
    Q_OBJECT
    
    QLineEdit *_lineEdit = nullptr;
    
public:
    ParamsWidgetNumber(const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);
    
private slots:
    void valueChangedSlot();
};
