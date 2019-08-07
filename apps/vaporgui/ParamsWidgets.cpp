#include "ParamsWidgets.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QIntValidator>
#include <QEvent>
#include <QColorDialog>

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
    _lineEdit->setValidator(new QIntValidator);
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChangedSlot()));
    
    layout()->addWidget(_lineEdit);
}

void ParamsWidgetNumber::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _lineEdit->setText(QString::number(p->GetValueLong(_tag, false)));
}

ParamsWidgetNumber *ParamsWidgetNumber::SetRange(int min, int max)
{
    const QValidator *toDelete = _lineEdit->validator();
    _lineEdit->setValidator(new QIntValidator(min, max));
    if (toDelete) delete toDelete;
    return this;
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
    _lineEdit->setValidator(new QDoubleValidator);
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChangedSlot()));
    
    layout()->addWidget(_lineEdit);
}

void ParamsWidgetFloat::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _lineEdit->setText(QString::number(p->GetValueDouble(_tag, false)));
}

ParamsWidgetFloat *ParamsWidgetFloat::SetRange(float min, float max)
{
    const QValidator *toDelete = _lineEdit->validator();
    _lineEdit->setValidator(new QDoubleValidator(min, max, 2));
    if (toDelete) delete toDelete;
    return this;
}

void ParamsWidgetFloat::valueChangedSlot()
{
    if (_params)
        _params->SetValueDouble(_tag, _tag, _lineEdit->text().toDouble());
}




ParamsWidgetColor::ParamsWidgetColor(const std::string &tag, const std::string &label)
: ParamsWidget(tag, label)
{
    _button = new QPushButton("Select");
    connect(_button, SIGNAL(clicked()), this, SLOT(pressed()));
    
    layout()->addWidget(_button);
}

void ParamsWidgetColor::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    
    QColor color = VectorToQColor(p->GetValueDoubleVec(_tag));
    QString style = "background-color: " + color.name() + "; ";
    if (color.valueF() > 0.6)
        style += "color: black;";
    else
        style += "color: white;";
    
    _button->setStyleSheet(style);
}

void ParamsWidgetColor::pressed()
{
    QColor newColor = QColorDialog::getColor();
    if (_params)
        _params->SetValueDoubleVec(_tag, _tag, QColorToVector(newColor));
}

QColor ParamsWidgetColor::VectorToQColor(const std::vector<double> &v)
{
    if (v.size() != 3) return QColor::fromRgb(0,0,0);
    return QColor::fromRgb(v[0]*255, v[1]*255, v[2]*255);
}

std::vector<double> ParamsWidgetColor::QColorToVector(const QColor &c)
{
    vector<double> v(3, 0);
    v[0] = c.redF();
    v[1] = c.greenF();
    v[2] = c.blueF();
    return v;
}




ParamsWidgetGroup::ParamsWidgetGroup(const std::string &title)
: QGroupBox(title.c_str())
{
    setLayout(new QVBoxLayout);
}

void ParamsWidgetGroup::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange && !fontUpdated) {
        fontUpdated = true;
        this->setStyleSheet("font-size: " + QString::number(font().pointSize()) + "pt");
    }
}




ParamsWidgetTabGroup::ParamsWidgetTabGroup(const std::string &title)
{
    addTab(new QWidget(this), QString::fromStdString(title));
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    _tab()->setLayout(layout);
}

QWidget *ParamsWidgetTabGroup::_tab() const
{
    return this->widget(0);
}

void ParamsWidgetTabGroup::Update(VAPoR::ParamsBase *p)
{
    for (ParamsWidget *w : _widgets)
        w->Update(p);
}

void ParamsWidgetTabGroup::Add(ParamsWidget *widget)
{
    _tab()->layout()->addWidget(widget);
    _widgets.push_back(widget);
}
