#pragma once

#include "PWidget.h"
#include <QLabel>

//! @class PDisplay
//! Creates a label that displays a value in the params database.
//! Does not allow for editing of said value.

class PDisplay : public PWidget {
    Q_OBJECT
    
public:
    PDisplay(const std::string &tag, const std::string &label="");

protected:
    QLabel *_label;
};



class PStringDisplay : public PDisplay {
    Q_OBJECT
    
public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};



class PIntegerDisplay : public PDisplay {
    Q_OBJECT
    
public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};



class PDoubleDisplay : public PDisplay {
    Q_OBJECT
    
public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};



class PBooleanDisplay : public PDisplay {
    Q_OBJECT
    
public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};
