#include "TFInfoWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleValidator>
#include <vapor/RenderParams.h>
#include <VLineItem.h>

#ifndef __FLT_MIN__
    #define __FLT_MIN__ FLT_MIN
    #define __FLT_MAX__ FLT_MAX
#endif

TFInfoWidget::TFInfoWidget(const std::string &variableNameTag) : _variableNameTag(variableNameTag)
{
    QBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(12);
    layout->setMargin(0);
    this->setLayout(layout);
    layout->addWidget(new VLineItem("Data Value", _valueEdit = new QLineEdit));
    _valueEditType = new QComboBox;

    _valueEdit->setValidator(new QDoubleValidator(__FLT_MIN__, __FLT_MAX__, 7));

    _valueEditType->blockSignals(true);
    _valueEditType->setMinimumSize(30, 10);
    _valueEditType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _valueEditType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _valueEditType->addItem("data");
    _valueEditType->addItem("%");
    _valueEditType->blockSignals(false);

    connect(_valueEditType, SIGNAL(currentIndexChanged(int)), this, SLOT(valueEditTypeChanged(int)));
    connect(_valueEdit, SIGNAL(returnPressed()), this, SLOT(valueEditChanged()));

    this->setDisabled(true);
}

void TFInfoWidget::Update(VAPoR::RenderParams *rParams)
{
    if (!rParams) return;

    string variableName = rParams->GetValueString(_variableNameTag, "");

    _min = rParams->GetMapperFunc(variableName)->getMinMapValue();
    _max = rParams->GetMapperFunc(variableName)->getMaxMapValue();

    updateValueEditValidator();
}

void TFInfoWidget::DeselectControlPoint()
{
    this->setDisabled(true);
    _valueEdit->clear();
}

void TFInfoWidget::SetNormalizedValue(float value)
{
    value = value < 0 ? 0 : value > 1 ? 1 : value;
    _value = value;
    updateValue();
}

void TFInfoWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

void TFInfoWidget::updateValue()
{
    if (!isEnabled()) return;

    float value = _value;
    if (isUsingMappedValue())
        value = toMappedValue(value);
    else
        value *= 100;
    _valueEdit->setText(QString::number(value));
}

void TFInfoWidget::updateValueEditValidator()
{
    // If the number of decimals are not specified for setRange
    // it resets them to zero. (in Qt4 at least)

    QDoubleValidator *v = (QDoubleValidator *)_valueEdit->validator();
    if (isUsingMappedValue())
        v->setRange(_min, _max, v->decimals());
    else
        v->setRange(0, 100, v->decimals());
}

bool TFInfoWidget::isUsingNormalizedValue() const { return _valueEditType->currentIndex() == ValueFormat::Percent; }

bool TFInfoWidget::isUsingMappedValue() const { return _valueEditType->currentIndex() == ValueFormat::Mapped; }

float TFInfoWidget::toMappedValue(float normalized) const { return normalized * (_max - _min) + _min; }

float TFInfoWidget::toNormalizedValue(float mapped) const
{
    if (_max == _min) return 0;

    return (mapped - _min) / (_max - _min);
}

float TFInfoWidget::getValueFromEdit() const
{
    float value = _valueEdit->text().toFloat();
    if (isUsingMappedValue())
        return toNormalizedValue(value);
    else
        return value / 100.f;
}

void TFInfoWidget::valueEditTypeChanged(int)
{
    updateValue();
    updateValueEditValidator();
}

void TFInfoWidget::valueEditChanged()
{
    _value = getValueFromEdit();
    controlPointChanged();
}
