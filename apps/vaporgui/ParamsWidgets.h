#ifndef PARAMSWIDGETS_H
#define PARAMSWIDGETS_H

#include <vapor/ParamsBase.h>

#include <QWidget>

class VaporWidget;
class VSpinBox;

class ParamsWidget : public QWidget
{
    Q_OBJECT

public:
    virtual void Update( VAPoR::ParamsBase* params ) = 0;

protected:
    ParamsWidget( 
        QWidget* parent, 
        const std::string& tag,
        const std::string& description
    );

    VAPoR::ParamsBase* _params = nullptr;
    VaporWidget* _vaporWidget;
    std::string _tag;
    std::string _description;

//protected slots:
//    void _updateParams();
};

class PSpinBox : public ParamsWidget
{
    Q_OBJECT

public:
    PSpinBox( 
        QWidget* parent, 
        const std::string& tag, 
        const std::string& description, 
        const std::string& label,
        int min=0, 
        int max=100, 
        int val = 0 
    );

    void Update( VAPoR::ParamsBase* params );

    int GetValue();

protected slots:
    void _updateParams();

signals:
    void _paramsUpdated();
};

#endif
