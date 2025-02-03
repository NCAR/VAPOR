#include "VRadioButtons.h"
#include <QLayoutItem>

VRadioButtons::VRadioButtons() : VHBoxWidget() {
    //for (auto o : options) {
    //    QRadioButton* rb = new QRadioButton(QString::fromStdString(o), this);
    //    layout()->addWidget(rb);
    //    connect(rb, SIGNAL(toggled()), this, SLOT(emitRadioButtonChecked(rb)));
    //    _buttons.push_back(rb);
    //}
}

void VRadioButtons::SetValues(std::vector<std::string>& values) {
    // Clear old values
    while (QLayoutItem *item = layout()->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater(); // Delete widget safely
        }
        delete item;
    }

    // Apply new ones
    for (auto v : values) {
        QRadioButton* rb = new QRadioButton(QString::fromStdString(v), this);
        //layout()->addWidget(rb);
        Add(rb);
        connect(rb, SIGNAL(toggled()), this, SLOT(radioButtonChecked(rb)));
        _buttons.push_back(rb);
    }

    //for (auto rb : _buttons) {
    //    if (rb->text().toStdString() == checked) {
    //        rb->blockSignals(true);
    //        rb->toggle();
    //        rb->blockSignals(false);
    //        return;
    //    }
    //}
}

void VRadioButtons::SetValue(std::string& checked) {
    for (auto rb : _buttons) {
        if (rb->text().toStdString() == checked) {
            rb->blockSignals(true);
            rb->toggle();
            rb->blockSignals(false);
            return;
        }
    }
}

std::string VRadioButtons::GetValue() const { 
    //int size = layout()->count();
    //for (int i=0; i<size; i++) {
    for (auto rb : _buttons) {
        //QRadioButton* rb = itemAt(i);
        if (rb->isChecked()) return rb->text().toStdString();
    }
    return "";
}

//void VRadioButtons::radioButtonChecked(QRadioButton* rb) { 
void VRadioButtons::radioButtonChecked() { 
    //emit ValueChanged(rb->text().toStdString()); 
    emit ValueChanged();
}
