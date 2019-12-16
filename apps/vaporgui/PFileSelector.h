#pragma once

#include "PLineItem.h"

class QPushButton;
class QLineEdit;

class PFileSelector : public PLineItem {
    Q_OBJECT

    QPushButton *_button = nullptr;
    QLineEdit *  _pathTexbox;
    QString      _fileTypeFilter = "All Files (*)";

public:
    PFileSelector(const std::string &tag, const std::string &label);
    PFileSelector *SetFileTypeFilter(const std::string &filter);

protected:
    void updateGUI() const override;

private slots:
    void buttonClicked();
};
