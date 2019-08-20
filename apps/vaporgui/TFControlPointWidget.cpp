#include "TFControlPointWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>

TFControlPointWidget::TFControlPointWidget()
{
    QBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    this->setLayout(layout);
    layout->addWidget(_locationEdit = new QLineEdit, 30);
    layout->addWidget(_locationEditType = new QComboBox, 20);
    layout->addStretch(20);
    layout->addWidget(_valueEdit = new QLineEdit, 30);
    
    _locationEditType->blockSignals(true);
    _locationEditType->setMinimumSize(30, 10);
    _locationEditType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _locationEditType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _locationEditType->addItem("%");
    _locationEditType->addItem("data");
    _locationEditType->blockSignals(false);
    
//    this->setStyleSheet(R"(QWidget:hover:!pressed {border: 1px solid red;})");
    this->setDisabled(true);
}

void TFControlPointWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    if (_opacityId >= 0) {
        float value;
        if (isUsingNormalizedValue())
            value = rParams->GetMapperFunc(rParams->GetVariableName())->GetOpacityMap(0)->controlPointValueNormalized(_opacityId);
        else
            value = rParams->GetMapperFunc(rParams->GetVariableName())->GetOpacityMap(0)->controlPointValue(_opacityId);
        _locationEdit->setText(QString::number(value));
        _valueEdit->setText(QString::number(rParams->GetMapperFunc(rParams->GetVariableName())->GetOpacityMap(0)->controlPointOpacity(_opacityId)));
    }
}

void TFControlPointWidget::SelectOpacityControlPoint(int index)
{
    this->setEnabled(true);
    _opacityId = index;
    _colorId = -1;
}

void TFControlPointWidget::SelectColorControlPoint(int index)
{
    this->setEnabled(true);
    _opacityId = -1;
    _colorId = index;
}

void TFControlPointWidget::DeselectControlPoint()
{
    this->setDisabled(true);
    _opacityId = -1;
    _colorId = -1;
}

void TFControlPointWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    QWidget::paintEvent(event);
}

bool TFControlPointWidget::isUsingNormalizedValue() const
{
    return _locationEditType->currentIndex() == 0;
}
