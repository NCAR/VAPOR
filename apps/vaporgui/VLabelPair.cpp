#include "VLabelPair.h"
#include <QLabel>

VLabelPair::VLabelPair() : VHBoxWidget() {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    _leftLabel = new QLabel();
    _rightLabel = new QLabel();
    _rightLabel->setAlignment(Qt::AlignRight);

    layout()->addWidget(_leftLabel);
    layout()->addWidget(_rightLabel);
}

void VLabelPair::SetLeftText(const std::string &text) const {
    _leftLabel->setText(QString::fromStdString(text));
}

void VLabelPair::SetRightText(const std::string &text) const {
    _rightLabel->setText(QString::fromStdString(text));
}
