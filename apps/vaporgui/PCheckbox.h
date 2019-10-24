#pragma once

#include "PWidget.h"
#include <QCheckBox>

class PCheckbox : public PWidget {
    Q_OBJECT
    
    QCheckBox *_qcheckbox;
    
public:
    PCheckbox(const std::string &tag, const std::string &label="");

protected:
    void update() const;
    
private slots:
    void checkboxStateChanged(int state);
};
