#pragma once

#include "PWidget.h"

class VLabel;

//! @class PDisplay
//! Creates a label that displays a value in the params database.
//! Does not allow for editing of said value.

class PDisplay : public PWidget {
    Q_OBJECT

public:
    PDisplay(const std::string &tag, const std::string &label = "");
    PDisplay *Selectable();

protected:
    VLabel *_label;
    
    void setText(std::string text) const;
};

//! @class PStringDisplay
//! @copydoc PDisplay

class PStringDisplay : public PDisplay {
    Q_OBJECT

public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};

//! @class PIntegerDisplay
//! @copydoc PDisplay

class PIntegerDisplay : public PDisplay {
    Q_OBJECT

public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};

//! @class PDoubleDisplay
//! @copydoc PDisplay

class PDoubleDisplay : public PDisplay {
    Q_OBJECT

public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};

//! @class PBooleanDisplay
//! @copydoc PDisplay

class PBooleanDisplay : public PDisplay {
    Q_OBJECT

public:
    using PDisplay::PDisplay;

protected:
    void updateGUI() const override;
};
