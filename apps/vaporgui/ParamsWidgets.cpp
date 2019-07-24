#include "ParamsWidgets.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QIntValidator>

using namespace VAPoR;

ParamsWidget::ParamsWidget(const std::string &tag, const std::string &label)
{
    assert(!tag.empty());
    _tag = tag;
    
    if (label.empty()) _label = tag;
    else _label = label;
    
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    this->setLayout(layout);
    
    QLabel *labelWidget = new QLabel(_label.c_str());
    QSpacerItem *spacer = new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    layout->addWidget(labelWidget);
    layout->addItem(spacer);
}




ParamsWidgetCheckbox::ParamsWidgetCheckbox(const std::string &tag, const std::string &labelText)
: ParamsWidget(tag, labelText)
{
    _checkBox = new QCheckBox();
    connect(_checkBox, SIGNAL(clicked(bool)), this, SLOT(checkbox_clicked(bool)));
    
    layout()->addWidget(_checkBox);
}

void ParamsWidgetCheckbox::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _checkBox->setChecked(p->GetValueLong(_tag, false));
}

void ParamsWidgetCheckbox::checkbox_clicked(bool checked)
{
    if (_params)
        _params->SetValueLong(_tag, _tag, checked);
}




ParamsWidgetNumber::ParamsWidgetNumber(const std::string &tag, const std::string &labelText)
: ParamsWidget(tag, labelText)
{
    _lineEdit = new QLineEdit();
    _lineEdit->setValidator(new QIntValidator(this));
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChangedSlot()));
    
    layout()->addWidget(_lineEdit);
}

void ParamsWidgetNumber::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _lineEdit->setText(QString::number(p->GetValueLong(_tag, false)));
}

void ParamsWidgetNumber::valueChangedSlot()
{
    if (_params)
        _params->SetValueLong(_tag, _tag, _lineEdit->text().toInt());
}





ParamsWidgetFloat::ParamsWidgetFloat(const std::string &tag, const std::string &labelText)
: ParamsWidget(tag, labelText)
{
    _lineEdit = new QLineEdit();
    _lineEdit->setValidator(new QDoubleValidator(this));
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChangedSlot()));
    
    layout()->addWidget(_lineEdit);
}

void ParamsWidgetFloat::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _lineEdit->setText(QString::number(p->GetValueDouble(_tag, false)));
}

void ParamsWidgetFloat::valueChangedSlot()
{
    if (_params)
        _params->SetValueDouble(_tag, _tag, _lineEdit->text().toDouble());
}




ParamsWidgetGroup::ParamsWidgetGroup(const std::string &title)
: QGroupBox(title.c_str())
{
    setLayout(new QVBoxLayout);
}
