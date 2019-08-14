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

    virtual void GetValue( int&                 value) const;
    virtual void GetValue( double&              value) const;
    virtual void GetValue( std::string&         value) const;
    virtual void GetValue( std::vector<double>& value) const;

protected:
    ParamsWidget( 
        QWidget* parent, 
        const std::string& tag,
        const std::string& description
    );

    ~ParamsWidget() {};

    VAPoR::ParamsBase* _params = nullptr;
    VaporWidget* _vaporWidget;
    std::string _tag;
    std::string _description;

protected slots:
    virtual void _updateParams() {};

signals:
    void _valueChanged();
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

    void Update( VAPoR::ParamsBase* params ) override;

    void GetValue( int& value ) const override;

protected slots:
    void _updateParams() override;

signals:
    void _valueChanged();
};

class PSlider : public ParamsWidget
{
    Q_OBJECT

public:
    PSlider(
        QWidget* parent,
        const std::string& tag,
        const std::string& description, 
        const std::string& label,
        double min=0, 
        double max=100, 
        double val = 0 
    );

    void Update( VAPoR::ParamsBase* params ) override;

    void GetValue( double& value ) const override;

protected slots:
    void _updateParams() override;

signals:
    void _valueChanged();
};

class PRange : public ParamsWidget
{
    Q_OBJECT

public:
    PRange(
        QWidget* parent,
        const std::string& tag,
        const std::string& description,
        double min,
        double max,
        const std::string& minLabel = "Min",
        const std::string& maxLabel = "Max"
    );

    void Update( VAPoR::ParamsBase* params ) override;

    void GetValue( std::vector<double>& values ) const override;

protected slots:
    void _updateParams() override;

signals:
    void _valueChanged();
        
};

#endif
