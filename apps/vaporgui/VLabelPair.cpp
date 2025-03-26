#include "VLabelPair.h"
#include <QLabel>

VLabelPair::VLabelPair() : VHBoxWidget() {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    _leftTime = new QLabel();
    _rightTime = new QLabel();
    _rightTime->setAlignment(Qt::AlignRight);

    layout()->addWidget(_leftTime);
    layout()->addWidget(_rightTime);
}

void VLabelPair::SetLeftText(const std::string &time) {
    _leftTime->setText(QString::fromStdString(time));
}

void VLabelPair::SetRightText(const std::string &time) {
    _rightTime->setText(QString::fromStdString(time));
}
